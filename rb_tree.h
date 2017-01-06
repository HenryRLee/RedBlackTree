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
  typedef typename std::allocator_traits<allocator_type>::pointer pointer;
  typedef typename std::allocator_traits<allocator_type>::const_pointer
    const_pointer;
  class iterator;
  typedef iterator const_iterator;
  class reverse_iterator;
  typedef reverse_iterator const_reverse_iterator;
  typedef ptrdiff_t difference_type;
  typedef size_t size_type;

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
  const_iterator cbegin() const;
  const_iterator cend() const;

  reverse_iterator rbegin();
  const_reverse_iterator rbegin() const;
  reverse_iterator rend();
  const_reverse_iterator rend() const;
  const_reverse_iterator crbegin() const;
  const_reverse_iterator crend() const;

  size_type size() const { return size_; }
  bool empty() const { return size_ == 0; }

  rb_tree() : root_(nil), begin_(nil), size_(0) {
    end_ = node_alloc_traits::allocate(alloc_, 1);
    end_->parent = end_;
    end_->left = root_;
    end_->right = root_;
  }

  class iterator {
   public:
    const T& operator*() const { return ptr_->key; }
    const T* operator->() const { return &(operator*()); }

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

    iterator& operator--();
    iterator operator--(int);

    friend inline bool operator==(const iterator& x, const iterator& y) {
      return x.ptr_ == y.ptr_;
    }

    friend inline bool operator!=(const iterator& x, const iterator& y) {
      return !(x.ptr_ == y.ptr_);
    }

    iterator() { }
    iterator(node_ptr x) { ptr_ = x; }

   private:
    node_ptr ptr_;
  };

  class reverse_iterator : public iterator {
   public:
    iterator& operator++() { return iterator::operator--(); }
    iterator operator++(int x) { return iterator::operator--(x); }
    iterator& operator--() { return iterator::operator++(); }
    iterator operator--(int x) { return iterator::operator++(x); }
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

  node_ptr root_;
  node_ptr begin_;
  node_ptr end_;
  size_type size_;

  static node_ptr MinimumNode(node_ptr x);
  static node_ptr MaximumNode(node_ptr x);

  static node_ptr SuccessorNode(node_ptr x);
  static node_ptr PredecessorNode(node_ptr x);

 private:
  void InsertFixup(node_ptr y, node_ptr z);
  void DeleteNode(node_ptr x);

  void Transplant(node_ptr u, node_ptr v);
  void LeftRotate(node_ptr x);
  void RightRotate(node_ptr x);
  void DeleteFixup(node_ptr x);

};

template <class T, class C, class A>
std::pair <typename rb_tree<T, C, A>::iterator, bool>
rb_tree<T, C, A>::insert(const value_type& val) {
  node_ptr z = node_alloc_traits::allocate(alloc_, 1);
  node_ptr x = root_;
  node_ptr y = end_;

  z->key = val;
  z->left = nil;
  z->right = nil;

  ++size_;

  bool comp = true;

  while (x != nil) {
    y = x;
    comp = comp_(val, x->key);
    x = comp ? x->left : x->right;
  }

  node_ptr j = y;
  if (comp) {
    y->left = z;
    z->parent = y;

    if (y == begin_) {
      /* begin */
      begin_ = z;

      return std::pair<iterator, bool>(iterator(z), true);
    } else if (y == end_) {
      /* root */
      root_ = z;
      y->right = z;
      begin_ = z;

      return std::pair<iterator, bool>(iterator(z), true);
    } else {
      /* decrement j */
      node_ptr tmp = j->parent;
      while (j == tmp->left) {
        j = tmp;
        tmp = j->parent;
      }
    }
  } else {
    y->right = z;
    z->parent = y;
  }

  if (comp_(j->key, val))
    return std::pair<iterator, bool>(iterator(z), true);
  else
    return std::pair<iterator, bool>(iterator(j), false);

}

template <class T, class C, class A>
void rb_tree<T, C, A>::InsertFixup(typename rb_tree<T, C, A>::node_ptr y,
                                   typename rb_tree<T, C, A>::node_ptr z) {
  z->color = red_;

  return z;
}


} // namespace rbtree

#endif // RB_TREE_H
