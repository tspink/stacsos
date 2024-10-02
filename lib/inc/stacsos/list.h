/* SPDX-License-Identifier: MIT */

/* StACSOS - Utility Library
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

namespace stacsos {
template <typename T> struct list_node {
	typedef T elem;
	typedef list_node<T> self;

	list_node(const elem &d)
		: data(d)
	{
	}

	self *next;
	elem data;
};

template <typename T> struct list_iterator {
	typedef T elem;
	typedef list_iterator<elem> self;
	typedef list_node<elem> node;

	list_iterator(node *current)
		: current_(current)
	{
	}

	list_iterator(const self &other)
		: current_(other.current_)
	{
	}

	list_iterator(self &&other)
		: current_(other.current_)
	{
		other.current_ = nullptr;
	}

	const elem &operator*() const { return current_->data; }

	void operator++()
	{
		if (current_)
			current_ = current_->next;
	}

	bool operator==(const self &other) const { return current_ == other.current_; }
	bool operator!=(const self &other) const { return current_ != other.current_; }

private:
	node *current_;
};

template <typename T> class list {
public:
	typedef T elem;
	typedef const T const_elem;

	typedef list<elem> self;
	typedef list_node<elem> node;
	typedef list_iterator<elem> iterator;

	list()
		: elems_(nullptr)
		, count_(0)
	{
	}

	// Copy

	list(const self &r)
		: elems_(nullptr)
		, count_(0)
	{
		for (const auto &elem : r) {
			append(elem);
		}
	}

	// Move

	list(self &&r)
		: elems_(r.elems_)
		, count_(r.count_)
	{
		r.elems_ = nullptr;
		r.count_ = 0;
	}

	~list()
	{
		node *slot = elems_;
		while (slot) {
			node *next = slot->next;
			delete slot;
			slot = next;
		}
	}

	void append(elem const &elem)
	{
		node **slot = &elems_;

		while (*slot != nullptr) {
			slot = &(*slot)->next;
		}

		*slot = new node(elem);
		(*slot)->next = nullptr;

		count_++;
	}

	void remove(elem const &elem)
	{
		node **slot = &elems_;

		while (*slot && (*slot)->data != elem) {
			slot = &(*slot)->next;
		}

		if (*slot) {
			node *candidate = *slot;

			*slot = candidate->next;

			delete candidate;
			count_--;
		}
	}

	elem dequeue()
	{
		node *front = elems_;

		elem ret = front->data;
		elems_ = front->next;
		delete front;
		count_--;

		return ret;
	}

	void enqueue(elem const &elem) { append(elem); }

	void push(elem const &elem)
	{
		node *slot = new node(elem);
		slot->next = elems_;
		elems_ = slot;

		count_++;
	}

	elem pop() { return dequeue(); }

	elem rotate()
	{
		node *front = elems_;
		elems_ = front->next;

		node **slot = &elems_;
		while (*slot != nullptr) {
			slot = &(*slot)->next;
		}

		*slot = front;
		(*slot)->next = nullptr;

		return front->data;
	}

	elem const &first() const { return elems_->data; }

	elem const &last() const
	{
		node *last = elems_;
		while (last->next) {
			last = last->next;
		}

		return last->data;
	}

	elem const &at(int index) const
	{
		if (index >= count_) {
			panic("argument %d out of range %d", index, count_);
		}

		int ctr = 0;
		node *n = elems_;

		while (ctr != index && n) {
			n = n->next;
			ctr++;
		}

		if (!n) {
			panic("end of list");
		}

		return n->data;
	}

	unsigned int count() const { return count_; }

	bool empty() const { return count_ == 0; }

	void clear()
	{
		node *cur = elems_;
		while (cur) {
			node *tmp = cur;
			cur = tmp->next;
			delete tmp;
		}

		elems_ = nullptr;
		count_ = 0;
	}

	iterator begin() const { return iterator(elems_); }
	iterator end() const { return iterator(nullptr); }

private:
	node *elems_;
	int count_;
};
} // namespace stacsos
