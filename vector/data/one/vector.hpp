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
     *   a class named const_iterator with same interfaces.
     */
        T *data;
    int memory;

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
        ~iterator() { delete p; }
        /**
         * return a new iterator which pointer n-next elements
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
        // if these two iterators point to different vectors, throw invaild_iterator.
        int operator-(const iterator &rhs) const
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
         * a operator to check whether two iterators are same (pointing to the same memory address).
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
        vector<T> *thisvector;
        const_iterator(const T *x = NULL,const vector<T> *temp = NULL) : p(x), thisvector(temp) {}
        ~const_iterator() { delete p; }
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
        data = NULL;
    }
    vector(const vector &other)
    {
        memory = other.memory;
        data = new T[memory];
        for (int i = 0; i < memory; i++)
        {
            data[i] = other.data[i];
        }
    }
    /**
     * TODO Destructor
     */
    ~vector()
    {
        memory = 0;
        delete[] data;
    }
    /**
     * TODO Assignment operator
     */
    vector &operator=(const vector &other)
    {
        if (this == &other)
            return *this;
        delete[] data;
        memory = other.memory;
        data = new T[memory];
        for (int i = 0; i < memory; i++)
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
        T *p = data + memory - 1;
        iterator iter(p, this);
        return iter;
    }
    const_iterator cend() const
    {
        const T *p = data + memory - 1;
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
            memory = 0;
            delete[] data;
        }
    }
    /**
     * inserts value before pos
     * returns an iterator pointing to the inserted value.
     */
    iterator insert(iterator pos, const T &value)
    {
        memory++;
        T *temp;
        temp = new T[memory];
        for (int i = 0; i < memory - 1; i++)
        {
            temp[i] = data[i];
        }
        T *temp2 = temp + memory;
        while (temp2 > pos.p)
        {
            *temp2 = *(temp2 - 1);
            --temp2;
        }
        *temp2 = value;
        delete[] data;
        data = temp;
        return pos;
    }
    /**
     * inserts value at index ind.
     * after inserting, this->at(ind) == value
     * returns an iterator pointing to the inserted value.
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
                memory++;
                T *temp;
                temp = new T[memory];
                for (int i = 0; i < memory - 1; i++)
                {
                    temp[i] = data[i];
                }
                temp[memory] = value;
                delete[] data;
                data = temp;
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
     * return an iterator pointing to the following element.
     * If the iterator pos refers the last element, the end() iterator is returned.
     */
    iterator erase(iterator pos)
    {
        T *temp;
        memory--;
        if (pos.p == data + memory - 1)
        {
            temp = new T[memory];
            for (int i = 0; i < memory; i++)
            {
                temp[i] = data[i];
            }
            delete[] data;
            data = temp;
            return end();
        }
        else
        {
            iterator iter(NULL, this);
            temp = new T[memory];
            size_t distance = pos.p - data;
            while (pos.p < data + memory)
            {
                *pos.p = *(pos.p);
                ++pos.p;
            }
            for (int i = 0; i < memory; i++)
            {
                temp[i] = data[i];
            }
            delete[] data;
            data = temp;
            iter.p = data + distance;
            return iter;
        }
    }
    /**
     * removes the element with index ind.
     * return an iterator pointing to the following element.
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
            T *temp;
            memory--;
            iterator iter(NULL, this);
            temp = new T[memory];
            int i = ind;
            while (i < memory)
            {
                data[i] = data[i + 1];
                ++i;
            }
            for (int i = 0; i < memory; i++)
            {
                temp[i] = data[i];
            }
            delete[] data;
            data = temp;
            iter.p = data + ind;
            return iter;
        }
    }
    /**
     * adds an element to the end.
     */
    void push_back(const T &value)
    {
        memory++;
        T *temp;
        temp = new T[memory];
        for (int i = 0; i < memory - 1; i++)
        {
            temp[i] = data[i];
        }
        temp[memory - 1] = value;
        delete[] data;
        data = temp;
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
            T *temp;
            temp = new T[memory];
            for (int i = 0; i < memory; i++)
            {
                temp[i] = data[i];
            }
            delete[] data;
            data = temp;
        }
    }
};

} // namespace sjtu

#endif
