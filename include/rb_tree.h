#ifndef RB_TREE_H
#define RB_TREE_H

#include <utility>
#include <memory>

namespace rb_tree {

template <class T,
          class Compare = std::less<T>,
          class Alloc = std::allocator<T> >
class rb_tree {
 protected:
  class iterator_type;
  class const_iterator_type;

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
  typedef const_iterator_type iterator;
  typedef const_iterator_type const_iterator;
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

  rb_tree() : size_(0) {
    end_ = node_alloc_traits::allocate(alloc_, 1);
    end_->parent = end_;
    end_->left = nil_;
    end_->right = nil_;
    end_->color = black_;
    begin_ = end_;
  }

 protected:
  template <class Pointer, class Reference>
  class iterator_base : public std::iterator<std::bidirectional_iterator_tag,
                                        value_type,
                                        difference_type,
                                        Pointer,
                                        Reference> {
   public:
    Reference operator*() const { return ptr_->key; }
    Pointer operator->() const { return &(operator*()); }

    iterator_base& operator++() {
      ptr_ = next_node(ptr_);
      return *this;
    }

    iterator_base operator++(int) {
      iterator_base tmp(ptr_);
      ++*this;
      return tmp;
    }

    iterator_base& operator--() {
      ptr_ = prev_node(ptr_);
      return *this;
    }

    iterator_base operator--(int) {
      iterator_base tmp(ptr_);
      --*this;
      return tmp;
    }

    inline bool operator==(const iterator_base& y) {
      return this->ptr_ == y.ptr_;
    }

    inline bool operator!=(const iterator_base& y) {
      return !(this->ptr_ == y.ptr_);
    }

    iterator_base() { }

   protected:
    node_ptr ptr_;
    iterator_base(node_ptr rhs) { ptr_ = rhs; }
  }; // iterator

  class iterator_type : public iterator_base <value_type *, value_type &> {
  public:
    iterator_type() { }
  protected:
    iterator_type(node_ptr rhs) { this->ptr_ = rhs; }
    friend class rb_tree;
  };

  class const_iterator_type : public iterator_base <const value_type *,
                                                    const value_type &> {
  public:
    const_iterator_type() { }
    const_iterator_type(const iterator_type &rhs) { this->ptr_ = rhs.ptr_; }
  protected:
    const_iterator_type(node_ptr rhs) { this->ptr_ = rhs; }
    friend class rb_tree;
  };

  struct rb_tree_node {
    value_type key;
    rb_tree_color color;
    node_ptr parent;
    node_ptr left;
    node_ptr right;

    rb_tree_node(const value_type& val) : key(val) { }
  }; // rb_tree_node

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

  std::pair <iterator_type, bool> insert_unique(const value_type& val);
  iterator_type insert_unique(node_ptr pos, const value_type& val);

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
  void erase_fixup(node_ptr x, node_ptr xparent);
};

template <class T, class C, class A>
std::pair <typename rb_tree<T, C, A>::iterator, bool>
rb_tree<T, C, A>::insert(const value_type& val) {
  return insert_unique(val);
}

template <class T, class C, class A>
typename rb_tree<T, C, A>::iterator
rb_tree<T, C, A>::insert(const_iterator pos, const value_type& val) {
  return insert_unique(pos.ptr_, val);
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
std::pair <typename rb_tree<T, C, A>::iterator_type, bool>
rb_tree<T, C, A>::insert_unique(const value_type& val) {
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

    if (y == end_) {
      /* root */
      node_ptr z = create_node(val);
      ++size_;

      begin_ = z;
      end_->left = z;
      end_->right = z;
      z->parent = end_;

      insert_fixup(z);
      return std::pair<iterator_type, bool>(z, true);
    } else if (y == begin_) {
      /* begin */
      node_ptr z = create_node(val);
      ++size_;

      begin_ = z;
      y->left = z;
      z->parent = y;

      insert_fixup(z);
      return std::pair<iterator_type, bool>(z, true);
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
        return std::pair<iterator_type, bool>(z, true);
      } else {
        /* duplicate */
        return std::pair<iterator_type, bool>(j, false);
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
      return std::pair<iterator_type, bool>(z, true);
    } else {
      /* duplicate */
      return std::pair<iterator_type, bool>(j, false);
    }
  }

}

/*
 * insert function with a hint iterator
 * If the hint is correct, the insertion can be done in constant time.
 * Otherwise, call the basic insert function.
 * According to C++11, the hint iterator follows the element being inserted.
 */
template <class T, class C, class A>
typename rb_tree<T, C, A>::iterator_type
rb_tree<T, C, A>::insert_unique(node_ptr pos, const value_type& val) {
  if (pos == end_) {
    if (pos == begin_) {
      /* root */
      node_ptr z = create_node(val);
      ++size_;

      begin_ = z;
      end_->left = z;
      end_->right = z;
      z->parent = end_;

      insert_fixup(z);
      return iterator(z);
    } else {
      node_ptr prev = prev_node(pos);

      if (comp_(prev->key, val)) {
        /* prev < val, correct hint
         * The rightmost node should not have a right child, so making the new
         * node as the right child would be safe. */
        node_ptr z = create_node(val);
        ++size_;

        prev->right = z;
        z->parent = prev;

        insert_fixup(z);
        return iterator(z);
      } else {
        /* incorrect hint */
        return insert_unique(val).first;
      }
    }
  } else if (pos == begin_) {
    if (comp_(val, pos->key)) {
      /* begin */
      node_ptr z = create_node(val);
      ++size_;

      begin_->left = z;
      z->parent = begin_;
      begin_ = z;

      insert_fixup(z);
      return iterator(z);
    } else {
      /* incorrect hint */
      return insert_unique(val).first;
    }
  } else {
    node_ptr prev = prev_node(pos);

    if (comp_(prev->key, val) && comp_(val, pos->key)) {
      /* prev < val < pos, correct hint */
      if (prev->right == nil_) {
        /* prev has no right child */
        node_ptr z = create_node(val);
        ++size_;

        prev->right = z;
        z->parent = prev;

        insert_fixup(z);
        return iterator(z);
      } else {
        /* prev has a right child, then the left child of pos is nil */
        node_ptr z = create_node(val);
        ++size_;

        pos->left = z;
        z->parent = pos;

        insert_fixup(z);
        return iterator(z);
      }
    } else {
      /* incorrect hint */
      return insert_unique(val).first;
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
void rb_tree<T, C, A>::erase_fixup(node_ptr x, node_ptr xparent) {
  while (x != root() && (x == nil_ || x->color == black_)) {
    if (x == xparent->left) {
      node_ptr w = xparent->right;
      if (w->color == red_) {
        w->color = black_;
        xparent->color = red_;
        left_rotate(xparent);
        w = xparent->right;
      }
      if ((w->left == nil_ || w->left->color == black_) && 
          (w->right == nil_ || w->right->color == black_)) {
        w->color = red_;
        x = xparent;
        xparent = xparent->parent;
      } else {
        if (w->right == nil_ || w->right->color == black_) {
          if (w->left != nil_)
            w->left->color = black_;
          w->color = red_;
          right_rotate(w);
          w = xparent->right;
        }
        w->color = xparent->color;
        xparent->color = black_;
        if (w->right != nil_)
          w->right->color = black_;
        left_rotate(xparent);
        x = root();
      }
    } else {
      node_ptr w = xparent->left;
      if (w->color == red_) {
        w->color = black_;
        xparent->color = red_;
        right_rotate(xparent);
        w = xparent->left;
      }
      if ((w->left == nil_ || w->left->color == black_) && 
          (w->right == nil_ || w->right->color == black_)) {
        w->color = red_;
        x = xparent;
        xparent = xparent->parent;
      } else {
        if (w->left == nil_ || w->left->color == black_) {
          if (w->right != nil_)
            w->right->color = black_;
          w->color = red_;
          left_rotate(w);
          w = xparent->left;
        }
        w->color = xparent->color;
        xparent->color = black_;
        if (w->left != nil_)
          w->left->color = black_;
        right_rotate(xparent);
        x = root();
      }
    }
  }
  if (x != nil_)
    x->color = black_;
}

template <class T, class C, class A>
void rb_tree<T, C, A>::erase_node(node_ptr z) {
  node_ptr x;
  node_ptr xparent;
  node_ptr y = z;
  int ycolor = y->color;

  if (z->left == nil_) {
    x = z->right;
    if (z == begin_)
      begin_ = next_node(z);
    transplant(z, z->right);
    xparent = y->parent;
  } else if (z->right == nil_) {
    x = z->left;
    transplant(z, z->left);
    xparent = y->parent;
  } else {
    y = min_node(z->right);
    ycolor = y->color;
    x = y->right;
    if (y->parent != z) {
      transplant(y, y->right);
      y->right = z->right;
      y->right->parent = y;
      xparent = y->parent;
    } else {
      xparent = y;
    }
    transplant(z, y);
    y->left = z->left;
    y->left->parent = y;
    y->color = z->color;
  }

  if (ycolor == black_)
    erase_fixup(x, xparent);

  destroy_node(z);
}

} // namespace rbtree

#endif // RB_TREE_H
