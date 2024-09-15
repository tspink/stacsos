/* SPDX-License-Identifier: MIT */

/* StACSOS - Utility Library
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

#include <stacsos/list.h>

namespace stacsos {
template <class K, class D> class avl_tree_node {
	DELETE_DEFAULT_COPY_AND_MOVE(avl_tree_node)

public:
	using key_type = K;
	using data_type = D;

	avl_tree_node(const K &key, const D &data)
		: key_(key)
		, data_(data)
		, left_(nullptr)
		, right_(nullptr)
	{
	}

	int height() const
	{
		int lh = left_ == nullptr ? 0 : left_->height();
		int rh = right_ == nullptr ? 0 : right_->height();

		return max(lh, rh) + 1;
	}

	int balance_factor() const
	{
		int lh = left_ == nullptr ? 0 : left_->height();
		int rh = right_ == nullptr ? 0 : right_->height();

		return lh - rh;
	}

	const K &key() const { return key_; }
	const D &data() const { return data_; }

	avl_tree_node *left() const { return left_; }
	avl_tree_node *right() const { return right_; }

	void left(avl_tree_node *n) { left_ = n; }
	void right(avl_tree_node *n) { right_ = n; }

private:
	K key_;
	D data_;

	avl_tree_node *left_, *right_;
};

template <class N> struct avl_tree_iterator_pair {
	using node_type = N;

	avl_tree_iterator_pair(const N &node)
		: key(node.key())
		, value(node.data())
	{
	}

	const typename node_type::key_type &key;
	const typename node_type::data_type &value;
};

template <class N> class avl_tree_iterator {
public:
	using node_type = N;
	using this_type = avl_tree_iterator<node_type>;
	using pair_type = avl_tree_iterator_pair<node_type>;

	avl_tree_iterator(node_type *start)
		: cur_(nullptr)
	{
		if (start) {
			enqueue(start);
			advance();
		}
	}

	const pair_type operator*() const { return pair_type(*cur_); }

	void operator++() { advance(); }

	bool operator==(const this_type &other) const { return other.cur_ == cur_; }

	bool operator!=(const this_type &other) const { return other.cur_ != cur_; }

private:
	list<node_type *> queue_;
	node_type *cur_;

	void enqueue(node_type *n)
	{
		if (n) {
			queue_.enqueue(n);
		}
	}

	void advance()
	{
		if (!queue_.empty()) {
			cur_ = queue_.dequeue();

			enqueue(cur_->left());
			enqueue(cur_->right());
		} else {
			cur_ = nullptr;
		}
	}
};

template <class K, class D> class avl_tree {
	DELETE_DEFAULT_COPY_AND_MOVE(avl_tree)

public:
	using node = avl_tree_node<K, D>;
	using const_iterator = const avl_tree_iterator<node>;

	avl_tree()
		: root_(nullptr)
	{
	}

	void add(const K &key, const D &data) { root_ = do_insert(root_, key, data); }

	bool try_get_value(const K &key, D &data)
	{
		node *ref = root_;
		while (ref) {
			if (ref->key() == key) {
				data = ref->data();
				return true;
			} else if (key < ref->key()) {
				ref = ref->left();
			} else {
				ref = ref->right();
			}
		}

		return false;
	}

	void dump() const { do_dump(root_); }

	const_iterator begin() const { return const_iterator(root_); }
	const_iterator end() const { return const_iterator(nullptr); }

private:
	node *root_;

	node *alloc_node(const K &key, const D &data) { return new node(key, data); }

	node *ll_rot(node *ref)
	{
		node *t = ref->left();
		ref->left(t->right());
		t->right(ref);
		return t;
	}

	node *lr_rot(node *ref)
	{
		node *t = ref->left();
		ref->left(rr_rot(t));
		return ll_rot(ref);
	}

	node *rl_rot(node *ref)
	{
		node *t = ref->right();
		ref->right(ll_rot(t));
		return rr_rot(ref);
	}

	node *rr_rot(node *ref)
	{
		node *t = ref->right();
		ref->right(t->left());
		t->left(ref);

		return t;
	}

	node *balance(node *ref)
	{
		int bf = ref->balance_factor();
		if (bf > 1) {
			if (ref->left()->balance_factor() > 0) {
				return ll_rot(ref);
			} else {
				return lr_rot(ref);
			}
		} else if (bf < -1) {
			if (ref->right()->balance_factor() > 0) {
				return rl_rot(ref);
			} else {
				return rr_rot(ref);
			}
		}

		return ref;
	}

	node *do_insert(node *ref, const K &key, const D &data)
	{
		if (ref == nullptr) {
			return alloc_node(key, data);
		} else if (key < ref->key()) {
			ref->left(do_insert(ref->left(), key, data));
			return balance(ref);
		} else {
			ref->right(do_insert(ref->right(), key, data));
			return balance(ref);
		}
	}
};
} // namespace stacsos
