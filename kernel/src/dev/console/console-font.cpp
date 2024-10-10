#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/dev/console/console-font.h>

using namespace stacsos::kernel::dev::console;

#define PSF1_FONT_MAGIC 0x0436

struct psf1_font_header {
	u16 magic;
	u8 font_mode;
	u8 char_size;
} __packed;

#define PSF2_FONT_MAGIC 0x864ab572

struct psf2_font_header {
	u32 magic;
	u32 version;
	u32 headersize;
	u32 flags;
	u32 numglyph;
	u32 bytesperglyph;
	u32 height;
	u32 width;
} __packed;

extern "C" u8 _binary_zap_light16_psf_start;
extern "C" u8 _binary_zap_vga16_psf_start;
extern "C" u8 _binary_tamsyn_8x15r_psf_start;

static console_font zap_light16(&_binary_zap_light16_psf_start, 0);
static console_font zap_vga16(&_binary_zap_vga16_psf_start, 0);
static console_font tamsyn_8x15r(&_binary_tamsyn_8x15r_psf_start, 0);

console_font *active_font = &tamsyn_8x15r;

void console_font::parse()
{
	u32 font_magic = *(const u32 *)font_data_;

	if ((font_magic & 0xffff) == PSF1_FONT_MAGIC) {
		font_data_start_ = font_data_ + sizeof(psf1_font_header);
		char_dims_ = console_font_char_dimensions(8, ((const psf1_font_header *)font_data_)->char_size);
	} else if (font_magic == PSF2_FONT_MAGIC) {
		const psf2_font_header *font_header = (const psf2_font_header *)font_data_;

		dprintf("font: psf2: version=%d, header-size=%d, flags=%x, length=%d, glyph-size=%d, height=%d, width=%d\n", font_header->version,
			font_header->headersize, font_header->flags, font_header->numglyph, font_header->bytesperglyph, font_header->height, font_header->width);

        font_data_start_ = font_data_ + font_header->headersize;
        char_dims_ = console_font_char_dimensions(font_header->width, font_header->height);
	} else {
		panic("corrupted font file (%08x)", font_magic);
	}
}
