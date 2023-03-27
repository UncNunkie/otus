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
class custom_allocator_2 {
public:
    using value_type = T;

    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

    template<typename U>
    struct rebind
    {
        using other = custom_allocator_2<U>;
    };

    custom_allocator_2()
    {}

    ~custom_allocator_2() = default;

    template<typename U>
    custom_allocator_2(const custom_allocator_2<U>&)
    {}

    pointer allocate(size_t n)
    {
        if (n == 0)
        {
            return nullptr;
        }

        std::cout << "curr alloced sz = "
                  << n << "  szof = " << sizeof(T) << std::endl;

        // leak some data hope container will call deallocate
        data_.release();
        data_ = std::make_unique<T[]>(n);

        return data_.get();
    }

    void deallocate(pointer p, size_t /*n*/)
    {
        delete [] p;
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
};

template<typename K, typename V, typename Alloc>
class flat_map
{
public:
    using mapped_type = V;
    using key_type = K;
    using value_type = std::pair</*const*/ K, V>;

// TODO FIXME maybe later
//  struct iterator : boost::iterator_facade<
//                          iterator
//                        , std::pair<K, V>
//                        , boost::forward_traversal_tag
//                    >
//  {
//  public:
//      using value = std::pair<K, V>;
//
//      iterator()
//          : node_(0)
//      {}
//
//      explicit iterator(value* p)
//          : node_(p)
//      {}
//
//      template <class Othervalue>
//      iterator(iterator const& other)
//          : node_(other.node_)
//      {}
//
//  private:
//      friend class boost::iterator_core_access;
//      friend class flat_map;
//
//      template <typename Other>
//      bool equal(iterator const& other) const
//      {
//          return this->node_ == other.node_;
//      }
//
//      void increment() { ++node_; }
//
//      value& dereference() const { return *node_; }
//
//      value* node_;
//  };

    flat_map() {}
//  iterator begin() const { return val_; }
//  iterator end() const { return {}; }
    size_t size() const { return size_; }

    V& operator[](const K& key)
    {
        auto it = std::lower_bound(val_, val_ + size_, key,
            [](const value_type& v, const K& k)
            {
                return v.first < k;
            }
        );

        size_t pos = std::distance(val_, it);

        if (it != val_ + size_)
        {
            if (it->first != key)
            {
                check_n_fix_capacity();
            } else {
                return it->second;
            }
        } else {
            check_n_fix_capacity();
        }

        value_type* ret_ptr = nullptr;
        if (pos == size_) // new element at the end
        {
            ret_ptr = val_ + size_;
            size_++;
        } else {
            ret_ptr = val_ + pos;
            // TODO use same func as re_store_data() method
            if constexpr (std::is_nothrow_move_constructible_v<value_type>
                && std::is_trivially_move_constructible_v<value_type>
                && std::is_trivially_copy_assignable_v<value_type>)
            {
                std::cout << "IN memmove" << std::endl;
                memmove(val_ + pos + 1, val_ + pos, sizeof(value_type) * (size_ - pos));
            } else if constexpr (std::is_nothrow_move_constructible_v<value_type>
                && std::is_move_assignable_v<value_type>)
            {
                std::cout << "IN move" << std::endl;
                std::move(val_ + pos, val_ + size_, val_ + pos + 1);
            } else {
                std::cout << "IN copy" << std::endl;
                std::copy(val_ + pos, val_ + size_, val_ + pos + 1);
            }
            return (val_ + size_)->second;
        }
        new (ret_ptr) value_type(key, {});
        return ret_ptr->second;
    }

private:

    void check_n_fix_capacity()
    {
        if (size_ + 1 >= capacity_)
        {
            auto ptr = alloc_.allocate((size_ + 1) * 2);
            re_store_data(ptr);
            alloc_.deallocate(val_, size_);
            capacity_ = (size_ + 1) * 2;
            val_ = ptr;
        }
    }

    void re_store_data(typename Alloc::pointer p)
    {
        if constexpr (std::is_nothrow_move_constructible_v<value_type>
            && std::is_trivially_move_constructible_v<value_type>
            && std::is_trivially_copy_assignable_v<value_type>)
        {
            std::cout << "RE memmove" << std::endl;
            memmove(p, val_, sizeof(value_type) * size_);
        } else if constexpr (std::is_nothrow_move_constructible_v<value_type>
            && std::is_move_assignable_v<value_type>)
        {
            std::cout << "RE move" << std::endl;
            std::move(val_, val_ + size_, p);
        } else {
            std::cout << "RE copy" << std::endl;
            std::copy(val_, val_ + size_, p);
        }
    }

    value_type* val_ = nullptr;
    size_t size_ = 0;
    size_t capacity_ = 0;
    Alloc alloc_;
};