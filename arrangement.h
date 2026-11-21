#ifndef ARRANGEMENT_H_INCLUDED
#define ARRANGEMENT_H_INCLUDED

#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

template<typename T>
class arrangement;
template<typename T>
struct is_arrangement : std::false_type {};
template<typename T>
struct is_arrangement<arrangement<T>> : std::true_type {};
template<typename T>
class arrangement final {
    private:
    T* data;
    size_t _size;
    size_t _capacity;

    public:
    arrangement(size_t capacity) : data(nullptr), _size(0), _capacity(capacity)
    {
        if (_capacity > SIZE_MAX/sizeof(T)) throw std::logic_error("Arrangement is too big.");
        if (_capacity == 0) return;

        data = static_cast<T*>( std::malloc(sizeof(T)*_capacity) );
        if (!data) throw std::logic_error("Couldn't allocate the data for new arrangement.");
    }

    arrangement(size_t capacity, const T& value)
    : data(nullptr), _size(0), _capacity(capacity)
    {
        if (_capacity > SIZE_MAX/sizeof(T)) throw std::logic_error("Arrangement is too big.");
        if (_capacity == 0) return;

        data = static_cast<T*>(std::malloc(sizeof(T)*_capacity));
        if (!data) throw std::bad_alloc();

        try {
            for (; _size<_capacity; _size++)
                new (&data[_size]) T(value);
        }
        catch (...) {
            for (size_t i=0; i<_size; i++)
                data[i].~T();

            std::free(data);
            throw std::logic_error("Couldn't construct item copies.");
        }
    }

    arrangement(const arrangement& other)
    : data(nullptr), _size(0), _capacity(other._capacity)
    {
        if (_capacity == 0) return;

        data = static_cast<T*>( std::malloc(sizeof(T)*_capacity) );
        if (!data) throw std::bad_alloc();

        try {
            for (; _size<other._size; _size++) {
                new (&data[_size]) T(other.data[_size]);
            }
        }
        catch (...) {
            for (size_t i=0; i<_size; i++)
                data[i].~T();

            std::free(data);
            throw std::logic_error("Couldn't reconstruct item copies.");
        }
    }

    ~arrangement() {
        for (size_t i=0; i<_size; i++)
            data[i].~T();

        std::free(data);
    }

    arrangement& operator=(const arrangement& other) {
        if (this == &other) return *this;

        arrangement temp(other);
        std::swap(data, temp.data);
        std::swap(_size, temp._size);
        std::swap(_capacity, temp._capacity);

        return *this;
    }

    class Iterator {
        private:
        arrangement* parent;
        size_t index;
        bool forward;
        public:
        Iterator(arrangement* parent, size_t index=0, bool forward=true)
            : parent(parent), index(index), forward(forward) {}

        T& get() { return parent->data[index]; }
        void set(const T& value) { parent->data[index]=value; }
        void next() { if (forward) {if (hasNext()) index++;} else {if (hasNext()) index--;} }
        bool hasNext() const { if (forward) return (index < parent->_size-1); else return (index > 0); }
    };
    Iterator iterator() {
        return Iterator(this, 0);
    }
    Iterator reverseiterator() {
        return Iterator(this, _size-1, false);
    }

    class ConstIterator {
        private:
        const arrangement* parent;
        size_t index;
        bool forward;
        public:
        ConstIterator(const arrangement* parent, size_t index=0, bool forward=true)
            : parent(parent), index(index), forward(forward) {}

        T& get() { return parent->data[index]; }
        void next() { if (forward) {if (hasNext()) index++;} else {if (hasNext()) index--;} }
        bool hasNext() const { if (forward) return (index < parent->_size-1); else return (index > 0); }
    };
    ConstIterator constiterator() {
        return ConstIterator(this, 0);
    }
    ConstIterator reverseconstiterator() {
        return ConstIterator(this, _size-1, false);
    }


    void grow() {
        size_t new_capacity = (_capacity == 0) ? 1 : _capacity*2;

        if (new_capacity > SIZE_MAX/sizeof(T))
            throw std::logic_error("The arrangement has become too big.");

        T* new_data = static_cast<T*>(std::malloc(sizeof(T)*new_capacity));
        if (!new_data) throw std::bad_alloc();
        if (!data) throw std::logic_error("Couldn't allocate the data for grown arrangement.");

        size_t constructed=0;

        try {
            for (; constructed<_size; ++constructed)
                relocate(new_data, constructed, data[constructed]);
        }
        catch (...) {
            for (size_t i=0; i<constructed; i++)
                new_data[i].~T();

            std::free(new_data);
            throw std::logic_error("Couldn't reconstruct item copies.");
        }

        for (size_t i=0; i<_size; i++)
            data[i].~T();

        std::free(data);

        data = new_data;
        _capacity = new_capacity;
    }

    template<typename... Args>
    void make(size_t index, Args&&... args) {
        new (&data[index]) T(std::forward<Args>(args)...);
    }

    template<typename IT>
    void relocate(T* destination, size_t index, IT& item) {
        if constexpr (std::is_nothrow_move_constructible_v<T>) {
            new (&destination[index]) T(std::move(item));
        }
        else if constexpr (std::is_copy_constructible_v<T>) {
            new (&destination[index]) T(item);
        }
        else {
            throw std::logic_error("I can't safely move nor copy this type.");
        }
    }

    void wipe(size_t index) {
        data[index].~T();
    }

    size_t size() const {
        return _size;
    }

    size_t capacity() const {
        return _capacity;
    }

    T& operator[](size_t index) {
        //if (index >= _size) throw std::out_of_range("Index out of range.");
        return data[index];
    }

    const T& operator[](size_t index) const {
        //if (index >= _size) throw std::out_of_range("Index out of range.");
        return data[index];
    }

    size_t insert(size_t index, T&& item){
        if (index > _size) throw std::out_of_range("Index out of range");
        if (_size == _capacity) grow();

        for (size_t i=_size;i>index;i--){
            relocate(data, i, data[i-1]);
            wipe(i-1);
        }

        relocate(data, index, item);
        _size++;
        return index;
    }

    size_t insert(size_t index, const T& item){
        if (index > _size) throw std::out_of_range("Index out of range");
        if (_size == _capacity) grow();

        for (size_t i=_size;i>index;i--){
            relocate(data, i, data[i-1]);
            wipe(i-1);
        }

        relocate(data, index, item);
        _size++;
        return _size;
    }

    size_t insert(size_t index, const T* item) {
        if (item == nullptr)
            throw std::invalid_argument("Pointer is null.");

        return insert(index, *item);
    }

    void remove(size_t index) {
        if (index >= _size) throw std::out_of_range("Index out of range");

        wipe(index);

        for (size_t i=index; i<_size-1; i++) {
            relocate(data, i, data[i+1]);
            wipe(i+1);
        }

        --_size;
    }

    void remove(){
        remove(_size-1);
    }

    size_t bytes() const {
        return _capacity * sizeof(T);
    }
    size_t deepbytes() const {
        size_t total = _capacity * sizeof(T);
        if constexpr (is_arrangement<T>::value) {
            for (size_t i=0; i<_size; i++) {
                total+=data[i].deepbytes();
            }
        }
        return total;
    }
};

#endif // ARRANGEMENT_H_INCLUDED
