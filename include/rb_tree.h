#ifndef RB_TREE_H
#define RB_TREE_H

#include <utility>
#include <memory>

namespace rb_tree {

template <class T,
          class Compare = std::less<T>,
          class Alloc = std::allocator<T> >
class rb_tree {

 public:
  typedef T key_type;
  typedef T value_type;
  typedef Compare key_compare;
  typedef Compare value_compare;
  typedef Alloc allocator_type;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef ptrdiff_t difference_type;
  typedef size_t size_type;
  typedef typename std::allocator_traits<allocator_type>::pointer pointer;
  typedef typename std::allocator_traits<allocator_type>::const_pointer
    const_pointer;
  class iterator;
  typedef iterator const_iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef reverse_iterator const_reverse_iterator;

 protected:
  struct rb_tree_node;
  typedef rb_tree_node* node_ptr;

  enum rb_tree_color { red_, black_ };
  static constexpr node_ptr nil_ = 0;

  typedef std::allocator_traits<allocator_type> alloc_traits;
  typedef typename alloc_traits::template rebind_traits<struct rb_tree_node>
    node_alloc_traits;
  typename node_alloc_traits::allocator_type alloc_;

  key_compare comp_;

 public:
  std::pair <iterator, bool> insert(const value_type& val);
  iterator insert(const_iterator pos, const value_type& val);

  iterator erase(const_iterator pos);
  size_type erase(const key_type& val);
  iterator erase(const_iterator first, const_iterator last);

  iterator find (const key_type& val);

  iterator begin() { return iterator(begin_); }
  const_iterator begin() const { return iterator(begin_); }
  iterator end() { return iterator(end_); }
  const_iterator end() const { return iterator(end_); }
  const_iterator cbegin() const { return const_iterator(begin_); }
  const_iterator cend() const { return const_iterator(end_); }

  reverse_iterator rbegin() { return reverse_iterator(end()); }
  const_reverse_iterator rbegin() const { return reverse_iterator(end()); }
  reverse_iterator rend() { return reverse_iterator(begin()); }
  const_reverse_iterator rend() const { return reverse_iterator(begin()); }
  const_reverse_iterator crbegin() const { return reverse_iterator(end_); }
  const_reverse_iterator crend() const { return reverse_iterator(begin_); }

  size_type size() const { return size_; }
  bool empty() const { return size_ == 0; }

  rb_tree() : begin_(nil_), size_(0) {
    end_ = node_alloc_traits::allocate(alloc_, 1);
    end_->parent = end_;
    end_->left = nil_;
    end_->right = nil_;
    end_->color = black_;
  }

  class iterator : public std::iterator<std::bidirectional_iterator_tag,
                                        value_type,
                                        difference_type,
                                        const_pointer,
                                        const_reference> {
   public:
    const value_type& operator*() const { return ptr_->key; }
    const value_type* operator->() const { return &(operator*()); }

    iterator& operator++() {
      ptr_ = next_node(ptr_);
      return *this;
    }

    iterator operator++(int) {
      iterator tmp(ptr_);
      ++*this;
      return tmp;
    }

    iterator& operator--() {
      ptr_ = prev_node(ptr_);
      return *this;
    }

    iterator operator--(int) {
      iterator tmp(ptr_);
      --*this;
      return tmp;
    }

    inline bool operator==(const iterator& y) {
      return this->ptr_ == y.ptr_;
    }

    inline bool operator!=(const iterator& y) {
      return !(this->ptr_ == y.ptr_);
    }

    iterator() { }
    iterator(node_ptr x) { ptr_ = x; }

   private:
    node_ptr ptr_;
    friend class rb_tree;
  };

 protected:
  struct rb_tree_node {
    value_type key;
    rb_tree_color color;
    node_ptr parent;
    node_ptr left;
    node_ptr right;

    rb_tree_node(const value_type& val) : key(val) { }
  };

  /*
   * Both children of the end_ node point to the root node, and the parent
   * of the root node should always point to the end_ node.
   * Consequently, incrementing the rightmost node and decrementing the leftmost
   * node ends up with the end_ node.
   */
  node_ptr begin_;
  node_ptr end_;
  node_ptr root() { return end_->left; }

  size_type size_;

  node_ptr create_node() { return node_alloc_traits::allocate(alloc_, 1); }

  node_ptr create_node(const value_type& val) {
    node_ptr z = node_alloc_traits::allocate(alloc_, 1);
    z->key = val;
    z->left = nil_;
    z->right = nil_;
    return z;
  }

  void destroy_node(node_ptr x) {
    node_alloc_traits::deallocate(alloc_, x, 1);
  }

  std::pair <iterator, bool> insert_unqiue(const value_type& val);

  static node_ptr min_node(node_ptr x) {
    while (x->left != nil_)
      x = x->left;
    return x;
  }

  static node_ptr max_node(node_ptr x) {
    while (x->right != nil_)
      x = x->right;
    return x;
  }

  static node_ptr next_node(node_ptr x) {
    if (x->right != nil_) {
      x = x->right;
      while (x->left != nil_) {
        x = x->left;
      }
    } else {
      node_ptr tmp = x->parent;
      while (tmp->left != x) {
        x = tmp;
        tmp = x->parent;
      }
      x = tmp;
    }
    return x;
  }

  static node_ptr prev_node(node_ptr x) {
    if (x->left != nil_) {
      x = x->left;
      while (x->right != nil_) {
        x = x->right;
      }
    } else {
      node_ptr tmp = x->parent;
      while (tmp->right != x) {
        x = tmp;
        tmp = x->parent;
      }
      x = tmp;
    }
    return x;
  }

  void left_rotate(node_ptr x);
  void right_rotate(node_ptr x);
  void transplant(node_ptr u, node_ptr v);
  void erase_node(node_ptr z);

  void insert_fixup(node_ptr z);
  void erase_fixup(node_ptr x);
};

template <class T, class C, class A>
std::pair <typename rb_tree<T, C, A>::iterator, bool>
rb_tree<T, C, A>::insert(const value_type& val) {
  return insert_unqiue(val);
}

template <class T, class C, class A>
typename rb_tree<T, C, A>::iterator
rb_tree<T, C, A>::erase(const_iterator pos) {
  if (pos == iterator(end_))
    return end();

  iterator next = pos;
  ++next;

  --size_;

  erase_node(pos.ptr_);

  return next;
}

template <class T, class C, class A>
std::pair <typename rb_tree<T, C, A>::iterator, bool>
rb_tree<T, C, A>::insert_unqiue(const value_type& val) {
  node_ptr x = root();
  node_ptr y = end_;

  bool comp = true;

  while (x != nil_) {
    y = x;
    comp = comp_(val, x->key);
    x = comp ? x->left : x->right;
  }

  node_ptr j = y;
  if (comp) {
    /* left */

    if (y == begin_) {
      /* begin */
      node_ptr z = create_node(val);
      ++size_;

      begin_ = z;
      y->left = z;
      z->parent = y;

      insert_fixup(z);
      return std::pair<iterator, bool>(iterator(z), true);
    } else if (y == end_) {
      /* root */
      node_ptr z = create_node(val);
      ++size_;

      begin_ = z;
      y->right = z;
      y->left = z;
      z->parent = y;

      insert_fixup(z);
      return std::pair<iterator, bool>(iterator(z), true);
    } else {
      /* decrement j */
      node_ptr tmp = j->parent;
      while (j != tmp->right) {
        j = tmp;
        tmp = j->parent;
      }
      j = tmp;

      if (comp_(j->key, val)) {
        node_ptr z = create_node(val);
        ++size_;

        y->left = z;
        z->parent = y;

        insert_fixup(z);
        return std::pair<iterator, bool>(iterator(z), true);
      } else {
        /* duplicate */
        return std::pair<iterator, bool>(iterator(j), false);
      }
    }
  } else {
    /* right */

    if (comp_(j->key, val)) {
      node_ptr z = create_node(val);
      ++size_;

      y->right = z;
      z->parent = y;

      insert_fixup(z);
      return std::pair<iterator, bool>(iterator(z), true);
    } else {
      /* duplicate */
      return std::pair<iterator, bool>(iterator(j), false);
    }
  }

}

template <class T, class C, class A>
void rb_tree<T, C, A>::left_rotate(node_ptr x) {
  node_ptr y = x->right;

  x->right = y->left;
  if (y->left != nil_)
    y->left->parent = x;

  y->parent = x->parent;

  if (x->parent == end_) {
    /* root */
    end_->left = y;
    end_->right = y;
  } else if (x == x->parent->left) {
    x->parent->left = y;
  } else {
    x->parent->right = y;
  }

  y->left = x;
  x->parent = y;
}

template <class T, class C, class A>
void rb_tree<T, C, A>::right_rotate(node_ptr x) {
  node_ptr y = x->left;

  x->left = y->right;
  if (y->right != nil_)
    y->right->parent = x;

  y->parent = x->parent;

  if (x->parent == end_) {
    /* root */
    end_->left = y;
    end_->right = y;
  } else if (x == x->parent->right) {
    x->parent->right = y;
  } else {
    x->parent->left = y;
  }

  y->right = x;
  x->parent = y;
}

template <class T, class C, class A>
void rb_tree<T, C, A>::transplant(node_ptr u, node_ptr v) {
  if (u->parent == end_) {
    /* root */
    end_->left = v;
    end_->right = v;
  } else if (u == u->parent->left) {
    u->parent->left = v;
  } else {
    u->parent->right = v;
  }
  
  if (v != nil_)
    v->parent = u->parent;
}

template <class T, class C, class A>
void rb_tree<T, C, A>::insert_fixup(node_ptr z) {
  node_ptr y;

  z->color = red_;

  while (z->parent->color == red_) {
    if (z->parent == z->parent->parent->left) {
      y = z->parent->parent->right;

      if (y != nil_ && y->color == red_) {
        z->parent->color = black_;
        y->color = black_;
        z->parent->parent->color = red_;
        z = z->parent->parent;
      } else {
        if (z == z->parent->right) {
          z = z->parent;
          left_rotate(z);
        }
        z->parent->color = black_;
        z->parent->parent->color = red_;

        right_rotate(z->parent->parent);
      }
    } else {
      y = z->parent->parent->left;

      if (y != nil_ && y->color == red_) {
        z->parent->color = black_;
        y->color = black_;
        z->parent->parent->color = red_;
        z = z->parent->parent;
      } else {
        if (z == z->parent->left) {
          z = z->parent;
          right_rotate(z);
        }
        z->parent->color = black_;
        z->parent->parent->color = red_;

        left_rotate(z->parent->parent);
      }
    }
  }

  root()->color = black_;
}

template <class T, class C, class A>
void rb_tree<T, C, A>::erase_fixup(node_ptr x) {
  while (x != root() && x->color == black_) {
    if (x == x->parent->left) {
      node_ptr w = x->parent->right;
      if (w->color == red_) {
        w->color = black_;
        x->parent->color = red_;
        left_rotate(x->parent);
        w = x->parent->right;
      }
      if (w->left->color == black_ && w->right->color == black_) {
        w->color = red_;
        x = x->parent;
      } else {
        if (w->right->color == black_) {
          w->left->color = black_;
          w->color = red_;
          right_rotate(w);
          w = x->parent->right;
        }
        w->color = x->parent->color;
        x->parent->color = black_;
        w->right->color = black_;
        left_rotate(x->parent);
        x = root();
      }
    } else {
      node_ptr w = x->parent->left;
      if (w->color == red_) {
        w->color = black_;
        x->parent->color = red_;
        right_rotate(x->parent);
        w = x->parent->left;
      }
      if (w->right->color == black_ && w->left->color == black_) {
        w->color = red_;
        x = x->parent;
      } else {
        if (w->left->color == black_) {
          w->right->color = black_;
          w->color = red_;
          left_rotate(w);
          w = x->parent->left;
        }
        w->color = x->parent->color;
        x->parent->color = black_;
        x->left->color = black_;
        right_rotate(x->parent);
        x = root();
      }
    }
  }
  x->color = black_;
}

template <class T, class C, class A>
void rb_tree<T, C, A>::erase_node(node_ptr z) {
  node_ptr x;
  node_ptr y = z;
  int ycolor = y->color;

  if (z->left == nil_) {
    x = z->right;
    if (z == begin_)
      begin_ = next_node(z);
    transplant(z, z->right);
  } else if (z->right == nil_) {
    x = z->left;
    transplant(z, z->left);
  } else {
    y = min_node(z->right);
    ycolor = y->color;
    x = y->right;

    if (y->parent != z) {
      transplant(y, y->right);
      y->right = z->right;
      y->right->parent = y;
    } else {
      x->parent = y;
    }
    transplant(z, y);
    y->left = z->left;
    y->left->parent = y;
    y->color = z->color;
  }

  if (ycolor == black_)
    erase_fixup(x);

  destroy_node(z);
}

} // namespace rbtree

#endif // RB_TREE_H
