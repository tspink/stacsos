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

int width = 80; // frame is 80x25
int height = 25;

atomic<u32> next_pixel = 0;
u32 last_pixel;

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

static void *mandelbrot(void *arg)
{
	u32 my_next_pixel = next_pixel++;

	while (my_next_pixel <= last_pixel) {
		int x = my_next_pixel % width;
		int y = my_next_pixel / width;

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
		my_next_pixel = next_pixel++;
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

	//    printf("\033\x09How many threads would you like to use?\n");
	//    int numThreads = getch();
	int numThreads = 8;
	thread *threads[numThreads];

	realMin = -2 * NORM_FACT;
	realMax = 1 * NORM_FACT;
	imagMin = -1 * NORM_FACT;
	imagMax = 1 * NORM_FACT;

	deltaReal = (realMax - realMin) / (width - 1);
	deltaImag = (imagMax - imagMin) / (height - 1);

	last_pixel = width * height;

	for (int k = 0; k < numThreads; k++) {
		threads[k] = thread::start(mandelbrot, (void *)(u64)k);
	}

	for (int k = 0; k < numThreads; k++) {
		threads[k]->join();
	}

	// wait for input so the prompt doesn't ruin the lovely image
	// remove this when timing!
	console::get().read_char();

	delete fb;

	return 0;
}
