#include <memory>
#include <cstddef>
#include <initializer_list>
#include <utility>
#include <limits>
 

namespace getcracked {
    template <typename T, typename Allocator = std::allocator<T>>
    class vector {
    public:
        vector() : vector(1){ } //default constructor
        vector(size_t capacity) { //constructor
            reserve(capacity);
        }

        vector(const vector& other) { //copy constructor
            copy_from(other);
        }

        vector& operator=(const vector& other) { //copy assignment
            if(this == &other) 
                return *this;

            copy_from(other);
            return *this;    
        } 

        //move semantics
        vector(vector&& other) {
            steal_from(other);
        }

        vector& operator=(vector&& other) {
            steal_from(other);
        }

        ~vector() {clear(); }

        bool empty() const {return size_ == 0; }

        T& at(size_t index) {
            if(index >= size_) {
                throw std::out_of_range("Index is too large");
            }

            return data_[index];
        }

        void shrink_to_fit() {
            reserve (size_);
        }

        size_t get_size() const { return size_; }

        size_t get_capacity() const {return capacity_; }


        //when you call reserve you allocate a new bigger block of memory
        void reserve(size_t capacity) {
            if(capacity == 0)
                return;

            Allocator allocator;
            T* memory = allocator.allocate(capacity);

            if(data_ != nullptr) {
                for(size_t i{}; i < size_; ++i) {
                    new(memory + i) T{data_[i]};
                }

                for (size_t i{}; i < size_; ++i)
                {
                    data_[i].~T();
                }
    
                allocator.deallocate(data_, capacity_);
            }
            data_ = memory;
            capacity_ = capacity;

        }

        void resize(size_t size) {
            resize(size, T{});
        }

        void push_back(const T& item) {
            try_expand_capacity();
            new(data_ + size_++) T{item}; 
        }

        template <typename... Args>
        void emplace_back(Args&&... args)
        {
            try_expand_capacity();
    
            new (data_ + size_++) T(std::forward<Args>(args)...);
        }

        void pop_back() {
            data_[--size_].~T();
        }

        void clear()
        {
            while (size_)
            {
                pop_back();
            }
    
            std::allocator<T> allocator;
            allocator.deallocate(data_, capacity_);
        }


    private:
        void steal_from(vector&& other) {
            data_ = std::exchange(other.data_, nullptr);
            capacity_ = other.capacity_;
            size_ = other.size_;
        }

        void copy_from(const vector& other) {
            Allocator allocator;
            T* memory = allocator.allocate(other.capacity_);
            for(size_t i {}; i < other.size_; i++) {
                new(memory) T {other.data_[i]};
            }

            if(data_ != nullptr) {
                for(size_t i {}; i < size_; i++) {
                    data_[i].~T();
                }
                allocator.deallocate(data_, capacity_);
            }

            data_ = memory;
            capacity_ = other.capacity_;
            size_ = other.size_;
        }

         void resize(size_t size, const T& item)
        {
            if (size_ > size)
                return;
    
            reserve(size);
    
            for (size_t i{ size_ }; i < size; ++i)
            {
                new (data_ + i) T{ item };
            }
    
            size_ = size;
        }

        void try_expand_capacity()
        {
            if (!should_increase_capacity())
                return;
    
            auto capacity = capacity_ == 0 ? 1 : capacity_;
            const auto maximum_capacity = std::numeric_limits<size_t>::max();
    
            if (maximum_capacity / capacity_increase_factor() < capacity)
                capacity = maximum_capacity;
    
            reserve(capacity * capacity_increase_factor());
        }

        bool should_increase_capacity() const
        {
            return capacity_ < std::numeric_limits<size_t>::max() / capacity_increase_factor() &&
                capacity_ / capacity_increase_factor() <= size_;
        }
    
        size_t capacity_increase_factor() const { return 3; }
    
        size_t size_{};
        size_t capacity_{};
        T* data_{ nullptr };

    };
}