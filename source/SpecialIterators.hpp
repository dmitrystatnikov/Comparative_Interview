#ifndef __SPECIALITERATORS_HPP__
#define __SPECIALITERATORS_HPP__

#include <iterator>

namespace utilities
{

class counting_iterator
{
public:

    using category_type     = std::random_access_iterator_tag;
    using value_type        = uint32_t;
    using difference_type   = uint32_t;

    constexpr counting_iterator(uint32_t position = 0) noexcept
    : i_position(position)
    {
    }

    constexpr counting_iterator & operator += (uint32_t offset) noexcept
    {
        i_position += offset;
        return *this;
    }

    constexpr counting_iterator & operator -= (uint32_t offset) noexcept
    {
        i_position -= offset;
        return *this;
    }

    constexpr counting_iterator & operator++ () noexcept
    {
        return *(this) += 1;
    }

    constexpr counting_iterator operator++ (int) noexcept
    {
        auto return_value = *this;
        *(this) += 1;
        return return_value;
    }

    constexpr counting_iterator & operator-- () noexcept
    {
        return *(this) -= 1;
    }

    constexpr counting_iterator operator-- (int) noexcept
    {
        auto return_value = *this;
        *(this) -= 1;
        return return_value;
    }

    constexpr counting_iterator::difference_type operator - (counting_iterator it) noexcept
    {
        return i_position - it.i_position;
    }


    constexpr bool operator== (counting_iterator it) const noexcept
    {
        return i_position == it.i_position;
    }

    constexpr bool operator!= (counting_iterator it) const noexcept
    {
        return !(*this == it);
    }

    constexpr bool operator < (counting_iterator it) const noexcept
    {
        return i_position < it.i_position;
    }

    constexpr bool operator > (counting_iterator it) const noexcept
    {
        return it < *this;
    }

    constexpr bool operator <= (counting_iterator it) const noexcept
    {
        return !(*this > it);
    }

    constexpr bool operator >= (counting_iterator it) const noexcept
    {
        return !(*this < it);
    }

    constexpr value_type operator * () const noexcept
    {
        return i_position;
    }

private:

    uint32_t i_position;
};

constexpr counting_iterator operator + (counting_iterator it1, counting_iterator::difference_type offset) noexcept
{
    return it1 += offset;
}

constexpr counting_iterator operator - (counting_iterator it1, counting_iterator::difference_type offset) noexcept
{
    return it1 -= offset;
}

}

#endif // __SPECIALITERATORS_HPP__