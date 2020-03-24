#ifndef SJTU_VECTOR_HPP
#define SJTU_VECTOR_HPP

#include "exceptions.hpp"

#include <climits>
#include <cstddef>

namespace sjtu
{
/**
 * a data container like std::vector
 * store data in a successive memory and support random access.
 */
template <typename T>
class vector
{
public:
    /**
     * TODO
     * a type for actions of the elements of a vector, and you should write
     *   a class named const_iterator with same long longerfaces.
     */
    T *data;
    long long memory;
    long long vectortop;
    void doublespace()
    {
        T *temp;
        vectortop *= 2;
        temp = (T *)malloc(sizeof(T) * vectortop);
        memset(temp, 0, sizeof(T) * vectortop);
        for (long long i = 0; i < memory; i++)
        {
            temp[i] = data[i];
            data[i].~T();
        }
        free(data);
        data = temp;
    }
    /**
     * you can see RandomAccessIterator at CppReference for help.
     */
    class const_iterator;
    class iterator
    {
    public:
        /**
         * TODO add data members
         *   just add whatever you want.
         */
        T *p;
        vector<T> *thisvector;
        iterator(T *x = NULL, vector<T> *temp = NULL) : p(x), thisvector(temp) {}
        /**
         * return a new iterator which polong longer n-next elements
         * as well as operator-
         */
        iterator operator+(const int &n) const
        {
            //TODO
            iterator temp(p + n, thisvector);
            return temp;
        }
        iterator operator-(const int &n) const
        {
            //TODO
            iterator temp(p - n, thisvector);
            return temp;
        }
        // return the distance between two iterators,
        // if these two iterators polong long to different vectors, throw invaild_iterator.
        long long operator-(const iterator &rhs) const
        {
            //TODO
            if (thisvector == rhs.thisvector)
                return p - rhs.p;
            else
            {
                throw invalid_iterator();
            }
        }
        iterator &operator+=(const int &n)
        {
            //TODO
            p += n;
            return *this;
        }
        iterator &operator-=(const int &n)
        {
            //TODO
            p -= n;
            return *this;
        }
        /**
         * TODO iter++
         */
        iterator operator++(int)
        {
            iterator temp = *this;
            p++;
            return temp;
        }
        /**
         * TODO ++iter
         */
        iterator &operator++()
        {
            p++;
            return *this;
        }
        /**
         * TODO iter--
         */
        iterator operator--(int)
        {
            iterator temp = *this;
            p--;
            return temp;
        }
        /**
         * TODO --iter
         */
        iterator &operator--()
        {
            p--;
            return *this;
        }
        /**
         * TODO *it
         */
        T &operator*() const
        {
            return *p;
        }
        /**
         * a operator to check whether two iterators are same (polong longing to the same memory address).
         */
        bool operator==(const iterator &rhs) const { return p == rhs.p; }
        bool operator==(const const_iterator &rhs) const { return p == rhs.p; }
        /**
         * some other operator for iterator.
         */
        bool operator!=(const iterator &rhs) const { return p != rhs.p; }
        bool operator!=(const const_iterator &rhs) const { return p != rhs.p; }
    };
    /**
     * TODO
     * has same function as iterator, just for a const object.
     */
    class const_iterator
    {
    public:
        T *p;
        const vector<T> *thisvector;
        const_iterator(T *x = NULL, const vector<T> *temp = NULL) : p(x), thisvector(temp) {}
        /**
         * return a new iterator which polong longer n-next elements
         * as well as operator-
         */
        const_iterator operator+(const int &n) const
        {
            //TODO
            const_iterator temp(p + n, thisvector);
            return temp;
        }
        const_iterator operator-(const int &n) const
        {
            //TODO
            const_iterator temp(p - n, thisvector);
            return temp;
        }
        // return the distance between two iterators,
        // if these two iterators polong long to different vectors, throw invaild_iterator.
        long long operator-(const const_iterator &rhs) const
        {
            //TODO
            if (thisvector == rhs.thisvector)
                return p - rhs.p;
            else
            {
                throw invalid_iterator();
            }
        }
        const_iterator &operator+=(const int &n)
        {
            //TODO
            p += n;
            return *this;
        }
        const_iterator &operator-=(const int &n)
        {
            //TODO
            p -= n;
            return *this;
        }
        /**
         * TODO iter++
         */
        const_iterator operator++(int)
        {
            const_iterator temp = *this;
            p++;
            return temp;
        }
        /**
         * TODO ++iter
         */
        const_iterator &operator++()
        {
            p++;
            return *this;
        }
        /**
         * TODO iter--
         */
        const_iterator operator--(int)
        {
            const_iterator temp = *this;
            p--;
            return temp;
        }
        /**
         * TODO --iter
         */
        const_iterator &operator--()
        {
            p--;
            return *this;
        }
        T &operator*() const
        {
            return *p;
        }
        bool operator==(const iterator &rhs) const { return p == rhs.p; }
        bool operator==(const const_iterator &rhs) const { return p == rhs.p; }
        bool operator!=(const iterator &rhs) const { return p != rhs.p; }
        bool operator!=(const const_iterator &rhs) const { return p != rhs.p; }
    };
    /**
     * TODO Constructs
     * Atleast two: default constructor, copy constructor
     */
    vector()
    {
        memory = 0;
        vectortop = 1;
        data = (T *)malloc(sizeof(T) * vectortop);
        memset(data, 0, sizeof(T) * vectortop);
    }
    vector(const vector &other)
    {
        vectortop = other.vectortop;
        memory = other.memory;
        data = (T *)malloc(sizeof(T) * vectortop);
        memset(data, 0, sizeof(T) * vectortop);
        for (long long i = 0; i < memory; i++)
        {
            data[i] = other.data[i];
        }
    }
    /**
     * TODO Destructor
     */
    ~vector()
    {
        for (long long i = 0; i < memory; i++)
        {
            data[i].~T();
        }
        memory = 0;
        vectortop = 1;
        free(data);
    }
    /**
     * TODO Assignment operator
     */
    vector &operator=(const vector &other)
    {
        if (this == &other)
            return *this;
        for (long long i = 0; i < memory; i++)
        {
            data[i].~T();
        }
        free(data);
        vectortop = other.vectortop;
        memory = other.memory;
        data = (T *)malloc(sizeof(T) * vectortop);
        memset(data, 0, sizeof(T) * vectortop);
        for (long long i = 0; i < memory; i++)
        {
            data[i] = other.data[i];
        }
        return *this;
    }
    /**
     * assigns specified element with bounds checking
     * throw index_out_of_bound if pos is not in [0, memory)
     */
    T &at(const size_t &pos)
    {
        if (pos >= 0 && pos < memory)
            return data[pos];
        else
        {
            throw index_out_of_bound();
        }
    }
    const T &at(const size_t &pos) const
    {
        if (pos >= 0 && pos < memory)
            return data[pos];
        else
        {
            throw index_out_of_bound();
        }
    }
    /**
     * assigns specified element with bounds checking
     * throw index_out_of_bound if pos is not in [0, memory)
     * !!! Pay attentions
     *   In STL this operator does not check the boundary but I want you to do.
     */
    T &operator[](const size_t &pos)
    {
        if (pos >= 0 && pos < memory)
            return data[pos];
        else
        {
            throw index_out_of_bound();
        }
    }
    const T &operator[](const size_t &pos) const
    {
        if (pos >= 0 && pos < memory)
            return data[pos];
        else
        {
            throw index_out_of_bound();
        }
    }
    /**
     * access the first element.
     * throw container_is_empty if memory == 0
     */
    const T &front() const
    {
        if (memory == 0)
            throw container_is_empty();
        else
        {
            return data[0];
        }
    }
    /**
     * access the last element.
     * throw container_is_empty if memory == 0
     */
    const T &back() const
    {
        if (memory == 0)
            throw container_is_empty();
        else
        {
            return data[memory - 1];
        }
    }
    /**
     * returns an iterator to the beginning.
     */
    iterator begin()
    {
        iterator iter(data, this);
        return iter;
    }
    const_iterator cbegin() const
    {
        const_iterator iter(data, this);
        return iter;
    }
    /**
     * returns an iterator to the end.
     */
    iterator end()
    {
        T *p = data + memory;
        iterator iter(p, this);
        return iter;
    }
    const_iterator cend() const
    {
        T *p = data + memory;
        const_iterator iter(p, this);
        return iter;
    }
    /**
     * checks whether the container is empty
     */
    bool empty() const
    {
        if (memory == 0)
            return true;
        else
        {
            return false;
        }
    }
    /**
     * returns the number of elements
     */
    size_t size() const
    {
        return memory;
    }
    /**
     * clears the contents
     */
    void clear()
    {
        if (memory > 0)
        {
            for (long long i = 0; i < memory; i++)
            {
                data[i].~T();
            }
            memory = 0;
            vectortop = 1;
            free(data);
        }
    }
    /**
     * inserts value before pos
     * returns an iterator polong longing to the inserted value.
     */
    iterator insert(iterator pos, const T &value)
    {
        long long i = pos.p - data;
        if (memory >= vectortop)
        {
            doublespace();
        }
        memory++;
        for (int j = memory - 1; j > i; --j)
        {
            data[j] = data[j - 1];
        }
        data[i] = value;
        pos.p = data + i;
        return pos;
    }
    /**
     * inserts value at index ind.
     * after inserting, this->at(ind) == value
     * returns an iterator polong longing to the inserted value.
     * throw index_out_of_bound if ind > memory (in this situation ind can be memory because after inserting the memory will increase 1.)
     */
    iterator insert(const size_t &ind, const T &value)
    {
        iterator iter(NULL, this);
        iter.p = data + ind;
        if (ind > memory)
            throw index_out_of_bound();
        else
        {
            if (ind == memory)
            {
                if (memory >= vectortop)
                {
                    doublespace();
                }
                memory++;
                data[memory - 1] = value;
                return iter;
            }
            else
            {
                data[ind] = value;
                return iter;
            }
        }
    }
    /**
     * removes the element at pos.
     * return an iterator polong longing to the following element.
     * If the iterator pos refers the last element, the end() iterator is returned.
     */
    iterator erase(iterator pos)
    {
        T *temp;
        memory--;
        if (pos.p == data + memory)
        {
            return end();
        }
        else
        {
            long long i = pos.p - data;
            for (; i < memory; i++)
            {
                data[i] = data[i + 1];
            }
            return pos;
        }
    }
    /**
     * removes the element with index ind.
     * return an iterator polong longing to the following element.
     * throw index_out_of_bound if ind >= memory
     */
    iterator erase(const size_t &ind)
    {
        if (ind >= memory)
        {
            throw index_out_of_bound();
        }
        else
        {
            memory--;
            long long i = ind;
            while (i < memory)
            {
                data[i] = data[i + 1];
                ++i;
            }
            iterator iter(data + ind, this);
            return iter;
        }
    }
    /**
     * adds an element to the end.
     */
    void push_back(const T &value)
    {
        if (memory >= vectortop)
        {
            doublespace();
        }
        memory++;
        data[memory - 1] = value;
    }
    /**
     * remove the last element from the end.
     * throw container_is_empty if memory() == 0
     */
    void pop_back()
    {
        if (memory == 0)
            throw container_is_empty();
        else
        {
            memory--;
        }
    }
};

} // namespace sjtu

#endif
