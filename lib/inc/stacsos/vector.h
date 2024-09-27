#pragma once

namespace stacsos {
template <class T> class vector {
public:
	vector()
		: storage_(new T[0])
		, size_(0)
	{
	}

	explicit vector(u32 capacity)
		: storage_(new T[capacity])
		, size_(capacity)
	{
	}

	vector(vector &&o)
		: storage_(o.storage_)
		, size_(o.size_)
	{
		o.storage_ = nullptr;
		o.size_ = 0;
	}

	vector(const vector &o)
		: storage_(new T[o.size_])
		, size_(o.size_)
	{
		for (unsigned long i = 0; i < size_; i++) {
			storage_[i] = o.storage_[i];
		}
	}

	~vector() { delete[] storage_; }

	vector(T *data, size_t size)
		: storage_(data)
		, size_(size)
	{
	}

	vector &operator=(vector v) = delete;

	const T *data() const { return storage_; }
	T *data() { return storage_; }

	size_t size() const { return size_; }

	void resize(size_t new_size)
	{
		T *new_storage_ = new T[new_size];
		for (unsigned long i = 0; i < size_; i++) {
			new_storage_[i] = storage_[i];
		}

		T *tmp = storage_;
		storage_ = new_storage_;
		delete tmp;

		size_ = new_size;
	}

	T &operator[](size_t index) { return storage_[index]; }

private:
	T *storage_;
	size_t size_;
};
} // namespace stacsos
