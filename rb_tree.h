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
  static constexpr node_ptr nil = 0;

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

  rb_tree() : begin_(nil), size_(0) {
    end_ = node_alloc_traits::allocate(alloc_, 1);
    end_->parent = end_;
    end_->left = nil;
    end_->right = nil;
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
      if (ptr_->right != nil) {
        ptr_ = ptr_->right;
        while (ptr_->left != nil) {
          ptr_ = ptr_->left;
        }
      } else {
        node_ptr tmp = ptr_->parent;
        while (tmp->left != ptr_) {
          ptr_ = tmp;
          tmp = ptr_->parent;
        }
        ptr_ = tmp;
      }
      return *this;
    }

    iterator operator++(int) {
      iterator tmp(ptr_);
      ++*this;
      return tmp;
    }

    iterator& operator--() {
      if (ptr_->left != nil) {
        ptr_ = ptr_->left;
        while (ptr_->right != nil) {
          ptr_ = ptr_->right;
        }
      } else {
        node_ptr tmp = ptr_->parent;
        while (tmp->right != ptr_) {
          ptr_ = tmp;
          tmp = ptr_->parent;
        }
        ptr_ = tmp;
      }
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
    z->left = nil;
    z->right = nil;
    return z;
  }

  static node_ptr min_node(node_ptr x);
  static node_ptr max_node(node_ptr x);

  void insert_fixup(node_ptr y, node_ptr z);
  std::pair <iterator, bool> insert_unqiue(const value_type& val);
};

template <class T, class C, class A>
std::pair <typename rb_tree<T, C, A>::iterator, bool>
rb_tree<T, C, A>::insert(const value_type& val) {
  return insert_unqiue(val);
}

template <class T, class C, class A>
std::pair <typename rb_tree<T, C, A>::iterator, bool>
rb_tree<T, C, A>::insert_unqiue(const value_type& val) {
  node_ptr x = root();
  node_ptr y = end_;

  ++size_;

  bool comp = true;

  while (x != nil) {
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

      begin_ = z;
      y->left = z;
      z->parent = y;

      return std::pair<iterator, bool>(iterator(z), true);
    } else if (y == end_) {
      /* root */
      node_ptr z = create_node(val);

      begin_ = z;
      y->right = z;
      y->left = z;
      z->parent = y;

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
        y->left = z;
        z->parent = y;

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
      y->right = z;
      z->parent = y;

      return std::pair<iterator, bool>(iterator(z), true);
    } else {
      /* duplicate */
      return std::pair<iterator, bool>(iterator(j), false);
    }
  }

}

template <class T, class C, class A>
void rb_tree<T, C, A>::insert_fixup(typename rb_tree<T, C, A>::node_ptr y,
                                    typename rb_tree<T, C, A>::node_ptr z) {
  z->color = red_;

  return z;
}


} // namespace rbtree

#endif // RB_TREE_H
