#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP
#define BuckSize 500
#include "exceptions.hpp"

#include <cstddef>

namespace sjtu
{

template <class T>
class deque
{
    friend class iterator;
    friend class const_iterator;

public:
    class iterator;
    class const_iterator;
    iterator start, finish;
    T **map;
    long long mapsize;
    void reallocateMap()
    {
        size_t new_mapsize = mapsize * 3;

        //配置新map空间
        T **new_map = (T **)malloc(sizeof(T *) * new_mapsize);
        memset(new_map, 0, sizeof(T *) * new_mapsize);
        for (int i = 0; i < mapsize; ++i)
        {
            new_map[i] = (T *)malloc(sizeof(T) * BuckSize);
            memset(new_map[i], 0, sizeof(T) * BuckSize);
        }
        for (int i = 0; i < mapsize; ++i)
        {
            new_map[i + mapsize] = map[i];
        }
        for (int i = 0; i < mapsize; ++i)
        {
            new_map[i + 2 * mapsize] = (T *)malloc(sizeof(T) * BuckSize);
            memset(new_map[i + 2 * mapsize], 0, sizeof(T) * BuckSize);
        }
        //释放原map
        free(map);
        //设定新的map的地址和大小
        map = new_map;
        mapsize = new_mapsize;
        start = iterator(this, mapsize / 3 + start.mapIndex, start.cur);
        finish = iterator(this, mapsize / 3 + finish.mapIndex, finish.cur);
    }
    class iterator
    {
        friend class deque;

    public:
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
        iterator(deque<T> *tmp = nullptr, long long t = 0, T *k = nullptr) : container(tmp), mapIndex(t), cur(k) {}
        iterator(const iterator &it)
            : mapIndex(it.mapIndex), cur(it.cur), container(it.container) {}
        iterator operator=(const iterator &it)
        {
            if (this != &it)
            {
                this->cur = it.cur;
                this->mapIndex = it.mapIndex;
                this->container = it.container;
            }
            return *this;
        }
        iterator operator+(const int &n) const
        {
            //TODO
            iterator tmp(*this);
            int offset = n + cur - container->map[mapIndex];
            if (offset >= 0 && offset < BuckSize)
            {
                tmp.cur += n;
            }
            else
            {
                if (offset > 0)
                {
                    size_t newBuckIndex = offset % BuckSize;
                    size_t newMapIndex = mapIndex + offset / BuckSize;
                    tmp.mapIndex = newMapIndex;
                    tmp.cur = container->map[tmp.mapIndex] + newBuckIndex;
                }
                else
                {
                    int temp = -offset - 1;
                    size_t newBuckIndex = BuckSize - 1 - temp % BuckSize;
                    size_t newMapIndex = mapIndex - temp / BuckSize - 1;
                    tmp.mapIndex = newMapIndex;
                    tmp.cur = container->map[tmp.mapIndex] + newBuckIndex;
                }
            }
            return tmp;
        }
        iterator operator-(const int &n) const
        {
            //TODO
            iterator tmp(*this);
            tmp = tmp + (-n);
            return tmp;
        }
        // return th distance between two iterator,
        // if these two iterators points to different vectors, throw invalid_iterator().
        int operator-(const iterator &rhs) const
        {
            //TODO
            if (container != rhs.container)
            {
                throw invalid_iterator();
            }

            int tmp =
                (mapIndex - rhs.mapIndex - 1) * BuckSize + (cur - container->map[mapIndex]) + (rhs.container->map[rhs.mapIndex] + BuckSize - rhs.cur);
            return tmp;
        }
        iterator &operator+=(const int &n)
        {
            //TODO
            *this = *this + n;
            return *this;
        }
        iterator &operator-=(const int &n)
        {
            //TODO
            *this = *this - n;
            return *this;
        }
        /**
         * TODO iter++
         */
        iterator operator++(int)
        {
            iterator tmp = *this;
            ++*this;
            return tmp;
        }
        /**
         * TODO ++iter
         */
        iterator &operator++()
        {
            if (cur != container->map[mapIndex] + BuckSize - 1)
            {
                ++cur;
            }
            else if (mapIndex + 1 < container->mapsize)
            {
                ++mapIndex;
                cur = container->map[mapIndex]; //指向下一个桶的开头
            }
            else
            {
                mapIndex = container->mapsize;

                cur = container->map[mapIndex]; //指向最后一个桶的最后一个元素的后方区域
            }
            return *this;
        }
        /**
         * TODO iter--
         */
        iterator operator--(int)
        {
            iterator tmp = *this;
            --*this;
            return tmp;
        }
        /**
         * TODO --iter
         */
        iterator &operator--()
        {
            if (cur != container->map[mapIndex])
            {
                --cur;
            }
            else if (mapIndex - 1 >= 0)
            {
                --mapIndex;
                cur = container->map[mapIndex] + BuckSize - 1;
            }
            else
            {
                mapIndex = 0;
                cur = container->map[mapIndex];
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
    public:
        // data members.
        const deque<T> *container;
        long long mapIndex;
        const T *cur;

    public:
        const_iterator() : mapIndex(-1), cur(nullptr), container(nullptr)
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
            int offset = n + cur - container->map[mapIndex];
            if (offset >= 0 && offset < BuckSize)
            {
                tmp.cur += n;
            }
            else
            {
                if (offset > 0)
                {
                    size_t newBuckIndex = offset % BuckSize;
                    size_t newMapIndex = mapIndex + offset / BuckSize;
                    tmp.mapIndex = newMapIndex;
                    tmp.cur = container->map[tmp.mapIndex] + newBuckIndex;
                }
                else
                {
                    int temp = -offset - 1;
                    size_t newBuckIndex = BuckSize - 1 - temp % BuckSize;
                    size_t newMapIndex = mapIndex - temp / BuckSize - 1;
                    tmp.mapIndex = newMapIndex;
                    tmp.cur = container->map[tmp.mapIndex] + newBuckIndex;
                }
            }
            return tmp;
        }
        const_iterator operator-(const int &n) const
        {
            //TODO
            const_iterator tmp(*this);
            tmp = tmp + (-n);
            return tmp;
        }
        // return th distance between two iterator,
        // if these two iterators points to different vectors, throw invalid_iterator().
        int operator-(const const_iterator &rhs) const
        {
            //TODO
            if (container != rhs.container)
            {
                throw invalid_iterator();
            }

            int tmp =
                (mapIndex - rhs.mapIndex) * BuckSize + (cur - container->map[mapIndex]) - (map[rhs.mapIndex] - rhs.cur);
            return tmp;
        }
        const_iterator &operator+=(const int &n)
        {
            //TODO
            *this = *this + n;
            return *this;
        }
        const_iterator &operator-=(const int &n)
        {
            //TODO
            *this = *this - n;
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
            if (cur != container->map[mapIndex] + BuckSize - 1)
            {
                ++cur;
            }
            else if (mapIndex + 1 < mapsize)
            {
                ++mapIndex;
                cur = container->map[mapIndex]; //指向下一个桶的开头
            }
            else
            {
                mapIndex = container->mapsize;

                cur = container->map[mapIndex]; //指向最后一个桶的最后一个元素的后方区域
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
            if (cur != container->map[mapIndex])
            {
                --cur;
            }
            else if (mapIndex - 1 >= 0)
            {
                --mapIndex;
                cur = container->map[mapIndex] + BuckSize - 1;
            }
            else
            {
                mapIndex = 0;
                cur = container->map[mapIndex];
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
        mapsize = 1;
        map = (T **)malloc(sizeof(T *) * mapsize);
        memset(map, 0, sizeof(T *) * mapsize);
        map[0] = (T *)malloc(sizeof(T) * BuckSize);
        memset(map[0], 0, sizeof(T) * BuckSize);
        start = iterator(this, 0, map[0]);
        finish = iterator(this, 0, map[0]);
    }
    deque(const deque &other)
    {
        mapsize = other.mapsize;
        map = (T **)malloc(sizeof(T *) * mapsize);
        memset(map, 0, sizeof(T *) * mapsize);
        for (long long i = 0; i < mapsize; i++)
        {
            map[i] = (T *)malloc(sizeof(T) * BuckSize);
            memset(map[i], 0, sizeof(T) * BuckSize);
        }
        int
            first = other.start.cur - other.map[other.start.mapIndex],
            last = other.finish.cur - other.map[other.finish.mapIndex];
        start = iterator(this, other.start.mapIndex, map[other.start.mapIndex] + first);
        finish = iterator(this, other.finish.mapIndex, map[other.finish.mapIndex] + last);
        for (int i = 0; i < other.size(); ++i)
        {
            new (start.cur) T(*(other.start + i));
        }
    }
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
    deque &operator=(const deque &other)
    {
        if (this == &other)
            return *this;
        for (int i = 0; i < size(); ++i)
        {
            (start + i).cur->~T();
        }
        for (long long i = 0; i < mapsize; i++)
        {
            if (!map[i])
                free(map[i]);
        }
        if (!map)
            free(map);
        mapsize = other.mapsize;
        map = (T **)malloc(sizeof(T *) * mapsize);
        memset(map, 0, sizeof(T *) * mapsize);
        for (long long i = 0; i < mapsize; i++)
        {
            map[i] = (T *)malloc(sizeof(T) * BuckSize);
            memset(map[i], 0, sizeof(T) * BuckSize);
        }
        int
            first = other.start.cur - other.map[other.start.mapIndex],
            last = other.finish.cur - other.map[other.finish.mapIndex];
        start = iterator(this, other.start.mapIndex, map[other.start.mapIndex] + first);
        finish = iterator(this, other.finish.mapIndex, map[other.finish.mapIndex] + last);
        for (int i = 0; i < other.size(); ++i)
        {
            new ((start+i).cur) T(*(other.start + i));
        }
        return *this;
    }
    /**
     * access specified element with bounds checking
     * throw index_out_of_bound if out of bound.
     */
    T &at(const size_t &pos)
    {
        if (pos > size())
            throw index_out_of_bound();
        else
            return *(start + pos);
    }
    const T &at(const size_t &pos) const
    {
        if (pos > size())
            throw index_out_of_bound();
        else
            return *(start + pos);
    }
    T &operator[](const size_t &pos)
    {
        if (pos > size())
            throw index_out_of_bound();
        else
            return *(start + pos);
    }
    const T &operator[](const size_t &pos) const
    {
        if (pos > size())
            throw index_out_of_bound();
        else
            return *(start + pos);
    }
    /**
     * access the first element
     * throw container_is_empty() when the container is empty.
     */
    const T &front() const
    {
        if (empty())
            throw container_is_empty();
        else
            return *start;
    }
    /**
     * access the last element
     * throw container_is_empty() when the container is empty.
     */
    const T &back() const
    {
        if (empty())
            throw container_is_empty();
        else
            return *(finish - 1);
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
    bool empty() const { return start == finish; }
    /**
     * returns the number of elements
     */
    size_t size() const { return finish - start; }
    /**
     * clears the contents
     */
    void clear()
    {
        for (int i = 0; i < size(); ++i)
        {
            (start + i).cur->~T();
        }
        for (long long i = 0; i < mapsize; ++i)
        {
            free(map[i]);
        }
        free(map);
        mapsize = 1;
        finish = iterator(this, 0, map[0]);
        start = iterator(this, 0, map[0]);
    }
    void copy(iterator bgn, iterator end, iterator front)
    {
        while (true)
        {
            if (bgn == end)
                return;
            new (front.cur) T(*bgn);
            ++front;
            ++bgn;
        }
    }
    void copy_backward(iterator bgn, iterator end, iterator back)
    {
        while (true)
        {
            new (back.cur) T(*end);
            if (end == bgn)
                return;
            --back;
            --end;
        }
    }
    /**
     * inserts elements at the specified locat on in the container.
     * inserts value before pos
     * returns an iterator pointing to the inserted value
     *     throw if the iterator is invalid or it point to a wrong place.
     */
    iterator insert(iterator pos, const T &value)
    {
        if (pos.cur == start.cur)
        {                      //如果插入点是 deque 的最前端
            push_front(value); //直接调用push_front()
            return start;
        }
        else if (pos.cur == finish.cur)
        {                     //插入点是 deque 的最尾端
            push_back(value); //交给push_back去处理
            iterator tmp(finish);
            --tmp;
            return tmp;
        }
        else
        {
            T x_copy = value;
            long long elem_before = pos - start; //插入点之前的元素个数
            if (elem_before < 0 || elem_before > finish - start)
                throw;
            if (elem_before < (finish - pos))
            {
                iterator front_old(start);          //记录最初的起始位置
                push_front(front());                //在最前端加入一个与第一元素相同的元素
                iterator move_front(front_old + 1); //原起始位置的元素已压入最前端，因此从原起始位置的下一位置开始移动
                copy(move_front, pos, front_old);   //将 [move_front, pos) 内的元素前移一格
                --pos;                              //pos 前移指向插入位置
            }
            else
            {
                iterator back_old = finish;
                push_back(back());
                iterator move_back = back_old - 1;       //从原结束位置的前一位置开始复制
                copy_backward(pos, move_back, back_old); //移动元素
                                                         //注意：pos 已经指向正确的插入位置
            }
            *pos.cur = x_copy;
            return pos;
        }
    }
    /**
     * removes specified element at pos.
     * removes the element at pos.
     * returns an iterator pointing to the following element, if pos pointing to the last element, end() will be returned.
     * throw if the container is empty, the iterator is invalid or it points to a wrong place.
     */
    iterator erase(iterator pos)
    {
        iterator next(pos);
        long long elem_before = pos - start; //清除点之前的元素个数
        if (empty() || elem_before < 0 || elem_before > finish - start)
            throw;
        if (elem_before < (finish - next))
        {
            --next;
            copy_backward(start, next, pos);
            pop_front(); //移动完毕，第一个元素多余，将其清除
        }
        else
        {
            ++next;
            copy(next, finish, pos);
            new ((finish - 1).cur) T(*finish);
            pop_back(); //移动完毕，最后一个元素多余，将其清除
        }
        return start + elem_before;
    }
    /**
     * adds an element to the end
     */
    void push_back(const T &value)
    {
        if (finish.cur == map[mapsize - 1] + BuckSize - 1)
        {
            reallocateMap();
            map[finish.mapIndex + 1] = (T *)malloc(sizeof(T) * BuckSize); //在当前区块地址元素的后一个元素赋值为新分配的内存空间
            memset(map[finish.mapIndex + 1], 0, sizeof(T) * BuckSize);
            new (finish.cur) T(value);
            ++finish.mapIndex; //调整finish_的区块位置
            finish.cur = map[finish.mapIndex];
        }
        else
        {
            new (finish.cur) T(value);
            ++finish;
        }
    }
    /**
     * removes the last element
     *     throw when the container is empty.
     */
    void pop_back()
    {
        if (empty())
            throw;
        else
        {
            --finish;
            finish.cur->~T();
        }
    }
    /**
     * inserts an element to the beginning.
     */
    void push_front(const T &value)
    {
        if (begin().cur == map[0])
        {
            reallocateMap();
            map[start.mapIndex - 1] = (T *)malloc(sizeof(T) * BuckSize);
            memset(map[start.mapIndex - 1], 0, sizeof(T) * BuckSize);
            --start.mapIndex;
            start.cur = map[start.mapIndex] + BuckSize - 1;
            new (start.cur) T(value);
        }
        else
        {
            --start;
            new (start.cur) T(value);
        }
    }
    /**
     * removes the first element.
     *     throw when the container is empty.
     */
    void pop_front()
    {
        if (empty())
            throw;
        else
        {
            start.cur->~T();
            ++start;
        }
    }
};

} // namespace sjtu

#endif
