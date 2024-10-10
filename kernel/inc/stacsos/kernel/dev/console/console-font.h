#pragma once

namespace stacsos::kernel::dev::console {
class console_font_char_dimensions {
public:
	console_font_char_dimensions()
		: width_(0)
		, height_(0)
	{
	}

	console_font_char_dimensions(size_t width, size_t height)
		: width_(width)
		, height_(height)
	{
	}

	size_t width() const { return width_; }
	size_t height() const { return height_; }

private:
	size_t width_;
	size_t height_;
};

class console_font_char {
public:
	console_font_char(const console_font_char_dimensions &dim, const u8 *data)
		: dim_(dim)
		, pixel_data_(data)
	{
	}

	const console_font_char_dimensions &dims() const { return dim_; }

	int get_pixel(int x, int y) { return !!(pixel_data_[y] & (1 << (7 - x))); }

private:
	console_font_char_dimensions dim_;
	const u8 *pixel_data_;
};

class console_font {
public:
	console_font(const u8 *font_data, size_t size)
		: font_data_(font_data)
		, font_data_start_(nullptr)
		, size_(size)
	{
		//parse();
	}

	const console_font_char_dimensions &char_dims() const { return char_dims_; }

	console_font_char get_char(unsigned char ch) const { return console_font_char(char_dims_, &font_data_start_[(unsigned int)ch * char_dims_.height()]); }

private:
	const u8 *font_data_, *font_data_start_;
	size_t size_;
	console_font_char_dimensions char_dims_;

public:
	void parse();
};
} // namespace stacsos::kernel::dev::console
