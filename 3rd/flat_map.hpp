#include <cstddef>
#include <cstring>
#include <utility>
#include <iostream>
#include <memory>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/iterator_categories.hpp>

// always wanted to write flat_map
// i think flat map should not rely on having same ptr
// for every <k,v> after inserting, like vector which can realloc etc

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

      auto tmp = std::make_unique<T[]>(n);
      std::cout << "curr alloced sz = "
                << n << "  szof = " << sizeof(T) << std::endl;
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
      // leak some data hope allocator will call deallocate
      data_.release();
      data_ = std::move(tmp);

      pointer ret = &data_[curr_idx_];
      curr_idx_ += n;
      return ret;
    }

    void deallocate(pointer p, size_t n)
    {
        delete [] p;
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

template<typename K, typename V, typename Alloc>
class flat_map
{
public:
  using mapped_type = V;
  using key_type = K;
  using value_type = std::pair<const K, V>;

  struct iterator : boost::iterator_facade<
                          iterator
                        , std::pair<K, V>
                        , boost::forward_traversal_tag
                    >
  {
  public:
      using Value = std::pair<K, V>;

      iterator()
        : node_(0) {}

      explicit iterator(Value* p)
        : node_(p) {}

      template <class OtherValue>
      iterator(iterator const& other)
        : node_(other.node_) {}

  private:
      friend class boost::iterator_core_access;
      friend class iterator;

      template <class OtherValue>
      bool equal(iterator const& other) const
      {
        return this->node_ == other.node_;
      }

      void increment()
      { ++node_; }

      Value& dereference() const
      { return *node_; }

      Value* node_;
  };

  flat_map() {}
  iterator begin() const { return val_; }
  iterator end() const { return {}; }
  size_t size() const { return size_; }

  V& operator[](const K& key)
  {
      auto it = std::lower_bound(val_, val_ + size_, key,
        [](const typename iterator::Value& v, const K& k)
        {
            return v.first < k;
        }
      );
      auto pos = std::distance(val_, it);

      check_n_fix_capacity();

      if (pos == size_) // new element at the end
      {
          auto ret_ptr = val_ + size_;
          size_++;
          new (ret_ptr) value_type(key, {});
          return ret_ptr->second;
      } else {
          new (val_ + size_) value_type (key, {});
          return (val_ + size_)->second;
      }


      } else {

      }
  }

private:

  void check_n_fix_capacity()
  {
      if (size_ + 1 >= capacity_)
      {
          auto ptr = alloc_.allocate(capacity_ * 2);
          re_store_data(ptr);
          alloc_.deallocate(val_, size_);
          capacity_ *= 2;
          val_ = ptr;
      }
  }

  void re_store_data()
  {
      if constexpr (std::is_nothrow_move_constructible_v<value_type>
          && std::is_trivially_move_constructible_v<value_type>)
      {
          std::cout << "memmove" << std::endl;
          memmove(tmp.get(), data_.get(), sizeof(T) * curr_idx_);
      } else if constexpr (std::is_nothrow_move_constructible_v<value_type>) {
          std::cout << "move" << std::endl;
          std::move(data_, data_ + curr_idx_, tmp);
      } else {
          std::cout << "copy" << std::endl;
          std::copy(data_, data_ + curr_idx_, tmp);
      }
  }

  value_type* val_ = nullptr;
  size_t size_ = 0;
  size_t capacity_ = 0;
  Alloc alloc_;
};