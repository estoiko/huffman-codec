#ifndef QUEUE_H
#define QUEUE_H

#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>
#include <concepts>
#include "Exceptions.h"

template <typename T, typename Compare = std::less<T>>
requires std::totally_ordered<T> ||
requires(T a, T b, Compare c) { { c(a, b) } -> std::convertible_to<bool>; }
class Queue {
private:
    Compare comp;

    struct Node {
        T data_;
        Node* next_;
    };

    Node* head_;
    Node* tail_;
    std::size_t current_size_;

public:
    Queue()
        : comp(Compare())
        , head_(nullptr)
        , tail_(nullptr)
        , current_size_(0)
    {}

    ~Queue() {
        while (head_ != nullptr) {
            Node* next = head_->next_;
            delete head_;
            head_ = next;
        }
        tail_ = nullptr;
        current_size_ = 0;
    }

    Queue(const Queue<T>& other) = delete;
    Queue(Queue<T>&& other) noexcept = delete;
    Queue<T>& operator=(const Queue<T>& other) = delete;
    Queue<T>& operator=(Queue<T>&& other) noexcept = delete;

    void push(const T& e) {
        Node* new_node = new Node {e, nullptr};

        if (isEmpty()) {
            new_node->next_ = head_;
            head_ = new_node;
            if (tail_ == nullptr) tail_ = head_;
        } else {
            tail_->next_ = new_node;
            tail_ = new_node;
        }

        ++current_size_;
    }

    static Node* split(Node* source) {
        if (source == nullptr || source->next_ == nullptr) {
            return nullptr;
        }

        Node* slow = source;
        Node* fast = source->next_;

        while (fast != nullptr && fast->next_ != nullptr) {
            slow = slow->next_;
            fast = fast->next_->next_;
        }

        Node* second = slow->next_;
        slow->next_ = nullptr;

        return second;
    }

    Node* merge(Node* left, Node* right) {
        if (left == nullptr) return right;
        if (right == nullptr) return left;

        if (!comp(right->data_, left->data_)) {
            left->next_ = merge(left->next_, right);
            return left;
        }

        right->next_ = merge(left, right->next_);
        return right;
    }

    Node* mergeSort(Node* node) {
        if (node == nullptr || node->next_ == nullptr) {
            return node;
        }

        Node* second = split(node);

        node = mergeSort(node);
        second = mergeSort(second);

        return merge(node, second);
    }

    void refreshTail() {
        tail_ = head_;

        if (tail_ == nullptr) {
            return;
        }

        while (tail_->next_ != nullptr) {
            tail_ = tail_->next_;
        }
    }

    void sort() {
        head_ = mergeSort(head_);
        refreshTail();
    }

    T pop() {
        if (isEmpty()) {
            throw QueueUnderflow();
        }

        T front = std::move_if_noexcept(head_->data_);

        Node* new_head = head_->next_;
        delete head_;
        head_ = new_head;

        --current_size_;

        if (isEmpty()) {
            tail_ = nullptr;
        }

        return front;
    }

    bool isEmpty() const {
        return current_size_ == 0;
    }

    const T& front() const {
        if (isEmpty()) {
            throw QueueUnderflow();
        }
        return head_->data_;
    }

    std::size_t size() const {
        return current_size_;
    }

    Compare get_comp() const {
        return comp;
    }
};

template <typename T, typename Compare>
T popMin(Queue<T, Compare>& q1, Queue<T, Compare>& q2) {
    if (q1.isEmpty()) return q2.pop();
    if (q2.isEmpty()) return q1.pop();

    Compare comp = q1.get_comp();

    if constexpr (std::is_pointer_v<T>) {
        if constexpr (std::is_same_v<Compare, std::less<T>>) {
            if (*q1.front() < *q2.front()) return q1.pop();
            return q2.pop();
        }
    }

    if (comp(q1.front(), q2.front())) {
        return q1.pop();
    }
    return q2.pop();
}

#endif // QUEUE_H
