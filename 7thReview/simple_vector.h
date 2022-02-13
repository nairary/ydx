#pragma once

#include <cassert>
#include "array_ptr.h"
#include <initializer_list>
#include <stdexcept>

struct ReserveProxyObj {
    explicit ReserveProxyObj(std::size_t size) : capacity_(size) { }
    std::size_t capacity_;
};

template<typename Type>
class SimpleVector {
public:
    using Iterator = Type *;
    using ConstIterator = const Type *;

    SimpleVector() noexcept = default;

    SimpleVector(ReserveProxyObj obj_) {
        Reserve(obj_.capacity_);
    }

    explicit SimpleVector(size_t size)
            : size_(size), capacity_(size), value_(size ? new Type[size] : nullptr) {
        std::fill(begin(), begin() + size, Type());
    }

    SimpleVector(size_t size, const Type &value)
            : size_(size), capacity_(size), value_(size ? new Type[size] : nullptr) {
        std::fill(begin(), end(), value);
    }

    SimpleVector(std::initializer_list<Type> init)
            : size_(init.size()), capacity_(init.size()), value_(init.size() ? new Type[init.size()] : nullptr) {
        std::move(init.begin(), init.end(), value_.Get());
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        if (size_ == 0) {
            return true;
        } else return false;
    }

    Type &operator[](size_t index) noexcept {
        return value_[index];
    }

    const Type &operator[](size_t index) const noexcept {
        return value_[index];
    }

    Type &At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("index>=size");
        } else {
            return value_[index];
        }
    }

    const Type &At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("index>=size");
        } else {
            return value_[index];
        }
    }

    void Clear() noexcept {
        size_ = 0;
    }

    void Resize(size_t new_size) {
        if (new_size > capacity_) {
            size_t new_capacity_ = std::max(new_size, capacity_ * 2);
            ArrayPtr<Type> new_values_(new_capacity_);

            std::fill(new_values_.Get(), new_values_.Get() + new_capacity_, Type());
            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), new_values_.Get());

            new_values_.swap(value_);
            size_ = new_size;
            capacity_ = new_capacity_;

        } else if (new_size > size_) {
            for (size_t i = size_; i < new_size; i++) {
                value_[i] = Type();
            }
        } else {
            size_ = new_size;
        }

    }

    Iterator begin() noexcept {
        return value_.Get();
    }

    Iterator end() noexcept {
        return value_.Get() + size_;
    }

    ConstIterator begin() const noexcept {
        return value_.Get();
    }

    ConstIterator end() const noexcept {
        return value_.Get() + size_;
    }

    ConstIterator cbegin() const noexcept {
        return value_.Get();
    }

    ConstIterator cend() const noexcept {
        return value_.Get() + size_;
    }

    SimpleVector(const SimpleVector &other)
            : size_(other.GetSize()), capacity_(other.GetCapacity()), value_(other.GetSize()) {
        std::copy(other.begin(), other.end(), begin());
    }

    SimpleVector(SimpleVector&& other) {
        swap(other);
    }

    SimpleVector &operator=(const SimpleVector &rhs) {
        if (rhs != *this) {
            SimpleVector copy(rhs);
            swap(copy);
        }
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& other) {
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
        (std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()), value_.Get());
        return *this;
    }

    void PushBack(const Type &item) {
        Insert(end(), item);
    }

    void PushBack(Type&& value) {
        Insert(end(), std::move(value));
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        return InsertInVector(pos, value);
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        int dist = pos - begin();
        if (size_ == capacity_) {
            size_t new_capacity = std::max(1, (int)capacity_ * 2);
            ArrayPtr<Type> tmp(new_capacity);
            std::move(begin(), end(), tmp.Get());
            value_.swap(tmp);
            capacity_ = new_capacity;
        }
        std::move_backward(begin() + dist, end(), end() + 1);
        *(begin() + dist) = std::move(value);
        ++size_;
        return begin() + dist;
    }

    void PopBack() noexcept {
        if (!IsEmpty()) {
            --size_;
        }
    }

    Iterator Erase(ConstIterator pos) {
        int dist = pos - begin();
        std::move(begin() + dist + 1, end(), begin() + dist);
        --size_;
        return begin() + dist;
    }

    void Reserve(size_t new_capacity){
        if (new_capacity > capacity_) {
            ArrayPtr<Type> new_values_(new_capacity);
            std::move(begin(), end(), new_values_.Get());
            value_.swap(new_values_);
            capacity_ = new_capacity;
        }
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector &other) noexcept {
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        value_.swap(other.value_);
    }

private:
    std::size_t size_ = 0;
    std::size_t capacity_ = 0;
    ArrayPtr<Type> value_;

};

template<typename Type>
inline bool operator==(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
    return (lhs.GetSize() == rhs.GetSize() && std::equal(lhs.begin(), lhs.end(), rhs.begin()));
}

template<typename Type>
inline bool operator!=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
    return !(lhs == rhs);
}

template<typename Type>
inline bool operator<(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template<typename Type>
inline bool operator<=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
    return !(lhs > rhs);
}

template<typename Type>
inline bool operator>(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
    return std::lexicographical_compare(rhs.begin(), rhs.end(), lhs.begin(), lhs.end());
}

template<typename Type>
inline bool operator>=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
    return !(lhs < rhs);
}

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}