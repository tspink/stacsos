/* SPDX-License-Identifier: MIT */

/* StACSOS - Utility Library
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/list.h>
#include <stacsos/memops.h>

namespace stacsos {
enum class pad_side { LEFT, RIGHT };

class string {
public:
	using char_type = char;
	using hash_type = u64;

	using const_iterator = const char_type *;

	string()
		: size_(0)
		, data_(new char_type[1])
		, has_hash_(false)
		, hash_(0)
	{
		data_[0] = 0;
	}

	string(const char_type *str)
		: size_(memops::strlen(str))
		, data_(nullptr)
		, has_hash_(false)
		, hash_(0)
	{
		data_ = new char_type[size_ + 1];

		memops::memcpy(data_, str, size_);
		data_[size_] = 0;
	}

	// Copy Constructor

	string(const string &str)
		: size_(str.size_)
		, data_(nullptr)
		, has_hash_(str.has_hash_)
		, hash_(str.hash_)
	{
		data_ = new char_type[size_ + 1];

		memops::memcpy(data_, str.data_, size_);
		data_[size_] = 0;
	}

	// Move Constructor

	string(string &&str)
		: size_(str.size_)
		, data_(str.data_)
		, has_hash_(str.has_hash_)
		, hash_(str.has_hash_)
	{
		str.data_ = nullptr;
		str.size_ = 0;
	}

	// Destructor

	~string() { delete[] data_; }

	static string format(const string &fmt, ...);

	/**
	 * Returns the length of the string.
	 * @return Returns the length of the string.
	 */
	size_t length() const { return size_; }

	/**
	 * Returns whether or not the string is empty (i.e. length == 0)
	 */
	bool empty() const { return size_ == 0; }

	/**
	 * Returns the C-representation of the string.
	 * @return Returns the C-representation of the string.
	 */
	const char_type *c_str() const { return data_; }

	/**
	 * Retrieves the hash of this string, lazily computing it
	 * if necessary.
	 *
	 * @return Returns a 64-bit hash of the string.
	 */
	hash_type get_hash() const
	{
		if (has_hash_)
			return hash_;

		hash_ = compute_hash();
		has_hash_ = true;

		return hash_;
	}

	/**
	 * Pads the string out to the specified width, using the specified padding
	 * character.
	 * @param width The number of characters to pad the string out to.
	 * @param ch The character to use for padding.
	 * @param side The side on which to insert the pad.
	 * @return Returns a new padded string.
	 */
	string pad(int width, char ch, pad_side side);

	const_iterator begin() const { return &data_[0]; }
	const_iterator end() const { return &data_[size_ + 1]; }

	friend string &operator+=(string &s, const char_type &ch)
	{
		size_t new_size = s.size_ + 1;
		char_type *new_data = new char_type[new_size + 1];
		memops::memcpy(new_data, s.data_, s.size_);
		new_data[new_size - 1] = ch;
		new_data[new_size] = 0;

		delete s.data_;
		s.data_ = new_data;
		s.size_ = new_size;
		s.has_hash_ = false;

		return s;
	}

	friend string &operator+=(string &l, const string &r)
	{
		size_t new_size = l.size_ + r.size_;
		char_type *new_data = new char_type[new_size + 1];

		memops::memcpy(new_data, l.data_, l.size_);
		memops::memcpy(new_data + l.size_, r.data_, r.size_);

		new_data[new_size] = 0;

		delete l.data_;
		l.data_ = new_data;
		l.size_ = new_size;
		l.has_hash_ = false;

		return l;
	}

	friend string operator+(const string &l, const string &r)
	{
		string n(l.size_ + r.size_);

		memops::memcpy(n.data_, l.data_, l.size_);
		memops::memcpy(n.data_ + l.size_, r.data_, r.size_);

		n.data_[n.size_] = 0;

		return n;
	}

	friend string operator+(const string &l, const char_type &r)
	{
		string n(l.size_ + 1);

		memops::memcpy(n.data_, l.data_, l.size_);

		n.data_[n.size_ - 1] = r;
		n.data_[n.size_] = 0;

		return n;
	}

	friend bool operator<(const string &l, const string &r) { return false; }

	/* Copy Assignment */
	string &operator=(const string &s)
	{
		if (this != &s) {
			delete data_;

			size_ = s.size_;
			data_ = new char_type[size_ + 1];

			memops::memcpy(data_, s.data_, size_);

			data_[size_] = 0;

			has_hash_ = s.has_hash_;
			hash_ = s.hash_;
		}

		return *this;
	}

	/* Move Assignment */
	string &operator=(string &&s)
	{
		if (this != &s) {
			delete data_;

			size_ = s.size_;
			data_ = s.data_;
			has_hash_ = s.has_hash_;
			hash_ = s.hash_;

			s.data_ = nullptr;
			s.size_ = 0;
		}

		return *this;
	}

	char_type operator[](unsigned int idx) const
	{
		if (idx >= size_)
			return 0;
		return data_[idx];
	}

	friend bool operator==(const string &l, const string &r)
	{
		if (l.size_ != r.size_)
			return false;

		for (unsigned int i = 0; i < l.size_; i++) {
			if (l.data_[i] != r.data_[i])
				return false;
		}

		return true;
	}

	list<string> split(char delim, bool remove_empty);

public:
	static string to_string(u32 i);
	static string to_string(s32 i);
	static string to_string(u64 i);
	static string to_string(s64 i);
	static string to_string(u64 i, int base);

private:
	string(unsigned int new_size)
		: size_(new_size)
		, data_(new char_type[new_size + 1])
		, has_hash_(false)
		, hash_(0)
	{
		data_[size_] = 0;
	}

	/*
	 * Computes the FNV-1a hash
	 */
	hash_type compute_hash() const
	{
		// Offset basis for 64-bit hash
		u64 hash = 14695981039346656037ULL;

		for (unsigned int i = 0; i < size_; i++) {
			hash ^= data_[i];

			// FNV Prime for 64-bit hash
			hash *= 1099511628211ULL;
		}

		return (hash_type)hash;
	}

	// Holds the number of characters in the string.
	size_t size_;

	// Holds the string data + null byte.  This array is
	// always (size_ + 1) in length.
	char_type *data_;

	// Lazy hash computation.
	mutable bool has_hash_;
	mutable hash_type hash_;
};
} // namespace stacsos
