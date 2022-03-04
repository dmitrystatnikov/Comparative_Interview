#ifndef __TESTHASHTABLE_HPP__
#define __TESTHASHTABLE_HPP__

#include <cstddef>

namespace unit_test
{

struct simple_size_hasher
{
public:

    constexpr explicit simple_size_hasher (size_t size)
    : i_size (size)
    {
    }

    template<typename T>
    [[nodiscard]]
    constexpr size_t operator ()(T value) const noexcept
    {
        return static_cast<size_t>(value) % i_size;
    }

    [[nodiscard]]
    constexpr size_t size() const noexcept
    {
        return i_size;
    }

private:
    size_t i_size;
};

}

#endif // __TESTHASHTABLE_HPP__