#ifndef SMALLKV_AVL_TREE_H
#define SMALLKV_AVL_TREE_H

#include <iostream> // For DebugPrint
#include <stack>    // For iterator
#include <memory>   // For memory usage calculation (approx)
#include <functional> // For std::function (optional for destructor)
#include <cstdint>

#include "memtable_iterator.h" // Include the base iterator

namespace smallkv {

template <typename Key, typename Value>
struct AVLNode {
    Key key;
    Value value;
    AVLNode *left;
    AVLNode *right;
    int height;
    // Consider adding memory usage per node if needed for accurate tracking

    AVLNode(const Key& k, const Value& v) :
        key(k), value(v), left(nullptr), right(nullptr), height(1) {}

    // Add destructor if Value needs specific cleanup not handled by RAII
    // ~AVLNode() { ... }
};


template <typename Key, typename Value>
class AVLTree {
private:
    AVLNode<Key, Value>* root_;
    int size_;
    int64_t mem_usage_; // Approximate memory usage

    // --- Private Helper Functions for AVL Operations ---
    int getHeight(AVLNode<Key, Value>* node);
    int getBalance(AVLNode<Key, Value>* node);
    AVLNode<Key, Value>* rightRotate(AVLNode<Key, Value>* y);
    AVLNode<Key, Value>* leftRotate(AVLNode<Key, Value>* x);
    AVLNode<Key, Value>* insertNode(AVLNode<Key, Value>* node, const Key& key, const Value& value);
    AVLNode<Key, Value>* findMinNode(AVLNode<Key, Value>* node);
    AVLNode<Key, Value>* deleteNode(AVLNode<Key, Value>* node, const Key& key);
    void destroyTree(AVLNode<Key, Value>* node); // Helper for destructor
    AVLNode<Key, Value>* searchNode(AVLNode<Key, Value>* node, const Key& key) const;

    // Optional: Custom cleanup function for values if needed
    // std::function<void(const Key&, Value&)> value_destructor_;

public:
    explicit AVLTree();
    ~AVLTree();

    // Insert a key-value pair. Returns true if inserted, false if key exists.
    bool Insert(const Key& key, const Value& value);

    // Search for a key. Returns true if found, sets value_ptr if provided.
    bool Search(const Key& key, Value* value_ptr = nullptr) const;

    // Delete a key. Returns true if deleted, false if not found.
    bool Delete(const Key& key);

    // Debugging function to print the tree (e.g., in-order)
    void DebugPrint() const; // Implement based on needs

    inline int GetSize() const { return size_; }
    inline int64_t GetMemUsage() const { return mem_usage_; }

    // --- AVL Tree Iterator ---
    class AVLIterator : public MemTableIterator<Key, Value> {
    public:
        explicit AVLIterator(const AVLTree* tree);
        ~AVLIterator() override = default;

        bool Valid() const override;
        const Key& key() const override;
        const Value& value() const override;
        void Next() override;
        void SeekToFirst() override;
        void Seek(const Key& target) override;

    private:
        void findFirst(); // Helper for SeekToFirst
        void findNext();  // Helper for Next
        void pushLeftPath(AVLNode<Key, Value>* node); // Helper to push left children onto stack

        const AVLTree* tree_;
        AVLNode<Key, Value>* current_node_;
        std::stack<AVLNode<Key, Value>*> path_stack_; // Stack for in-order traversal
    };

    // Factory function to create an iterator
    std::unique_ptr<MemTableIterator<Key, Value>> NewIterator() const {
         return std::make_unique<AVLIterator>(this);
    }

    // No copying/assignment
    AVLTree(const AVLTree&) = delete;
    AVLTree& operator=(const AVLTree&) = delete;
};

// --- Implementation details for AVLTree and AVLIterator ---
// (Usually placed in a .cpp file or below the class definition in the .h)

// --- AVLTree Method Implementations (Placeholders) ---
template <typename Key, typename Value>
AVLTree<Key, Value>::AVLTree() : root_(nullptr), size_(0), mem_usage_(sizeof(AVLTree)) {}

template <typename Key, typename Value>
AVLTree<Key, Value>::~AVLTree() {
    destroyTree(root_);
}

template <typename Key, typename Value>
void AVLTree<Key, Value>::destroyTree(AVLNode<Key, Value>* node) {
    if (node) {
        destroyTree(node->left);
        destroyTree(node->right);
        // Call value_destructor_ if defined and needed
        delete node;
    }
}

template <typename Key, typename Value>
int AVLTree<Key, Value>::getHeight(AVLNode<Key, Value>* node) {
    return node ? node->height : 0;
}

template <typename Key, typename Value>
int AVLTree<Key, Value>::getBalance(AVLNode<Key, Value>* node) {
     return node ? getHeight(node->left) - getHeight(node->right) : 0;
}

// ... Implement rightRotate, leftRotate, insertNode, deleteNode, searchNode ...
// ... These are standard AVL tree operations, quite involved ...

template <typename Key, typename Value>
bool AVLTree<Key, Value>::Insert(const Key& key, const Value& value) {
    int initial_size = size_;
    root_ = insertNode(root_, key, value);
    // insertNode should update size_ and mem_usage_ on successful insertion
    return size_ > initial_size;
}

template <typename Key, typename Value>
bool AVLTree<Key, Value>::Search(const Key& key, Value* value_ptr) const {
    AVLNode<Key, Value>* node = searchNode(root_, key);
    if (node && value_ptr) {
        *value_ptr = node->value;
    }
    return node != nullptr;
}

template <typename Key, typename Value>
AVLNode<Key, Value>* AVLTree<Key, Value>::searchNode(AVLNode<Key, Value>* node, const Key& key) const {
     if (!node) return nullptr;
     if (key < node->key) return searchNode(node->left, key);
     else if (key > node->key) return searchNode(node->right, key);
     else return node; // Found
}


template <typename Key, typename Value>
bool AVLTree<Key, Value>::Delete(const Key& key) {
    int initial_size = size_;
    root_ = deleteNode(root_, key);
     // deleteNode should update size_ and mem_usage_ on successful deletion
    return size_ < initial_size;
}

template <typename Key, typename Value>
void AVLTree<Key, Value>::DebugPrint() const {
    // Implement in-order traversal print or other debug format
    std::cout << "============= AVLTree DEBUG =============" << std::endl;
    // ... recursive print helper ...
    std::cout << "Size: " << size_ << ", MemUsage: " << mem_usage_ << std::endl;
    std::cout << "=======================================" << std::endl;
}


// --- AVLIterator Method Implementations (Placeholders) ---

template <typename Key, typename Value>
AVLTree<Key, Value>::AVLIterator::AVLIterator(const AVLTree* tree)
    : tree_(tree), current_node_(nullptr) {
    SeekToFirst(); // Initialize iterator to the first element
}

template <typename Key, typename Value>
bool AVLTree<Key, Value>::AVLIterator::Valid() const {
    return current_node_ != nullptr;
}

template <typename Key, typename Value>
const Key& AVLTree<Key, Value>::AVLIterator::key() const {
    // REQUIRES: Valid()
    // Add assertion: assert(Valid());
    return current_node_->key;
}

template <typename Key, typename Value>
const Value& AVLTree<Key, Value>::AVLIterator::value() const {
    // REQUIRES: Valid()
    // Add assertion: assert(Valid());
    return current_node_->value;
}

template <typename Key, typename Value>
void AVLTree<Key, Value>::AVLIterator::pushLeftPath(AVLNode<Key, Value>* node) {
     while(node) {
         path_stack_.push(node);
         node = node->left;
     }
}

template <typename Key, typename Value>
void AVLTree<Key, Value>::AVLIterator::findFirst() {
     while(!path_stack_.empty()) path_stack_.pop(); // Clear stack
     current_node_ = nullptr;
     pushLeftPath(tree_->root_);
     if (!path_stack_.empty()) {
         current_node_ = path_stack_.top();
     }
}

template <typename Key, typename Value>
void AVLTree<Key, Value>::AVLIterator::SeekToFirst() {
    findFirst();
}

template <typename Key, typename Value>
void AVLTree<Key, Value>::AVLIterator::findNext() {
    // REQUIRES: Valid() or stack not empty after Seek
    if (path_stack_.empty()) {
        current_node_ = nullptr; // Should not happen if called after Valid() check
        return;
    }

    AVLNode<Key, Value>* node = path_stack_.top();
    path_stack_.pop();

    // If the popped node has a right child, find the leftmost node in its right subtree
    if (node->right) {
        pushLeftPath(node->right);
    }

    // The next node is now at the top of the stack (if any)
    if (!path_stack_.empty()) {
        current_node_ = path_stack_.top();
    } else {
        current_node_ = nullptr; // Reached the end
    }
}


template <typename Key, typename Value>
void AVLTree<Key, Value>::AVLIterator::Next() {
    // REQUIRES: Valid()
    // Add assertion: assert(Valid());
    findNext();
}


template <typename Key, typename Value>
void AVLTree<Key, Value>::AVLIterator::Seek(const Key& target) {
    while(!path_stack_.empty()) path_stack_.pop(); // Clear stack
    current_node_ = nullptr;

    AVLNode<Key, Value>* node = tree_->root_;
    while(node) {
        if (target <= node->key) {
            // Potential candidate, push and go left
            path_stack_.push(node);
            node = node->left;
        } else {
            // Target is greater, go right (don't push this node)
            node = node->right;
        }
    }

    // The top of the stack is the first node >= target, or stack is empty
    if (!path_stack_.empty()) {
        current_node_ = path_stack_.top();
    } else {
        current_node_ = nullptr;
    }
}


} // namespace smallkv

#endif // SMALLKV_AVL_TREE_H