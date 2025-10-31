#ifndef ITER_H
#define ITER_H
#include <util/types.h>


/// @brief finds the return type for an iterators arrow operator
/// @tparam Iter the iterator we want to deduce the arrow type of
template<typename Iter>
using iter_arrow = std::result_of_t<decltype(&Iter::operator->)(Iter*)>;


/// @brief finds the return type for an iterators deref operator
/// @tparam Iter the iterator we want to deduce the deref type of
template<typename Iter>
using iter_deref = std::result_of_t<decltype(&Iter::operator*)(Iter*)>;


/// @brief finds the return type for an iterators rev function
/// @tparam Iter the iterator we want to deduce the rev type of
template<typename Iter>
using iter_rev = std::result_of_t<decltype(&Iter::rev)(Iter*)>;



/// @brief A value and an index (or enumeration) from the position in the iterator
/// @tparam T the type of the value this stores
template<typename T>
struct Enumerated{
public:
    T val;
    usize index;
    Enumerated(T _val, const usize _index) : val(_val), index(_index){}
};


template<typename T>
struct EnumeratedIterRev;

/// @brief wraps an iterator to enumerate over it
/// @tparam T the type of iterator to wrap
template<typename T>
struct EnumeratedIter{
private:
    /// @brief the actual iterator
    T m_iter;
    /// @brief current enumeration
    usize m_index;

    EnumeratedIter(T _iter, usize _index) : m_iter(_iter), m_index(_index){}
public:
    explicit EnumeratedIter(T _iter) : m_iter(_iter), m_index(0){}

    /// @return A enumerated iterator that counts backwards from the max count, over a reverse iterator
    EnumeratedIterRev<iter_rev<T>> rev() {
        return {this->m_iter.rev()};
    }

    /// @return where this iterator will begin
    EnumeratedIter<T> begin() const {
        return this->m_iter.begin();
    }

    /// @return where this iterator will end
    EnumeratedIter<T> end() const {
        return this->m_iter.end();
    }

    /// @return the number of elements this iterator will iterate over
    [[nodiscard]]
    usize size() const {
        return this->m_iter.size();
    }

    /// @return the current element of this iterator
    Enumerated<iter_deref<T>> operator*() const {
        return Enumerated<iter_deref<T>>(*this->m_iter, this->m_index);
    }

    /// @return the current element of this iterator
    Enumerated<iter_arrow<T>> operator->() {
        return Enumerated<iter_arrow<T>>(*this->m_iter, this->m_index);
    }

    /// @brief  increment our iterator (move to the next value in our iterator)
    /// @return a reference to this
    EnumeratedIter<T>& operator++() {
        ++this->m_iter;
        ++this->m_index;
        return *this;
    }

    /// @brief  decrement our iterator (move to the previous value in our iterator)
    /// @return a reference to this
    EnumeratedIter<T>& operator--() {
        --this->m_iter;
        --this->m_index;
        return *this;
    }

    /// @brief increment out iterator (move to the next value in our iterator) but return an iterator representing our current state
    /// @return the state of the iterator before this operation
    EnumeratedIter<T> operator++(int) {
        return {this->m_iter++, this->m_index++};
    }

    /// @brief decrement out iterator (move to the previous value in our iterator) but return an iterator representing our current state
    /// @return the state of the iterator before this operation
    EnumeratedIter<T> operator--(int) {
        return {this->m_iter--, this->m_index--};
    }

    /// @return true if the two provided iterators are equal
    friend bool operator==(const EnumeratedIter<T>& a, const EnumeratedIter<T>& b) {
        return a.m_iter == b.m_iter;
    }

    /// @return true if the two provided iterators are not equal
    friend bool operator!=(const EnumeratedIter<T>& a, const EnumeratedIter<T>& b) {
        return a.m_iter != b.m_iter;
    }
};


/// @brief wraps an iterator to enumerate over it (starting from max iterations counting down)
/// @tparam T the type of iterator to wrap
template<typename T>
struct EnumeratedIterRev{
private:
    /// @brief the actual iterator
    T m_iter;
    /// @brief current enumeration
    usize m_index;

public:
    explicit EnumeratedIterRev(T _iter) : m_iter(_iter), m_index(_iter.size() - 1){}

    /// @return an enumerated iterator that counts from 0 to size over a reversed iterator
    EnumeratedIter<iter_rev<T>> rev() {
        return {this->m_iter.rev()};
    }

    /// @return where this iterator will begin
    EnumeratedIterRev<T> begin() const {
        return this->m_iter.begin();
    }

    /// @return where this iterator will end
    EnumeratedIterRev<T> end() const {
        return this->m_iter.end();
    }

    /// @return the number of elements this iterator will iterate over
    [[nodiscard]]
    usize size() const {
        return this->m_iter.size();
    }

    /// @return the current element of this iterator
    Enumerated<iter_deref<T>> operator*() const {
        return Enumerated<iter_deref<T>>(*this->m_iter, this->m_index);
    }

    /// @return the current element of this iterator
    Enumerated<iter_arrow<T>> operator->() {
        return Enumerated<iter_arrow<T>>(*this->m_iter, this->m_index);
    }

    /// @brief  increment our iterator (move to the next value in our iterator)
    /// @return a reference to this
    EnumeratedIterRev<T>& operator++() {
        ++this->m_iter;
        --this->m_index;
        return *this;
    }

    /// @brief  decrement our iterator (move to the previous value in our iterator)
    /// @return a reference to this
    EnumeratedIterRev<T>& operator--() {
        --this->m_iter;
        ++this->m_index;
        return *this;
    }

    /// @brief increment out iterator (move to the next value in our iterator) but return an iterator representing our current state
    /// @return the state of the iterator before this operation
    EnumeratedIterRev<T> operator++(int) {
        return {this->m_iter++, this->m_index--};
    }

    /// @brief decrement out iterator (move to the previous value in our iterator) but return an iterator representing our current state
    /// @return the state of the iterator before this operation
    EnumeratedIterRev<T> operator--(int) {
        return {this->m_iter--, this->m_index++};
    }

    /// @return true if the two provided iterators are equal
    friend bool operator==(const EnumeratedIterRev<T>& a, const EnumeratedIterRev<T>& b) {
        return a.m_iter == b.m_iter;
    }

    /// @return true if the two provided iterators are not equal
    friend bool operator!=(const EnumeratedIterRev<T>& a, const EnumeratedIterRev<T>& b) {
        return a.m_iter != b.m_iter;
    }
};

template<typename T>
struct SliceIterRev;

/// @brief An iterator over contiguous memory
/// @tparam T the type stored in the contiguous array
template<typename T>
struct SliceIter{
private:
    T* _start;
    T* _end;
public:
    SliceIter(const SliceIter<T>& _other) : _start(_other._start), _end(_other._end){}
    SliceIter(T* start, T* end) : _start(start), _end(end){}

    /// @return an iterator that starts from this iterators end and iterates backwards towards this iterators start
    SliceIterRev<T> rev() const{
        return SliceIterRev<T>{this->_end - 1, this->_start - 1};
    }

    /// @return an iterator which enumerates all values of this iterator starting at 0 and ending at size-1
    EnumeratedIter<SliceIter<T>> enumerate() const {
        return {{*this}};
    }

    /// @return an iterator which enumerates all values of this iterator starting at 0 and ending at size-1
    EnumeratedIter<SliceIter<T>> enumerate() {
        return {{*this}};
    }

    /// @return where this iterator will begin
    SliceIter<T> begin() const {
        return {this->_start, this->_end};
    }

    /// @return where this iterator will end
    SliceIter<T> end() const {
        return {this->_end, this->_end};
    }

    /// @return the number of elements this iterator will iterate over
    [[nodiscard]]
    usize size() const {
        return (static_cast<usize>(this->_end) - static_cast<usize>(this->_start)) / sizeof(T);
    }

    /// @return the current element of this iterator
    const T& operator*() const {
        return *this->_start;
    }

    /// @return the current element of this iterator
    T& operator*() {
        return *this->_start;
    }

    /// @return the current element of this iterator
    T& operator->() {
        return *this->_start;
    }

    /// @brief  increment our iterator (move to the next value in our iterator)
    /// @return a reference to this
    SliceIter<T>& operator++() {
        ++this->_start;
        return *this;
    }

    /// @brief  decrement our iterator (move to the previous value in our iterator)
    /// @return a reference to this
    SliceIter<T>& operator--() {
        --this->_start;
        return *this;
    }

    /// @brief increment out iterator (move to the next value in our iterator) but return an iterator representing our current state
    /// @return the state of the iterator before this operation
    SliceIter<T> operator++(int) {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    /// @brief decrement out iterator (move to the previous value in our iterator) but return an iterator representing our current state
    /// @return the state of the iterator before this operation
    SliceIter<T> operator--(int) {
        auto tmp = *this;
        --(*this);
        return tmp;
    }

    /// @return true if the two provided iterators are equal
    friend bool operator==(const SliceIter<T>& a, const SliceIter<T>& b) {
        return a._start == b._start;
    }

    /// @return true if the two provided iterators are not equal
    friend bool operator!=(const SliceIter<T>& a, const SliceIter<T>& b) {
        return a._start != b._start;
    }
};


/// @brief An iterator over contiguous memory that starts are the highest memory location and goes backwards
/// @tparam T the type stored in the contiguous array
template<typename T>
struct SliceIterRev{
private:
    T* _start;
    T* _end;
public:
    SliceIterRev(T* start, T* end) : _start(start), _end(end){}

    /// @return an iterator that starts from this iterators end and iterates backwards towards this iterators start
    SliceIter<T> rev() const{
        return SliceIter<T>{this->_end + 1, this->_start + 1};
    }

    /// @return an iterator which enumerates all values of this iterator starting at 0 and ending at size-1
    EnumeratedIter<SliceIterRev<T>> enumerate() const {
        return {{this->_start, this->_end}};
    }

    /// @return an iterator which enumerates all values of this iterator starting at 0 and ending at size-1
    EnumeratedIter<SliceIterRev<T>> enumerate() {
        return {{this->_start, this->_end}};
    }

    /// @return where this iterator will begin
    SliceIterRev<T> begin() const {
        return {this->_start, this->_end};
    }

    /// @return where this iterator will end
    SliceIterRev<T> end() const {
        return {this->_end, this->_end};
    }

    /// @return the number of elements this iterator will iterate over
    [[nodiscard]]
    usize size() const {
        return (static_cast<usize>(this->_start) - static_cast<usize>(this->_end)) / sizeof(T);
    }

    /// @return the current element of this iterator
    const T& operator*() const{
        return *this->_start;
    }

    /// @return the current element of this iterator
    T& operator*() {
        return *this->_start;
    }

    /// @return the current element of this iterator
    T& operator->(){
        return this->_start;
    }

    /// @brief  increment our iterator (move to the next value in our iterator)
    /// @return a reference to this
    SliceIterRev<T>& operator++() {
        --this->_start;
        return *this;
    }

    /// @brief  decrement our iterator (move to the previous value in our iterator)
    /// @return a reference to this
    SliceIterRev<T>& operator--() {
        ++this->_start;
        return *this;
    }

    /// @brief increment out iterator (move to the next value in our iterator) but return an iterator representing our current state
    /// @return the state of the iterator before this operation
    SliceIterRev<T> operator++(int) {
        auto tmp = *this;
        --(*this);
        return tmp;
    }

    /// @brief decrement out iterator (move to the previous value in our iterator) but return an iterator representing our current state
    /// @return the state of the iterator before this operation
    SliceIterRev<T> operator--(int) {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    /// @return true if the two provided iterators are equal
    friend bool operator==(const SliceIterRev<T>& a, const SliceIterRev<T>& b) {
        return a._start == b._start;
    }

    /// @return true if the two provided iterators are not equal
    friend bool operator!=(const SliceIterRev<T>& a, const SliceIterRev<T>& b) {
        return a._start != b._start;
    }
};


#endif //ITER_H
