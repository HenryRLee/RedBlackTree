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
  typedef std::allocator_traits<allocator_type>::pointer pointer;
  typedef std::allocator_traits<allocator_type>::const_pointer const_pointer;
  class iterator;
  typedef iterator const_iterator;
  class reverse_iterator;
  typedef reverse_iterator const_reverse_iterator;
  typedef ptrdiff_t difference_type;
  typedef size_t size_type;

 protected:
  struct rb_tree_node;
  typedef rb_tree_node* node_ptr;

  typedef bool rb_tree_color;

  typedef std::allocator_traits<allocator_type> alloc_traits;
  typedef typename alloc_traits::template rebind_traits<struct rb_tree_node> node_alloc_traits;

 public:
  std::pair <iterator, bool> insert(const value_type& val);
  iterator insert(const_iterator pos, const value_type& val);

  iterator erase(const_iterator pos);
  size_type erase(const key_type& val);
  iterator erase(const_iterator first, const_iterator last);

  iterator find (const key_type& val);

  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;
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

  rb_tree() : root_(nil), size_(0) { }

  class iterator {
   public:
	const T& operator*() const { return ptr_->key; }
	const T* operator->() const { return &(operator*()); }

	iterator& operator++();
	iterator operator++(int);
	iterator& operator--();
	iterator operator--(int);

	friend inline bool operator==(const iterator& x, const iterator& y) {
	  return x.ptr_ == y.ptr_;
	}

	friend inline bool operator!=(const iterator& x, const iterator& y) {
	  return !(x.ptr_ == y.ptr_);
	}

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
	T key;
    rb_tree_color color;
    node_ptr parent;
    node_ptr left;
    node_ptr right;

	rb_tree_node(const T& val) : key(val) { }
  };

  static const node_ptr nil = 0;
  static const rb_tree_color color_red = false;
  static const rb_tree_color color_black = true;
  node_ptr root_;
  size_type size_;

  static node_ptr MinimumNode(node_ptr x);
  static node_ptr MaximumNode(node_ptr x);

  static node_ptr SuccessorNode(node_ptr x);
  static node_ptr PredecessorNode(node_ptr x);

  std::pair <node_ptr, bool> InsertValue(node_ptr y, node_ptr z);
  node_ptr DeleteValue(node_ptr x);

 private:
  std::pair <node_ptr, bool> InsertNode(node_ptr y, node_ptr z);
  node_ptr DeleteNode(node_ptr x);

  void Transplant(node_ptr u, node_ptr v);
  void LeftRotate(node_ptr x);
  void RightRotate(node_ptr x);
  void InsertFixup(node_ptr x);
  void DeleteFixup(node_ptr x);

};

} // namespace rbtree

#endif // RB_TREE_H
