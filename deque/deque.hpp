#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP

#include "exceptions.hpp"

#include <cstddef>

namespace sjtu
{

template <class T>
class deque
{
    const int BuckSize = 300;

public:
    class iterator;
    class const_iterator;
    T value;
    iterator start, finish;
    T **map;
    long long mapsize;
    class iterator
    {
    private:
        /**
         * TODO add data members
         *   just add whatever you want.
         */
        deque<T> *container;
        long long mapIndex;
        T *cur;

    public:
        /**
         * return a new iterator which pointer n-next elements
         *   even if there are not enough elements, the behaviour is **undefined**.
         * as well as operator-
         */
        iterator(const iterator &it)
            : mapIndex(it.mapIndex), cur(it.cur), container(it.container) {}
        iterator operator+(const int &n) const
        {
            //TODO
            iterator tmp(*this);
            auto leftSize = map[mapIndex] + BuckSize - 1 - cur;
            if (n <= leftSize)
            { //前进n步后依然在当前桶内
                tmp.cur += n;
            }
            else
            {
                n -= leftSize;
                size_t newBuckIndex = n % BuckSize - 1;
                size_t newMapIndex = mapIndex + n / BuckSize + 1;
                tmp.mapIndex = newMapIndex;
                tmp.cur = map[mapIndex] + newBuckIndex;
            }
            return tmp;
        }
        iterator operator-(const int &n) const
        {
            //TODO
            iterator tmp(*this);
            auto leftSize = cur - map[mapIndex];
            if (leftSize >= n)
            { //后退n步后依然在当前桶内
                tmp.cur -= n;
            }
            else
            {
                n -= leftSize;
                size_t newBuckIndex = n % BuckSize - 1;
                size_t newMapIndex = mapIndex + n / BuckSize + 1;
                tmp.mapIndex = newMapIndex;
                tmp.cur = map[mapIndex] + newBuckIndex;
            }
            return tmp;
        }
        // return th distance between two iterator,
        // if these two iterators points to different vectors, throw invaild_iterator.
        int operator-(const iterator &rhs) const
        {
            //TODO
            if (container != rhs.container)
            {
                throw invaild_iterator();
            }

            int tmp =
                (mapIndex - rhs.mapIndex) * BuckSize + (cur - map[mapIndex]) - (rhs.map[mapIndex] - rhs.cur);
            return tmp;
        }
        iterator &operator+=(const int &n)
        {
            //TODO
            auto leftSize = map[mapIndex] + BuckSize - 1 - cur;
            if (n <= leftSize)
            { //前进n步后依然在当前桶内
                cur += n;
            }
            else
            {
                n -= leftSize;
                size_t newBuckIndex = n % BuckSize - 1;
                size_t newMapIndex = mapIndex + n / BuckSize + 1;
                mapIndex = newMapIndex;
                cur = map[mapIndex] + newBuckIndex;
            }
            return *this;
        }
        iterator &operator-=(const int &n)
        {
            //TODO
            auto leftSize = cur - map[mapIndex];
            if (leftSize >= n)
            { //后退n步后依然在当前桶内
                cur -= n;
            }
            else
            {
                n -= leftSize;
                size_t newBuckIndex = n % BuckSize - 1;
                size_t newMapIndex = mapIndex + n / BuckSize + 1;
                mapIndex = newMapIndex;
                cur = map[mapIndex] + newBuckIndex;
            }
            return *this;
        }
        /**
         * TODO iter++
         */
        iterator operator++(int)
        {
            auto tmp = *this;
            ++*this;
            return tmp;
        }
        /**
         * TODO ++iter
         */
        iterator &operator++()
        {
            if (cur != map[mapIndex] + BuckSize - 1)
            { //+1之后依然在桶内
                ++cur;
            }
            else if (mapIndex + 1 < mapsize)
            { //已经在桶的结尾，但是之后还有新的map指针
                ++mapIndex;
                cur = map[mapIndex]; //指向下一个桶的开头
            }
            else
            { //mapIndex +1之后没有了map
                mapIndex = container->mapsize;

                cur = map[mapIndex]; //指向最后一个桶的最后一个元素的后方区域
            }
            return *this;
        }
        /**
         * TODO iter--
         */
        iterator operator--(int)
        {
            auto tmp = *this;
            --*this;
            return tmp;
        }
        /**
         * TODO --iter
         */
        iterator &operator--()
        {
            if (cur != map[mapIndex])
            {
                --cur;
            }
            else if (mapIndex - 1 >= 0)
            {
                --mapIndex;
                cur = map[mapIndex] + BuckSize - 1;
            }
            else
            {
                mapIndex = 0;
                cur = map[mapIndex];
            }
            return *this;
        }
        /**
         * TODO *it
         */
        T &operator*() const { return *cur; }
        /**
         * TODO it->field
         */
        T *operator->() const noexcept { return &(operator*()); }
        /**
         * a operator to check whether two iterators are same (pointing to the same memory).
         */
        bool operator==(const iterator &rhs) const { return ((mapIndex == rhs.mapIndex) && (cur == rhs.cur) && (container == rhs.container)); }
        bool operator==(const const_iterator &rhs) const { return ((mapIndex == rhs.mapIndex) && (cur == rhs.cur) && (container == rhs.container)); }
        /**
         * some other operator for iterator.
         */
        bool operator!=(const iterator &rhs) const { return !(rhs == *this); }
        bool operator!=(const const_iterator &rhs) const { return !(rhs == *this); }
    };
    class const_iterator
    {
        // it should has similar member method as iterator.
        //  and it should be able to construct from an iterator.
    private:
        // data members.
        const deque<T> *container;
        long long mapIndex;
        const T *cur;

    public:
        const_iterator() mapIndex(-1), cur(nullptr), container(nullptr)
        {
            // TODO
        }
        const_iterator(const const_iterator &other) : mapIndex(other.mapIndex), cur(other.cur), container(other.container)
        {
            // TODO
        }
        const_iterator(const iterator &other) : mapIndex(other.mapIndex), cur(other.cur), container(other.container)
        {
            // TODO
        }
        // And other methods in iterator.
        // And other methods in iterator.
        // And other methods in iterator.
        const_iterator operator+(const int &n) const
        {
            //TODO
            const_iterator tmp(*this);
            auto leftSize = map[mapIndex] + BuckSize - 1 - cur;
            if (n <= leftSize)
            { //前进n步后依然在当前桶内
                tmp.cur += n;
            }
            else
            {
                n -= leftSize;
                size_t newBuckIndex = n % BuckSize - 1;
                size_t newMapIndex = mapIndex + n / BuckSize + 1;
                tmp.mapIndex = newMapIndex;
                tmp.cur = map[mapIndex] + newBuckIndex;
            }
            return tmp;
        }
        const_iterator operator-(const int &n) const
        {
            //TODO
            const_iterator tmp(*this);
            auto leftSize = cur - map[mapIndex];
            if (leftSize >= n)
            { //后退n步后依然在当前桶内
                tmp.cur -= n;
            }
            else
            {
                n -= leftSize;
                size_t newBuckIndex = n % BuckSize - 1;
                size_t newMapIndex = mapIndex + n / BuckSize + 1;
                tmp.mapIndex = newMapIndex;
                tmp.cur = map[mapIndex] + newBuckIndex;
            }
            return tmp;
        }
        // return th distance between two iterator,
        // if these two iterators points to different vectors, throw invaild_iterator.
        int operator-(const const_iterator &rhs) const
        {
            //TODO
            if (container != rhs.container)
            {
                throw invaild_iterator();
            }

            int tmp =
                (mapIndex - rhs.mapIndex) * BuckSize + (cur - map[mapIndex]) - (rhs.map[mapIndex] - rhs.cur);
            return tmp;
        }
        const_iterator &operator+=(const int &n)
        {
            //TODO
            auto leftSize = map[mapIndex] + BuckSize - 1 - cur;
            if (n <= leftSize)
            { //前进n步后依然在当前桶内
                cur += n;
            }
            else
            {
                n -= leftSize;
                size_t newBuckIndex = n % BuckSize - 1;
                size_t newMapIndex = mapIndex + n / BuckSize + 1;
                mapIndex = newMapIndex;
                cur = map[mapIndex] + newBuckIndex;
            }
            return *this;
        }
        const_iterator &operator-=(const int &n)
        {
            //TODO
            auto leftSize = cur - map[mapIndex];
            if (leftSize >= n)
            { //后退n步后依然在当前桶内
                cur -= n;
            }
            else
            {
                n -= leftSize;
                size_t newBuckIndex = n % BuckSize - 1;
                size_t newMapIndex = mapIndex + n / BuckSize + 1;
                mapIndex = newMapIndex;
                cur = map[mapIndex] + newBuckIndex;
            }
            return *this;
        }
        /**
         * TODO iter++
         */
        const_iterator operator++(int)
        {
            auto tmp = *this;
            ++*this;
            return tmp;
        }
        /**
         * TODO ++iter
         */
        const_iterator &operator++()
        {
            if (cur != map[mapIndex] + BuckSize - 1)
            { //+1之后依然在桶内
                ++cur;
            }
            else if (mapIndex + 1 < mapsize)
            { //已经在桶的结尾，但是之后还有新的map指针
                ++mapIndex;
                cur = map[mapIndex]; //指向下一个桶的开头
            }
            else
            { //mapIndex +1之后没有了map
                mapIndex = container->mapsize;

                cur = map[mapIndex]; //指向最后一个桶的最后一个元素的后方区域
            }
            return *this;
        }
        /**
         * TODO iter--
         */
        const_iterator operator--(int)
        {
            auto tmp = *this;
            --*this;
            return tmp;
        }
        /**
         * TODO --iter
         */
        const_iterator &operator--()
        {
            if (cur != map[mapIndex])
            {
                --cur;
            }
            else if (mapIndex - 1 >= 0)
            {
                --mapIndex;
                cur = map[mapIndex] + BuckSize - 1;
            }
            else
            {
                mapIndex = 0;
                cur = map[mapIndex];
            }
            return *this;
        }
        /**
         * TODO *it
         */
        T &operator*() const { return *cur; }
        /**
         * TODO it->field
         */
        T *operator->() const noexcept { return &(operator*()); }
        /**
         * a operator to check whether two iterators are same (pointing to the same memory).
         */
        bool operator==(const iterator &rhs) const { return ((mapIndex == rhs.mapIndex) && (cur == rhs.cur) && (container == rhs.container)); }
        bool operator==(const const_iterator &rhs) const { return ((mapIndex == rhs.mapIndex) && (cur == rhs.cur) && (container == rhs.container)); }
        /**
         * some other operator for iterator.
         */
        bool operator!=(const iterator &rhs) const { return !(rhs == *this); }
        bool operator!=(const const_iterator &rhs) const { return !(rhs == *this); }
    };
    /**
     * TODO Constructors
     */
    deque()
    {
        mapsize = 0;
        map = nullptr;
    }
    deque(const deque &other) {}
    /**
     * TODO Deconstructor
     */
    ~deque()
    {
        if (map != nullptr)
        {
            clear();
        }
    }
    /**
     * TODO assignment operator
     */
    deque &operator=(const deque &other) {}
    /**
     * access specified element with bounds checking
     * throw index_out_of_bound if out of bound.
     */
    T &at(const size_t &pos)
    {
        if (pos > size())
            throw index_out_of_bound();
        else
            return *(begin() + pos);
    }
    const T &at(const size_t &pos) const
    {
        if (pos > size())
            throw index_out_of_bound();
        else
            return *(begin() + pos);
    }
    T &operator[](const size_t &pos)
    {
        if (pos > size())
            throw index_out_of_bound();
        else
            return *(begin() + pos);
    }
    const T &operator[](const size_t &pos) const
    {
        if (pos > size())
            throw index_out_of_bound();
        else
            return *(begin() + pos);
    }
    /**
     * access the first element
     * throw containeris_empty when the container is empty.
     */
    const T &front() const
    {
        if (empty())
            throw containeris_empty();
        else
            return *begin();
    }
    /**
     * access the last element
     * throw containeris_empty when the container is empty.
     */
    const T &back() const
    {
        if (empty())
            throw containeris_empty();
        else
            return *(end() - 1);
    }
    /**
     * returns an iterator to the beginning.
     */
    iterator begin() { return start; }
    const_iterator cbegin() const { return start; }
    /**
     * returns an iterator to the end.
     */
    iterator end() { return finish; }
    const_iterator cend() const { return finish; }
    /**
     * checks whether the container is empty.
     */
    bool empty() const { return begin() == end(); }
    /**
     * returns the number of elements
     */
    size_t size() const { return end() - begin(); }
    /**
     * clears the contents
     */
    void clear()
    {
        //将除了头尾缓存区之外的所有缓存区中的元素析构（其都是饱和状态）
        for (T *node = map + start.mapIndex + 1; node < map + finish.mapIndex; ++node)
        {
            free(*node);
        }
        //释放头尾缓存区
        if (start.mapIndex != finish.mapIndex)
        { //至少有头尾两个缓存区
            //析构所有头尾缓存区元素
            free(start.cur, start.getNowBuckTail());
            free(finish.getNowBuckHead(), finish.cur);
            //释放尾部缓存区，根据设计，至少需要保留一个缓存区
        }
        else
        { //只有一个缓存区
            free(start.cur, finish.cur);
        }
        finish = start;
    }
    /**
     * inserts elements at the specified locat on in the container.
     * inserts value before pos
     * returns an iterator pointing to the inserted value
     *     throw if the iterator is invalid or it point to a wrong place.
     */
    iterator insert(iterator pos, const T &value) {}
    /**
     * removes specified element at pos.
     * removes the element at pos.
     * returns an iterator pointing to the following element, if pos pointing to the last element, end() will be returned.
     * throw if the container is empty, the iterator is invalid or it points to a wrong place.
     */
    iterator erase(iterator pos) {}
    /**
     * adds an element to the end
     */
    void push_back(const T &value) {}
    /**
     * removes the last element
     *     throw when the container is empty.
     */
    void pop_back() {
        if(empty())throw;
        else --finish;
    }
    /**
     * inserts an element to the beginning.
     */
    void push_front(const T &value) {}
    /**
     * removes the first element.
     *     throw when the container is empty.
     */
    void pop_front() {
        if(empty())throw;
        else ++start;
    }
};

} // namespace sjtu

#endif
