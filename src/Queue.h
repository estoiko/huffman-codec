#ifndef QUEUE_H
#define QUEUE_H

#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>
#include <concepts>
#include "Exceptions.h"

template <typename T>
class Queue {
private:
    struct Node {
        T data_;
        Node* next_;
    };

    Node* head_;
    Node* tail_;
    std::size_t current_size_;

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

    template<typename Comp>
    Node* merge(Node* left, Node* right, Comp comp) {
        if (left == nullptr) return right;
        if (right == nullptr) return left;

        if (!comp(right->data_, left->data_)) {
            left->next_ = merge(left->next_, right, comp);
            return left;
        }

        right->next_ = merge(left, right->next_, comp);
        return right;
    }

    template<typename Comp>
    Node* mergeSort(Node* node, Comp comp) {
        if (node == nullptr || node->next_ == nullptr) {
            return node;
        }

        Node* second = split(node);

        node = mergeSort(node, comp);
        second = mergeSort(second, comp);

        return merge(node, second, comp);
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

public:
    Queue()
        : head_(nullptr)
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

    template<typename Comp>
    void sort(Comp comp) {
        head_ = mergeSort(head_, comp);
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
};

template <typename T, typename Comp>
T popMin(Queue<T>& q1, Queue<T>& q2, Comp comp) {
    if (q1.isEmpty()) return q2.pop();
    if (q2.isEmpty()) return q1.pop();

    if (!comp(q2.front(), q1.front())) {
        return q1.pop();
    }

    return q2.pop();
}

#endif // QUEUE_H
