/* SPDX-License-Identifier: MIT */

/* StACSOS - mandelbrot utility
 *
 * Copyright (c) Kim Stonehouse 2022
 *
 * Originally developed for InfOS by:
 * Kim Stonehouse <Kim.Stonehouse@ed.ac.uk>
 *
 * Ported to StACSOS by:
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */

/*
 * A program that prints a rather crude version of the Mandelbrot fractal to the StACSOS terminal.
 */

#include <stacsos/atomic.h>
#include <stacsos/console.h>
#include <stacsos/objects.h>
#include <stacsos/threads.h>

using namespace stacsos;

#define BLACK 0
#define BLUE 2
#define GREEN 3
#define CYAN 4
#define RED 4
#define MAGENTA 5
#define ORANGE 6
#define L_GREY 7
#define GREY 8
#define L_BLUE 9
#define L_GREEN 10
#define L_CYAN 11
#define L_RED 12
#define L_MAGENTA 13
#define YELLOW 14
#define WHITE 15

#define MAXITERATE 10000000
#define NORM_FACT 67108864
#define NORM_BITS 26

s64 realMin, realMax;
s64 imagMin, imagMax;
s64 deltaReal, deltaImag;

const u32 WIDTH = 80; // frame is 80x25
const u32 HEIGHT = 25;
const u32 LAST_PIXEL = WIDTH * HEIGHT;

//#define WORKLIST

#ifdef WORKLIST
atomic<u32> next_pixel = 0;
#endif

object *fb;

static void drawchar(int x, int y, int attr, unsigned char c)
{
	u16 u = (attr << 8) | c;
	fb->pwrite((const char *)&u, sizeof(u), x + (y * 80));
}

void output(int value, int i, int j)
{
	if (value == 10000000) {
		drawchar(j, i, BLACK, ' ');
	} else if (value > 9000000) {
		drawchar(j, i, RED, '*');
	} else if (value > 5000000) {
		drawchar(j, i, L_RED, '*');
	} else if (value > 1000000) {
		drawchar(j, i, ORANGE, '*');
	} else if (value > 500) {
		drawchar(j, i, YELLOW, '*');
	} else if (value > 100) {
		drawchar(j, i, L_GREEN, '*');
	} else if (value > 10) {
		drawchar(j, i, GREEN, '*');
	} else if (value > 5) {
		drawchar(j, i, L_CYAN, '*');
	} else if (value > 4) {
		drawchar(j, i, CYAN, '*');
	} else if (value > 3) {
		drawchar(j, i, L_BLUE, '*');
	} else if (value > 2) {
		drawchar(j, i, BLUE, '*');
	} else if (value > 1) {
		drawchar(j, i, MAGENTA, '*');
	} else {
		drawchar(j, i, L_MAGENTA, '*');
	}
}

#ifndef WORKLIST
struct thread_data {
	u32 start_pixel, end_pixel;
};
#endif

static void *mandelbrot(void *arg)
{
#ifndef WORKLIST
	thread_data *data = (thread_data *)arg;
	u32 my_pixel = data->start_pixel;

	while (my_pixel < data->end_pixel) {
#else
	u32 my_pixel = next_pixel++;

	while (my_pixel < LAST_PIXEL) {
#endif

		int x = my_pixel % WIDTH;
		int y = my_pixel / WIDTH;

		s64 real0, imag0, realq, imagq, real, imag;
		int count;

		real0 = realMin + x * deltaReal; // current real value
		imag0 = imagMax - y * deltaImag;

		real = real0;
		imag = imag0;
		for (count = 0; count < MAXITERATE; count++) {
			realq = (real * real) >> NORM_BITS;
			imagq = (imag * imag) >> NORM_BITS;

			if ((realq + imagq) > ((s64)4 * NORM_FACT))
				break;

			imag = ((real * imag) >> (NORM_BITS - 1)) + imag0;
			real = realq - imagq + real0;
		}

		output(count, y, x);

#ifdef WORKLIST
		my_pixel = next_pixel++;
#else
		my_pixel++;
#endif
	}

	return nullptr;
}

int main(const char *cmdline)
{
	fb = object::open("/dev/virtcon0");

	if (!fb) {
		console::get().write("error: unable to open virtual console\n");
		return 1;
	}

	const int NUM_THREADS = 8;

	thread *threads[NUM_THREADS];

#ifndef WORKLIST
	thread_data thread_data[NUM_THREADS];
#endif

	realMin = -2 * NORM_FACT;
	realMax = 1 * NORM_FACT;
	imagMin = -1 * NORM_FACT;
	imagMax = 1 * NORM_FACT;

	deltaReal = (realMax - realMin) / (WIDTH - 1);
	deltaImag = (imagMax - imagMin) / (HEIGHT - 1);

	const u32 CHUNK_SIZE = LAST_PIXEL / NUM_THREADS;

	for (int thread_index = 0; thread_index < NUM_THREADS; thread_index++) {
#ifndef WORKLIST
		thread_data[thread_index].start_pixel = thread_index * CHUNK_SIZE;
		thread_data[thread_index].end_pixel = thread_data[thread_index].start_pixel + CHUNK_SIZE;
		if (thread_data[thread_index].end_pixel > LAST_PIXEL) {
			thread_data[thread_index].end_pixel = LAST_PIXEL;
		}

		threads[thread_index] = thread::start(mandelbrot, &thread_data[thread_index]);
#else
		threads[thread_index] = thread::start(mandelbrot, nullptr);
#endif
	}

	for (int k = 0; k < NUM_THREADS; k++) {
		threads[k]->join();
	}

	// wait for input so the prompt doesn't ruin the lovely image
	// remove this when timing!
	console::get().read_char();

	delete fb;

	return 0;
}
