#ifndef RB_TREE_H
#define RB_TREE_H

#include <utility>

namespace rb_tree {

template <class T>
class rb_tree {

 public:
  typedef unsigned int size_type;
  class iterator;
  typedef iterator const_iterator;
 protected:
  struct rb_tree_node;
  typedef rb_tree_node* node_ptr;

  typedef bool rb_tree_color;
  static const rb_tree_color color_red = false;
  static const rb_tree_color color_black = true;

 public:
  std::pair <iterator, bool> insert(const T& val);
  iterator insert(const_iterator pos, const T& val);

  iterator erase(const_iterator pos);
  size_type erase(const T& val);
  iterator erase(const_iterator first, const_iterator last);

  const_iterator find (const T& val) const;

  const_iterator begin() const;
  const_iterator end() const;
  const_iterator cbegin() const;
  const_iterator cend() const;

  const_iterator rbegin() const;
  const_iterator rend() const;
  const_iterator crbegin() const;
  const_iterator crend() const;
  
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

   private:
	node_ptr ptr_;
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
  node_ptr root_;
  size_type size_;

  static node_ptr MinimumNode(node_ptr x);
  static node_ptr MaximumNode(node_ptr x);

  static node_ptr SuccessorNode(node_ptr x);
  static node_ptr PredecessorNode(node_ptr x);

 private:
  void Transplant(node_ptr u, node_ptr v);
  void InsertNode(node_ptr z);
  void DeleteNode(node_ptr z);

};

} // namespace rbtree

#endif // RB_TREE_H
