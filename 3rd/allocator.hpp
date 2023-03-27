#include <array>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <new>
#include <type_traits>
#include <memory>
#include <string.h>
#include <iostream>


template<typename T>
class custom_allocator {
public:
    using value_type = T;

    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

    template<typename U>
    struct rebind
    {
      using other = custom_allocator<U>;
    };

    custom_allocator()
        : allocated_(0)
        , curr_idx_(0)
    {}

    ~custom_allocator() = default;

    template<typename U>
    custom_allocator(const custom_allocator<U>&)
    {}

    pointer allocate(size_t n)
    {
      if (n == 0)
      {
          return nullptr;
      }

      if (curr_idx_ + n > allocated_)
      {
          // need to allocate more memory
          auto to_allocate = (curr_idx_ + n) * 2; // why not?
          auto tmp = std::make_unique<T[]>(to_allocate);
          std::cout << "curr alloced sz = " << to_allocate << "  szof = " << sizeof(T) << std::endl;
          if constexpr (std::is_nothrow_move_constructible_v<T>
              && std::is_trivially_move_constructible_v<T>)
          {
              std::cout << "memmove" << std::endl;
              memmove(tmp.get(), data_.get(), sizeof(T) * curr_idx_);
          } else if constexpr (std::is_nothrow_move_constructible_v<T>) {
              std::cout << "move" << std::endl;
              std::move(data_, data_ + curr_idx_, tmp);
          } else {
              std::cout << "copy" << std::endl;
              std::copy(data_, data_ + curr_idx_, tmp);
          }
          allocated_ = to_allocate;
          data_ = std::move(tmp);
      }

      pointer ret = &data_[curr_idx_];
      curr_idx_ += n;
      return ret;
    }

    void deallocate(pointer, size_t n)
    {
        curr_idx_ -= n;
    }

    template<typename U, typename ...Args>
    void construct(U *p, Args &&...args)
    {
        new (p) U(std::forward<Args>(args)...);
    }

    void destroy(pointer p)
    {
        p->~T();
    }
private:
    std::unique_ptr<T[]> data_;
    size_t curr_idx_ = 0;
    size_t allocated_ = 0;
};


template<typename T, size_t N>
class fixed_size_allocator {
public:
    using value_type = T;

    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

    template<typename U>
    struct rebind
    {
        using other = fixed_size_allocator<U, N>;
    };

    fixed_size_allocator()
        : curr_idx_(0)
    {}

    ~fixed_size_allocator() = default;

    template<typename U>
    fixed_size_allocator(const fixed_size_allocator<U, N>&)
    {}

    pointer allocate(size_t n)
    {
        if (n == 0)
        {
            return nullptr;
        }

        if (curr_idx_ + n > N)
        {
            throw std::bad_alloc();
        }

        pointer ret = &data_[curr_idx_];
        curr_idx_ += n;
        return ret;
    }

    void deallocate(pointer, size_t n)
    {
        curr_idx_ -= n;
    }

    template<typename U, typename ...Args>
    void construct(U *p, Args &&...args)
    {
        new (p) U(std::forward<Args>(args)...);
    }

    void destroy(pointer p)
    {
        p->~T();
    }
private:
    std::array<T, N> data_;
    size_t curr_idx_ = 0;
};