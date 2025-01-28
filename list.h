#pragma once

#include <cstddef>
#include <iterator>
#include <new>
#include <stdexcept>

template <typename T>
class List;

template <typename T>
class ListNode {
   public:
    ListNode() = default;
    virtual ~ListNode() = default;

    ListNode(const ListNode& other) = delete;
    ListNode(ListNode&& other) = delete;
    ListNode& operator=(const ListNode& other) = delete;
    ListNode& operator=(ListNode&& other) = delete;

    [[nodiscard]] ListNode* GetNext() const {
        return next_;
    }

    [[nodiscard]] ListNode* GetPrev() const {
        return prev_;
    }

    void SetNext(ListNode* next) {
        next_ = next;
    }

    void SetPrev(ListNode* prev) {
        prev_ = prev;
    }

   private:
    ListNode* prev_ = nullptr;
    ListNode* next_ = nullptr;
};

template <typename T>
class TNode : public ListNode<T> {
   public:
    explicit TNode(const T& other) : value_(other) {};
    explicit TNode(T&& other) noexcept : value_(std::move(other)) {};

    [[nodiscard]] T& GetValue() {
        return value_;
    }

    [[nodiscard]] const T& GetValue() const {
        return value_;
    }

   private:
    T value_;
};

template <typename T>
class List {
   public:
    class Iterator {
       public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        Iterator() = default;

        explicit Iterator(ListNode<T>* node) : iter_(node) {
        }

        Iterator& operator++() {
            if (iter_ == nullptr) {
                throw std::out_of_range("Iterator is out of range");
            }
            iter_ = iter_->GetNext();
            return *this;
        }

        Iterator operator++(int) {
            if (iter_ == nullptr) {
                throw std::out_of_range("Iterator is out of range");
            }
            auto old = *this;
            iter_ = iter_->GetNext();
            return old;
        }

        Iterator& operator--() {
            if (iter_ == nullptr || iter_->GetPrev() == nullptr) {
                throw std::out_of_range("Decrementing past begin of list");
            }
            iter_ = iter_->GetPrev();
            return *this;
        }

        Iterator operator--(int) {
            if (iter_ == nullptr) {
                throw std::out_of_range("Iterator is out of range");
            }
            auto old = *this;
            iter_ = iter_->GetPrev();
            return old;
        }

        reference operator*() const {
            if (iter_ == nullptr) {
                throw std::runtime_error("Dereferencing a null iterator");
            }
            return static_cast<TNode<T>*>(iter_)->GetValue();
        }

        pointer operator->() const {
            if (iter_ == nullptr) {
                throw std::runtime_error("Dereferencing a null iterator");
            }
            return &(static_cast<TNode<T>*>(iter_)->GetValue());
        }

        bool operator==(const Iterator& other) const {
            return iter_ == other.iter_;
        }

        bool operator!=(const Iterator& other) const {
            return iter_ != other.iter_;
        }

       private:
        ListNode<T>* iter_ = nullptr;
        friend class List;
    };

    List() : end_(new ListNode<T>()) {
        end_->SetNext(end_);
        end_->SetPrev(end_);
    }

    List(const List& other) : end_(new ListNode<T>()) {
        end_->SetNext(end_);
        end_->SetPrev(end_);
        auto iter = other.end_->GetNext();
        while (iter != other.end_) {
            this->PushBack(static_cast<TNode<T>*>(iter)->GetValue());
            iter = iter->GetNext();
        }
    }

    List(List&& other) noexcept : end_(other.end_), size_(other.size_) {
        try {
            other.end_ = new ListNode<T>();  // NOLINT(cppcoreguidelines-owning-memory)
            other.end_->SetNext(other.end_);
            other.end_->SetPrev(other.end_);
            other.size_ = 0;
        } catch (...) {
            other.end_ = nullptr;
            other.size_ = 0;
        }
    }

    ~List() {
        while (!IsEmpty()) {
            PopFront();
        }
        delete end_;  // NOLINT(cppcoreguidelines-owning-memory)
        end_ = nullptr;
    }

    List& operator=(const List& other) {
        if (this != &other) {
            while (!IsEmpty()) {
                PopBack();
            }
            for (auto it = other.Begin(); it != other.End(); ++it) {
                PushBack(*it);
            }
        }
        return *this;
    }

    List& operator=(List&& other) noexcept {
        if (this != &other) {
            while (!IsEmpty()) {
                PopBack();
            }
            delete end_;

            end_ = other.end_;
            size_ = other.size_;
            // NOLINTBEGIN(cppcoreguidelines-owning-memory)
            other.end_ = new (std::nothrow) ListNode<T>();
            // NOLINTEND(cppcoreguidelines-owning-memory)
            if (other.end_ != nullptr) {
                other.end_->SetNext(other.end_);
                other.end_->SetPrev(other.end_);
                other.size_ = 0;
            }
        }
        return *this;
    }

    [[nodiscard]] bool IsEmpty() const {
        return size_ == 0;
    }

    [[nodiscard]] size_t Size() const {
        return size_;
    }

    // NOLINTBEGIN(cppcoreguidelines-owning-memory)
    void PushBack(const T& element) {
        TNode<T>* new_node = nullptr;
        try {
            new_node = new TNode<T>(element);
            LinkAfter(end_->GetPrev(), new_node);
            ++size_;
        } catch (...) {
            delete new_node;
            throw;
        }
    }

    void PushBack(T&& element) {
        TNode<T>* new_node = nullptr;
        try {
            new_node = new TNode<T>(std::move(element));
            LinkAfter(end_->GetPrev(), new_node);
            ++size_;
        } catch (...) {
            delete new_node;
            throw;
        }
    }

    void PushFront(const T& element) {
        TNode<T>* new_node = nullptr;
        try {
            new_node = new TNode<T>(element);
            LinkAfter(end_, new_node);
            ++size_;
        } catch (...) {
            delete new_node;
            throw;
        }
    }

    void PushFront(T&& element) {
        TNode<T>* new_node = nullptr;
        try {
            new_node = new TNode<T>(std::move(element));
            LinkAfter(end_, new_node);
            ++size_;
        } catch (...) {
            delete new_node;
            throw;
        }
    }

    // NOLINTEND(cppcoreguidelines-owning-memory)

    T& Front() {
        return static_cast<TNode<T>*>(end_->GetNext())->GetValue();
    }

    [[nodiscard]] const T& Front() const {
        return static_cast<TNode<T>*>(end_->GetNext())->GetValue();
    }

    T& Back() {
        return static_cast<TNode<T>*>(end_->GetPrev())->GetValue();
    }

    [[nodiscard]] const T& Back() const {
        return static_cast<TNode<T>*>(end_->GetPrev())->GetValue();
    }

    void PopBack() {
        UnLink(end_->GetPrev());
        --size_;
    }

    void PopFront() {
        UnLink(end_->GetNext());
        --size_;
    }

    void Erase(Iterator place) {
        auto to_erase = place.iter_;
        if (to_erase != end_) {
            UnLink(to_erase);
            --size_;
        }
    }

    [[nodiscard]] Iterator Begin() const {
        return Iterator(end_->GetNext());
    }

    [[nodiscard]] Iterator End() const {
        return Iterator(end_);
    }

   private:
    ListNode<T>* end_ = nullptr;
    size_t size_ = 0;

    void LinkAfter(ListNode<T>* after, ListNode<T>* emplace) {
        emplace->SetPrev(after);
        emplace->SetNext(after->GetNext());
        after->GetNext()->SetPrev(emplace);
        after->SetNext(emplace);
    }

    void UnLink(ListNode<T>* node) {
        if (node != nullptr) {
            node->GetPrev()->SetNext(node->GetNext());
            node->GetNext()->SetPrev(node->GetPrev());
            delete node;  // NOLINT(cppcoreguidelines-owning-memory)
        }
    }

    friend class Iterator;
};

template <typename T>
typename List<T>::Iterator begin(List<T>& list) {
    return list.Begin();
}

template <typename T>
typename List<T>::Iterator end(List<T>& list) {
    return list.End();
}
