#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP
#define BuckSize 800
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
    struct node
    {
        int len;
        T *data;
        node *next;
        node *before;
        node(const node &other)
        {
            next = before = 0;
            data = (T *)malloc(sizeof(T) * BuckSize);
            memset(data, 0, BuckSize * sizeof(T));
            len = other.len;
            for (int i = 0; i < len; i++)
            {
                new (data + i) T(other.data[i]);
            }
        }
        node()
        {
            data = (T *)malloc(sizeof(T) * BuckSize);
            memset(data, 0, BuckSize * sizeof(T));
            next = before = 0;
            len = 0;
        }
        ~node()
        {
            for (int i = 0; i < len; i++)
                data[i].~T();
            free(data);
            next = before = 0;
            len = 0;
        }
    };
    void split(node *ptr)
    {
        node *tmp = ptr->next;
        ptr->next = new node;
        ptr->len = ptr->next->len = BuckSize / 2;
        for (int i = BuckSize / 2; i < BuckSize; ++i)
        {
            new (ptr->next->data + i - BuckSize / 2) T(ptr->data[i]);
            ptr->data[i].~T();
        }
        ptr->next->next = tmp;
        ptr->next->before = ptr;
        if (tmp != 0)
            tmp->before = ptr->next;
        else
            rear = rear->next;
    }
    void merge(node *ptr)
    {
        node *tmp = ptr->next->next;
        for (int i = 0; i < ptr->next->len; i++)
            new (ptr->data + ptr->len + i) T(ptr->next->data[i]);

        ptr->len += ptr->next->len;
        delete ptr->next;
        ptr->next = tmp;
        if (tmp != 0)
            tmp->before = ptr;
        else
            rear = ptr;
    }
    node *first, *rear;
    size_t totlen;
    class const_iterator;
    class iterator
    {
        friend class deque;

    public:
        /**
         * TODO add data members
         *   just add whatever you want.
         */
        deque<T> *container;
        node *mapIndex;
        int cur;

    public:
        /**
         * return a new iterator which pointer n-next elements
         *   even if there are not enough elements, the behaviour is **undefined**.
         * as well as operator-
         */
        iterator(deque<T> *tmp = nullptr, node *t = nullptr, int k = 0) : container(tmp), mapIndex(t), cur(k) {}
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
            if (n == 0)
                return *this;
            //TODO
            iterator tmp(*this);
            int offset = n + cur;
            if (offset >= 0 && offset < mapIndex->len)
            {
                tmp.cur += n;
            }
            else
            {
                if (offset > 0)
                {
                    while (tmp.mapIndex->next != 0 && offset >= tmp.mapIndex->len)
                    {
                        offset -= tmp.mapIndex->len;
                        tmp.mapIndex = tmp.mapIndex->next;
                    }
                    tmp.cur = offset;
                }
                else
                {
                    int temp = -offset;
                    if (tmp.mapIndex->before == nullptr)
                        throw invalid_iterator();
                    tmp.mapIndex = tmp.mapIndex->before;
                    while (tmp.mapIndex->before != 0 && temp > tmp.mapIndex->len)
                    {
                        temp -= tmp.mapIndex->len;
                        tmp.mapIndex = tmp.mapIndex->before;
                    }
                    tmp.cur = tmp.mapIndex->len - temp;
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

            node *find = container->first;
            while (find != mapIndex && find != rhs.mapIndex)
            {
                find = find->next;
            }
            node *findnext = find;
            int num = 0;
            if (find == mapIndex)
            {
                while (findnext != rhs.mapIndex)
                {
                    num += findnext->len;
                    findnext = findnext->next;
                }
                num = num - cur + rhs.cur;
                return -num;
            }
            else
            {
                while (findnext != mapIndex)
                {
                    num += findnext->len;
                    findnext = findnext->next;
                }
                num = num + cur - rhs.cur;
                return num;
            }
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
            *this += 1;
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
            *this -= 1;
            return *this;
        }
        /**
         * TODO *it
         */
        T &operator*() const
        {
            int n = this->cur;
            if (n < 0 || n >= mapIndex->len)
                throw invalid_iterator();
            return mapIndex->data[cur];
        }
        /**
         * TODO it->field
         */
        T *operator->() const noexcept
        {
            int n = this->cur;
            if (n < 0 || n >= mapIndex->len)
                throw invalid_iterator();
            return &(operator*());
        }
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
        const node *mapIndex;
        int cur;

    public:
        const_iterator() : mapIndex(nullptr), cur(0), container(nullptr)
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
            if (n == 0)
                return *this;
            const_iterator tmp(*this);
            int offset = n + cur;
            if (offset >= 0 && offset < mapIndex->len)
            {
                tmp.cur += n;
            }
            else
            {
                if (offset > 0)
                {
                    while (tmp.mapIndex->next != 0 && offset >= tmp.mapIndex->len)
                    {
                        offset -= tmp.mapIndex->len;
                        tmp.mapIndex = tmp.mapIndex->next;
                    }
                    tmp.cur = offset;
                }
                else
                {
                    int temp = -offset;
                    tmp.mapIndex = tmp.mapIndex->before;
                    while (tmp.mapIndex->before != 0 && temp > tmp.mapIndex->len)
                    {
                        temp -= tmp.mapIndex->len;
                        tmp.mapIndex = tmp.mapIndex->before;
                    }
                    tmp.cur = tmp.mapIndex->len - temp;
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

            node *find = container->first;
            while (find != mapIndex && find != rhs.mapIndex)
            {
                find = find->next;
            }
            node *findnext = find;
            int num = 0;
            if (find == mapIndex)
            {
                while (findnext != rhs.mapIndex)
                {
                    num += findnext->len;
                    findnext = findnext->next;
                }
                num = num - cur + rhs.cur;
                return -num;
            }
            else
            {
                while (findnext != mapIndex)
                {
                    num += findnext->len;
                    findnext = findnext->next;
                }
                num = num + cur - rhs.cur;
                return num;
            }
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
            *this += 1;
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
            *this -= 1;
            return *this;
        }
        /**
         * TODO *it
         */
        T &operator*() const
        {
            int n = this->cur;
            if (n < 0 || n >= mapIndex->len)
                throw invalid_iterator();
            return mapIndex->data[cur];
        }
        /**
         * TODO it->field
         */
        T *operator->() const noexcept
        {
            int n = this->cur;
            if (n < 0 || n >= mapIndex->len)
                throw invalid_iterator();
            return &(operator*());
        }
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
        first = new node;
        rear = first;
        totlen = 0;
    }
    deque(const deque &other)
    {
        totlen = other.totlen;
        if (other.first == nullptr)
        {
            first = new node;
            rear = first;
            totlen = 0;
            return;
        }
        first = new node(*other.first);
        node *tmp1 = first, *tmp2 = other.first;
        while (tmp2 != other.rear)
        {
            tmp1->next = new node(*(tmp2->next));
            tmp1->next->before = tmp1;
            tmp1 = tmp1->next;
            tmp2 = tmp2->next;
        }
        rear = tmp1;
    }
    /**
     * TODO Deconstructor
     */
    ~deque()
    {
        node *tmp;
        while (first != 0)
        {
            tmp = first;
            first = first->next;
            delete tmp;
        }
        rear = first;
        totlen = 0;
    }
    /**
     * TODO assignment operator
     */
    deque &operator=(const deque &other)
    {
        if (this == &other)
            return *this;
        this->~deque();
        if (other.first == nullptr)
        {
            first = new node;
            rear = first;
            return *this;
        }
        first = new node(*other.first);
        node *tmp1 = first, *tmp2 = other.first;
        while (tmp2 != other.rear)
        {
            tmp1->next = new node(*(tmp2->next));
            tmp1->next->before = tmp1;
            tmp1 = tmp1->next;
            tmp2 = tmp2->next;
        }
        rear = tmp1;
        totlen = other.totlen;
        return *this;
    }
    /**
     * access specified element with bounds checking
     * throw index_out_of_bound if out of bound.
     */
    T &at(const size_t &pos)
    {
        if (pos >= size() || pos < 0)
            throw index_out_of_bound();
        else
            return *(begin() + pos);
    }
    const T &at(const size_t &pos) const
    {
        if (pos >= size() || pos < 0)
            throw index_out_of_bound();
        else
            return *(cbegin() + pos);
    }
    T &operator[](const size_t &pos)
    {
        if (pos >= size() || pos < 0)
            throw index_out_of_bound();
        else
            return *(begin() + pos);
    }
    const T &operator[](const size_t &pos) const
    {
        if (pos >= size() || pos < 0)
            throw index_out_of_bound();
        else
            return *(cbegin() + pos);
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
            return first->data[0];
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
            return rear->data[rear->len - 1];
    }
    /**
     * returns an iterator to the beginning.
     */
    iterator begin() { return iterator(this, first, 0); }
    const_iterator cbegin() const
    {
        const_iterator tmp;
        tmp.container = this;
        tmp.mapIndex = first;
        tmp.cur = 0;
        return tmp;
    }
    /**
     * returns an iterator to the end.
     */
    iterator end() { return iterator(this, rear, rear->len); }
    const_iterator cend() const
    {
        const_iterator tmp;
        tmp.container = this;
        tmp.mapIndex = rear;
        tmp.cur = rear->len;
        return tmp;
    }
    /**
     * checks whether the container is empty.
     */
    bool empty() const { return first->len == 0; }
    /**
     * returns the number of elements
     */
    size_t size() const
    {
        return totlen;
    }
    /**
     * clears the contents
     */
    void clear()
    {
        this->~deque();
        first = new node;
        totlen = 0;
        rear = first;
    }
    /**
     * inserts elements at the specified locat on in the container.
     * inserts value before pos
     * returns an iterator pointing to the inserted value
     *     throw if the iterator is invalid or it point to a wrong place.
     */
    iterator insert(iterator pos, const T &value)
    {
        if (pos.container != this)
            throw invalid_iterator();
        if (pos == end())
        {
            push_back(value);
            return end() - 1;
        }
        if (pos.mapIndex->len < pos.cur + 1)
            throw invalid_iterator();
        int i = pos.mapIndex->len;
        new (pos.mapIndex->data + i) T(pos.mapIndex->data[i - 1]);
        --i;
        for (; i > pos.cur; i--)
        {
            pos.mapIndex->data[i] = pos.mapIndex->data[i - 1];
        }
        pos.mapIndex->data[pos.cur] = value;
        if ((++pos.mapIndex->len) == BuckSize)
        {
            split(pos.mapIndex);
            if (pos.cur >= (BuckSize / 2))
            {
                pos.mapIndex = pos.mapIndex->next;
                pos.cur -= (BuckSize / 2);
            }
        }
        totlen++;
        return pos;
    }
    /**
         * removes specified element at pos.
         * removes the element at pos.
         * returns an iterator pointing to the following element, if pos pointing to the last element, end() will be returned.
         * throw if the container is empty, the iterator is invalid or it points to a wrong place.
         */
    iterator erase(iterator pos)
    {
        if (pos.mapIndex->len < pos.cur || pos.cur < 0)
            throw invalid_iterator();
        if (pos == end() - 1)
        {
            pop_back();
            return end();
        }
        for (int i = pos.cur; i < pos.mapIndex->len - 1; i++)
        {
            pos.mapIndex->data[i] = pos.mapIndex->data[i + 1];
        }
        pos.mapIndex->data[pos.mapIndex->len - 1].~T();
        --(pos.mapIndex->len);
        if (pos.mapIndex->before != 0 && ((pos.mapIndex->len + pos.mapIndex->before->len) < BuckSize))
        {
            pos.mapIndex = pos.mapIndex->before;
            pos.cur += (pos.mapIndex->len);
            merge(pos.mapIndex);
        }
        else
        {
            if (pos.mapIndex->next != 0 && ((pos.mapIndex->len + pos.mapIndex->next->len) < BuckSize))
            {
                merge(pos.mapIndex);
            }
        }
        if (pos.mapIndex->len == pos.cur)
        {
            if (pos.mapIndex->next != 0)
            {
                pos.mapIndex = pos.mapIndex->next;
                pos.cur = 0;
            }
        }
        totlen--;
        return pos;
    }
    /**
     * adds an element to the end
     */
    void push_back(const T &value)
    {
        new (rear->data + rear->len) T(value);
        if ((++rear->len) == BuckSize)
            split(rear);
        totlen++;
    }
    /**
         * removes the last element
         *     throw when the container is empty.
         */
    void pop_back()
    {
        if (first->len == 0)
            throw container_is_empty();
        rear->data[rear->len - 1].~T();
        --(rear->len);
        if (rear->before != 0 && ((rear->len + rear->before->len) < BuckSize))
        {
            merge(rear->before);
        }
        totlen--;
    }
    /**
         * inserts an element to the beginning.
         */
    void push_front(const T &value)
    {
        int i = first->len;
        if (i == 0)
        {
            new (first->data) T(value);
            ++first->len;
            totlen++;
            return;
        }
        new (first->data + i) T(first->data[i - 1]);
        --i;
        for (; i > 0; i--)
        {
            first->data[i] = first->data[i - 1];
        }
        first->data[0] = value;
        if ((++first->len) == BuckSize)
            split(first);
        totlen++;
    }
    /**
         * removes the first element.
         *     throw when the container is empty.
         */
    void pop_front()
    {
        if (first->len == 0)
            throw container_is_empty();
        --first->len;
        for (int i = 0; i < first->len; i++)
        {
            first->data[i] = first->data[i + 1];
        }
        first->data[first->len].~T();
        if (first->next != 0 && (first->len + first->next->len) < BuckSize)
        {
            merge(first);
        }
        totlen--;
    }
};

} // namespace sjtu

#endif
