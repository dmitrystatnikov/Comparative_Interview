#ifndef __HASHTABLE_HPP__
#define __HASHTABLE_HPP__

#include <iterator>
#include <exception>
#include <vector>

namespace specialized_datatypes
{

template < typename T, typename HashFunction, typename Predicate = std::equal_to<T>, typename EmptyT = T >
class open_addressing_hash_set
{
    using container_type            = std::vector<T>;
    using self_type                 = open_addressing_hash_set<T, HashFunction, Predicate, EmptyT>;

    using container_iterator        = typename container_type::iterator;
    using const_container_iterator  = typename container_type::const_iterator;


public:

    class table_is_full : public std::exception
    {
    public:
        const char * what () const noexcept override
        {
            return "Table is full";
        }
    };

    class rebalancing_size_too_small : public std::exception
    {
    public:
        const char * what () const noexcept override
        {
            return "Rebalancing size must be bigger then current table size";
        }
    };

    using value_type            = T;
    using empty_type            = EmptyT;
    using pointer               = T *;
    using const_pointer         = T const *;
    using reference             = T &;
    using const_reference       = T const &;
    using hash_function_type    = HashFunction;
    using predicate_type        = Predicate;

    class const_iterator
    {
    public:

        using iterator_category = std::bidirectional_iterator_tag;
        using value_type        = typename const_container_iterator::value_type;
        using difference_type   = typename const_container_iterator::difference_type;
        using pointer           = typename const_container_iterator::pointer;
        using reference         = typename const_container_iterator::reference;

        constexpr pointer operator -> () const noexcept
        {
            return i_iterator.operator-> ();
        }

        constexpr reference operator * () const noexcept
        {
            return *i_iterator;
        }

        constexpr const_iterator & operator++ () noexcept
        {
            while   (   i_iterator < i_hash_table->i_container.end()
                    &&  i_hash_table->predicate()(*(++i_iterator), s_empty_value)
                    );

            return *this;
        }

        constexpr const_iterator operator++ (int) noexcept
        {
            auto current = *this;
            ++(*this);
            return current;
        }

        constexpr const_iterator & operator-- () noexcept
        {
            while   (   i_iterator >= i_hash_table->i_container.begin()
                    &&  i_hash_table->predicate()(*(--i_iterator), s_empty_value)
                    );

            if (i_iterator < i_hash_table->i_container.begin())
            {
                i_iterator = i_hash_table->i_container.end();
            }

            return *this;
        }

        constexpr const_iterator & operator-- (int) noexcept
        {
            auto current = *this;
            --(*this);
            return current;
        }

        constexpr bool operator == (const_iterator const & it) const noexcept
        {
            return i_iterator == it.i_iterator;
        }

        constexpr bool operator != (const_iterator const & it) const noexcept
        {
            return !(*this == it);
        }

    private:
        constexpr const_iterator    (   open_addressing_hash_set const * htable
                                    ,   const_container_iterator it
                                    ) noexcept
        : i_hash_table  (htable)
        , i_iterator    (it)
        {}

        friend open_addressing_hash_set;

    private:
        open_addressing_hash_set const *    i_hash_table;
        const_container_iterator            i_iterator;
    };

    constexpr explicit open_addressing_hash_set (   size_t              reserve_count
                                                ,   hash_function_type  hasher      = hash_function_type()
                                                ,   predicate_type      predicator  = predicate_type()
                                                )
    :   i_container     (reserve_count, s_empty_value)
    ,   i_hash_function (hasher)
    ,   i_predicate     (predicator)
    ,   i_occupancy     ()
    {
    }

    constexpr bool emplace                      (value_type && value)
    {
        if (value == s_empty_value) return false;

        auto it = find_position(value);

        if (it == i_container.end()) throw table_is_full();

        auto result = false;

        if (predicate()(*it, s_empty_value))
        {
            *it = std::move(value);
            ++i_occupancy;
            result = true;
        }

        return result;

    }

    template <typename... Args>
    constexpr bool emplace                      (Args &&... args)
    {
        return emplace(value_type(args...));
    }

    constexpr size_t erase                      (const_reference value)
    {
        auto it = find_position(value);
        size_t result = 0;

        if (!predicate()(*it, s_empty_value))
        {
            *it = s_empty_value;
            --i_occupancy;
            result = 1;
        }

        return result;
    }

    template<typename HasherT>
    constexpr void rebalance                    (   size_t reserve_count
                                                ,   HasherT && rebalance_hasher
                                                )
    {
        hash_function_type current_hasher{std::move(i_hash_function)};
        i_hash_function = std::forward<HasherT>(rebalance_hasher);

        try
        {
            rebalance (reserve_count);
        }
        catch(rebalancing_size_too_small& e)
        {
            i_hash_function = std::move(current_hasher);
            throw;
        }
        
    }

    constexpr void rebalance                    (size_t reserve_count)
    {        
        if (reserve_count < size()) throw rebalancing_size_too_small();

        container_type original (reserve_count, s_empty_value);
        std::swap(i_container, original);

        for (auto & value : original)
        {
            if (!predicate()(value, s_empty_value))
            {
                emplace(std::move(value));
                value = s_empty_value;
            }
        }
    }

    [[nodiscard]]
    constexpr const_iterator find               (const_reference value) const
    {
        auto it = find_position(value);

        if (it != i_container.end() && predicate()(*it, s_empty_value))
        {
            it = i_container.end();
        }

        return const_iterator(this, it);        
    }

    [[nodiscard]]
    constexpr size_t capacity                   () const noexcept
    {
        return i_container.size();
    }

    [[nodiscard]]
    constexpr size_t size                       () const noexcept
    {
        return i_occupancy;
    }

    [[nodiscard]]
    constexpr bool is_empty                     () const noexcept
    {
        return i_occupancy == 0;
    }

    [[nodiscard]]
    constexpr hash_function_type const & hasher () const noexcept
    {
        return i_hash_function;
    }

    [[nodiscard]]
    constexpr predicate_type const & predicate  () const noexcept
    {
        return i_predicate;
    }

    [[nodiscard]]
    constexpr static empty_type empty_value     () noexcept
    {
        return s_empty_value;
    }

    [[nodiscard]]
    constexpr const_iterator begin              () const noexcept
    {
        const_iterator it(this, i_container.begin());
        if (!i_container.empty() && predicate()(s_empty_value, *it)) ++it;

        return it;
    }

    [[nodiscard]]
    constexpr const_iterator cbegin             () const noexcept
    {
        return begin();
    }

    [[nodiscard]]
    constexpr const_iterator end                () const noexcept
    {
        return const_iterator(this, i_container.end());
    }

    [[nodiscard]]
    constexpr const_iterator cend               () const noexcept
    {
        return end();
    }

private:

    [[nodiscard]]
    constexpr const_container_iterator find_position    (const_reference value) const
    {
        if (predicate()(value, s_empty_value)) return i_container.end();

        auto const first                = i_container.begin();
        auto const expected_position    = hasher()(value);
        auto it                         = std::next(first, expected_position);

        auto const count_limit          = i_container.size();
        size_t steps                    = 0;
        while   (   steps < count_limit
                &&  !predicate()(*it, s_empty_value)
                &&  !predicate()(*it, value)
                )
        {
            ++steps;
            it = std::next(first, (expected_position + steps) % count_limit);
        }

        if (steps == count_limit) it = i_container.end();

        return it;
    }

    [[nodiscard]]
    constexpr container_iterator find_position          (const_reference value)
    {
        auto it = const_cast<const self_type *>(this)->find_position(value);
        return container_iterator(std::next(i_container.begin(), std::distance(i_container.cbegin(), it)));
    }

    container_type      i_container;
    hash_function_type  i_hash_function;
    predicate_type      i_predicate;

    size_t              i_occupancy;

    constexpr static empty_type s_empty_value {};
};

}

#endif // __HASHTABLE_HPP__