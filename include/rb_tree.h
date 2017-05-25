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
  typedef typename allocator_type::reference reference;
  typedef typename allocator_type::const_reference const_reference;
  typedef typename allocator_type::difference_type difference_type;
  typedef typename allocator_type::size_type size_type;
  typedef typename allocator_type::pointer pointer;
  typedef typename allocator_type::const_pointer const_pointer;

 protected:
  template <class Pointer, class Reference> class iterator_base;
  /* iterator_type is convertible to any other iterators */
  typedef iterator_base<value_type *, value_type &> iterator_type;
  typedef iterator_base<const value_type *, const value_type &>
    const_iterator_type;

 public:
  typedef const_iterator_type iterator;
  typedef const_iterator_type const_iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef reverse_iterator const_reverse_iterator;

 protected:
  struct rb_tree_node;
  typedef rb_tree_node* node_ptr;

  static constexpr node_ptr nil_ = 0;

 public:
  /*
   * constructors
   */

  // empty
  rb_tree()
    : rb_tree(key_compare(), allocator_type()) { }

  explicit rb_tree(const key_compare& comp,
                   const allocator_type& alloc = allocator_type())
    : size_(0),
      comp_(value_compare(comp)),
      alloc_(alloc),
      node_alloc_(node_allocator_type(alloc)) {
    end_ = &end_node_;
    end_->parent = end_;
    end_->left = nil_;
    end_->right = nil_;
    end_->color = black_;
    begin_ = end_;
  }

  explicit rb_tree(const allocator_type& alloc)
    : rb_tree(key_compare(), alloc) { }

  // range
  template <class InputIterator>
  rb_tree(InputIterator first, InputIterator last,
          const key_compare& comp = key_compare(),
          const allocator_type& alloc = allocator_type())
    : rb_tree(comp, alloc) { insert(first, last); }

  // copy
  rb_tree(const rb_tree& other) : rb_tree(other, other.alloc_) { }

  rb_tree(const rb_tree& other, const allocator_type& alloc)
    : alloc_(alloc),
      node_alloc_(node_allocator_type(alloc)) {
    copy_tree(other);
  }

  // move
  rb_tree(rb_tree&& other)
    : size_(other.size_),
      comp_(other.comp_),
      alloc_(other.alloc_),
      node_alloc_(node_allocator_type(other.alloc_)),
      begin_(other.begin_) {

    end_ = &end_node_;
    end_->parent = end_;
    end_->color = black_;
    end_->left = other.root();
    end_->right = other.root();
    if (root() != nil_)
      root()->parent = end_;

    other.end_->left = nil_;
    other.end_->right = nil_;
    other.begin_ = other.end_;
    other.size_ = 0;
  }

  rb_tree(rb_tree&& other, const allocator_type& alloc)
    : comp_(other.comp_),
      alloc_(alloc),
      node_alloc_(node_allocator_type(alloc)) {

    end_ = &end_node_;
    end_->parent = end_;
    end_->color = black_;

    if (alloc == other.alloc) {
      end_->left = other.root();
      end_->right = other.root();
      if (root() != nil_)
        root()->parent = end_;

      begin_ = other.begin_;
      size_ = other.size_;

      other.end_->left = nil_;
      other.end_->right = nil_;
      other.begin_ = other.end_;
      other.size_ = 0;
    } else {
      end_->left = nil_;
      end_->right = nil_;
      begin_ = end_;
      size_ = 0;
    }
  }

  // end of constructors

  /* destructor */
  ~rb_tree() { clear(); }

  void clear() noexcept { erase(begin(), end()); }

  std::pair <iterator, bool> insert(const value_type& val) {
    return insert_unique(val);
  }
  iterator insert(const_iterator pos, const value_type& val) {
    return insert_unique(pos.ptr_, val);
  }
  template <class InputIterator>
  void insert(InputIterator first, InputIterator last) {
    for (InputIterator it = first; it != last; ++it) {
      insert_unique(*it);
    }
  }
  void insert(std::initializer_list<value_type> il) {
    for (const value_type *it = il.begin(); it != il.end(); ++it) {
      insert_unique(*it);
    }
  }

  iterator erase(const_iterator pos) {
    return erase_iter(pos.ptr_);
  }
  size_type erase(const key_type& val) {
    return erase_unique(val);
  }
  iterator erase(const_iterator first, const_iterator last) {
    return erase_range(first.ptr_, last.ptr_);
  }

  iterator begin() { return iterator(begin_); }
  const_iterator begin() const { return const_iterator(begin_); }
  iterator end() { return iterator(end_); }
  const_iterator end() const { return const_iterator(end_); }
  const_iterator cbegin() const { return const_iterator(begin_); }
  const_iterator cend() const { return const_iterator(end_); }

  reverse_iterator rbegin() { return reverse_iterator(end()); }
  const_reverse_iterator rbegin() const {return const_reverse_iterator(end());}
  reverse_iterator rend() { return reverse_iterator(begin()); }
  const_reverse_iterator rend() const {return const_reverse_iterator(begin());}
  const_reverse_iterator crbegin() const {return const_reverse_iterator(end_);}
  const_reverse_iterator crend() const {return const_reverse_iterator(begin_);}

  iterator lower_bound(const value_type& val) {
    return lower_bound_unique(val);
  }
  const_iterator lower_bound(const value_type& val) const {
    return lower_bound_unique(val);
  }
  iterator upper_bound(const value_type& val) {
    return upper_bound_unique(val);
  }
  const_iterator upper_bound(const value_type& val) const {
    return upper_bound_unique(val);
  }

  iterator find(const value_type& val) {
    return find_unique(val);
  }
  const_iterator find(const value_type& val) const {
    return find_unique(val);
  }

  std::pair<iterator, iterator> equal_range(const value_type& val) {
    return equal_range_unique(val);
  }
  std::pair<const_iterator,const_iterator>
  equal_range(const value_type& val) const {
    return equal_range_unique(val);
  }

  size_type size() const { return size_; }
  bool empty() const { return size_ == 0; }

 protected:
  template <class Pointer, class Reference>
  class iterator_base : public std::iterator<std::bidirectional_iterator_tag,
                                        value_type,
                                        difference_type,
                                        Pointer,
                                        Reference> {
   public:
    Reference operator*() const { return ptr_->value; }
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

    /*
     * Both const_iterator and iterator can be converted from the iterator type
     */
    iterator_base(const iterator_base<value_type *, value_type &>& rhs)
      : ptr_(rhs.ptr_) { }

   protected:
    node_ptr ptr_;
    iterator_base(node_ptr rhs) : ptr_(rhs) { }
    friend class rb_tree;
  }; // iterator_base

  enum rb_tree_color { red_, black_ };

  struct rb_tree_node {
    value_type value;
    rb_tree_color color;
    node_ptr parent;
    node_ptr left;
    node_ptr right;

    rb_tree_node() { }
    rb_tree_node(const value_type& val) : value(val) { }
  }; // rb_tree_node

  typedef std::allocator_traits<allocator_type> alloc_traits;
  typedef typename alloc_traits::template rebind_traits<struct rb_tree_node>
    node_alloc_traits;
  typedef typename node_alloc_traits::allocator_type node_allocator_type;
  allocator_type alloc_;
  node_allocator_type node_alloc_;

  value_compare comp_;

  /*
   * Both children of the end_ node point to the root node, and the parent
   * of the root node should always point to the end_ node.
   * Consequently, incrementing the rightmost node and decrementing the leftmost
   * node ends up with the end_ node.
   */
  rb_tree_node end_node_;
  node_ptr begin_;
  node_ptr end_;
  node_ptr root() const { return end_->left; }
  void set_root(node_ptr ptr) {
    end_->left = ptr;
    end_->right = ptr;
    ptr->parent = end_;
  }

  size_type size_;

  /* Allocate space for an empty node */
  node_ptr create_node() { return node_alloc_traits::allocate(node_alloc_, 1); }

  /* Allocate space for a node and construct the value */
  node_ptr create_node(const value_type& val) {
    node_ptr z = create_node();
    alloc_traits::construct(alloc_, &z->value, val);
    z->left = nil_;
    z->right = nil_;
    return z;
  }

  /* Recycle the node. If the node has a constructed value, destruct it. */
  void destroy_node(node_ptr x, bool has_value = true) {
    if (has_value)
      alloc_traits::destroy(alloc_, &x->value);
    node_alloc_traits::deallocate(node_alloc_, x, 1);
  }

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

  void copy_tree(const rb_tree& other);

  void copy_tree(node_ptr src, node_ptr dst);

  std::pair <iterator_type, bool> insert_unique(const value_type& val);
  iterator_type insert_unique(node_ptr pos, const value_type& val);
  iterator_type erase_iter(node_ptr pos);
  iterator_type erase_range(node_ptr first, node_ptr last);
  size_type erase_unique(const value_type& val);

  void left_rotate(node_ptr x);
  void right_rotate(node_ptr x);
  void transplant(node_ptr u, node_ptr v);
  void erase_node(node_ptr z);

  void insert_fixup(node_ptr z);
  void erase_fixup(node_ptr x, node_ptr xparent);

  iterator_type lower_bound_unique(const key_type& val);
  iterator_type upper_bound_unique(const key_type& val);
  iterator_type find_unique(const key_type& val);

  std::pair<iterator_type, iterator_type>
  equal_range_unique(const key_type& val);

};

template <class T, class C, class A>
void rb_tree<T, C, A>::copy_tree(const rb_tree& other) {
  size_ = other.size_;
  comp_ = other.comp_;

  end_ = &end_node_;
  end_->parent = end_;
  end_->color = black_;

  /* root */
  if (other.root() != nil_) {
    set_root(create_node(other.root()->value));
    root()->color = other.root()->color;
    copy_tree(other.root(), root());
  } else {
    end_->left = nil_;
    end_->right = nil_;
  }

  begin_ = min_node(root());

}

template <class T, class C, class A>
void rb_tree<T, C, A>::copy_tree(node_ptr src, node_ptr dst) {
  /*
   * assume src != nil_
   * and dst has been constructed
   */
  if (src->left != nil_) {
    dst->left = create_node(src->left->value);
    dst->left->color = src->left->color;
    dst->left->parent = dst;
    copy_tree(src->left, dst->left);
  } else {
    dst->left = nil_;
  }

  if (src->right != nil_) {
    dst->right = create_node(src->right->value);
    dst->right->color = src->right->color;
    dst->right->parent = dst;
    copy_tree(src->right, dst->right);
  } else {
    dst->right = nil_;
  }

}

template <class T, class C, class A>
std::pair <typename rb_tree<T, C, A>::iterator_type, bool>
rb_tree<T, C, A>::insert_unique(const value_type& val) {
  node_ptr x = root();
  node_ptr y = end_;

  bool comp = true;

  while (x != nil_) {
    y = x;
    comp = comp_(val, x->value);
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
      set_root(z);

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

      if (comp_(j->value, val)) {
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

    if (comp_(j->value, val)) {
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
      set_root(z);

      insert_fixup(z);
      return iterator_type(z);
    } else {
      node_ptr prev = prev_node(pos);

      if (comp_(prev->value, val)) {
        /* prev < val, correct hint
         * The rightmost node should not have a right child, so making the new
         * node as the right child would be safe. */
        node_ptr z = create_node(val);
        ++size_;

        prev->right = z;
        z->parent = prev;

        insert_fixup(z);
        return iterator_type(z);
      } else {
        /* incorrect hint */
        return insert_unique(val).first;
      }
    }
  } else if (pos == begin_) {
    if (comp_(val, pos->value)) {
      /* begin */
      node_ptr z = create_node(val);
      ++size_;

      begin_->left = z;
      z->parent = begin_;
      begin_ = z;

      insert_fixup(z);
      return iterator_type(z);
    } else {
      /* incorrect hint */
      return insert_unique(val).first;
    }
  } else {
    node_ptr prev = prev_node(pos);

    if (comp_(prev->value, val) && comp_(val, pos->value)) {
      /* prev < val < pos, correct hint */
      if (prev->right == nil_) {
        /* prev has no right child */
        node_ptr z = create_node(val);
        ++size_;

        prev->right = z;
        z->parent = prev;

        insert_fixup(z);
        return iterator_type(z);
      } else {
        /* prev has a right child, then the left child of pos is nil */
        node_ptr z = create_node(val);
        ++size_;

        pos->left = z;
        z->parent = pos;

        insert_fixup(z);
        return iterator_type(z);
      }
    } else {
      /* incorrect hint */
      return insert_unique(val).first;
    }
  }
}

/*
 * Return the iterator that follows the erased one
 */
template <class T, class C, class A>
typename rb_tree<T, C, A>::iterator_type
rb_tree<T, C, A>::erase_iter(node_ptr pos) {
  if (pos == end_)
    return iterator_type(end_);

  iterator_type next(pos);
  ++next;

  erase_node(pos);

  return next;
}

/*
 * Erase the range [first, last)
 * Return the iterator that follows the last erased one
 */
template <class T, class C, class A>
typename rb_tree<T, C, A>::iterator_type
rb_tree<T, C, A>::erase_range(node_ptr first, node_ptr last) {
  if (first == end_)
    return iterator_type(end_);

  for (node_ptr now = first, next; now != last; now = next) {
    next = next_node(now);
    erase_node(now);
  }

  return last;
}

/*
 * Return the number of elements erased
 * Since the elements are all unique, the return value is either 0 or 1
 */
template <class T, class C, class A>
typename rb_tree<T, C, A>::size_type
rb_tree<T, C, A>::erase_unique(const value_type& val) {
  iterator_type j = lower_bound_unique(val);

  if (j.ptr_ == end_ || comp_(val, *j)) {
    return 0;
  } else {
    erase_node(j.ptr_);
    return 1;
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
    set_root(y);
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
    set_root(y);
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
    /* set root */
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

  --size_;

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

/*
 * Return the iterator of the first element no less than val
 */
template <class T, class C, class A>
typename rb_tree<T, C, A>::iterator_type
rb_tree<T, C, A>::lower_bound_unique(const key_type& val) {
  node_ptr y = end_;

  for (node_ptr x = root(); x != nil_;) {
    if (comp_(x->value, val)) {
      // x < val
      x = x->right;
    } else {
      // val <= x
      y = x;
      x = x->left;
    }
  }
  return iterator_type(y);
}

/*
 * Return the iterator of the first element strictly greater than val
 */
template <class T, class C, class A>
typename rb_tree<T, C, A>::iterator_type
rb_tree<T, C, A>::upper_bound_unique(const key_type& val) {
  node_ptr y = end_;

  for (node_ptr x = root(); x != nil_;) {
    if (comp_(val, x->value)) {
      // val < x
      y = x;
      x = x->left;
    } else {
      // x <= val
      x = x->right;
    }
  }
  return iterator_type(y);
}

/*
 * If the element is found, return the iterator to the element.
 * Otherwise return end()
 */
template <class T, class C, class A>
typename rb_tree<T, C, A>::iterator_type
rb_tree<T, C, A>::find_unique(const key_type& val) {
  iterator_type j = lower_bound_unique(val);

  if (j.ptr_ == end_ || comp_(val, *j))
    return iterator_type(end_);
  else
    return j;
}

/*
 * Return the pair (lower_bound, upper_bound)
 * However we can skip calculating the upper_bound since the elements in the
 * tree are all unique.
 */
template <class T, class C, class A>
std::pair<typename rb_tree<T, C, A>::iterator_type,
          typename rb_tree<T, C, A>::iterator_type>
rb_tree<T, C, A>::equal_range_unique(const key_type& val) {
  iterator_type j = lower_bound_unique(val);

  if (j.ptr_ == end_ || comp_(val, *j))
    return std::pair<iterator_type,iterator_type>(j, j);
  else
    return std::pair<iterator_type,iterator_type>(j, std::next(j));
}
} // namespace rbtree

#endif // RB_TREE_H
