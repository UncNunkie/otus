#include <iostream>
#include <map>
#include "allocator.hpp"
#include "flat_map.hpp"

int main()
{
    std::map<int, int, std::less<>,
             fixed_size_allocator<std::pair<const int, int>, 10>> m;
    for (int i = 0; i < 10; ++i) {
        std::cout << " i = " << i << std::endl;
        m[i] = i;
        std::cout << "m[i] = " << m[i] << " ptr=" << std::hex << uint64_t(&m[i]) << std::dec << std::endl;
    }

    for (auto& [k, v] : m)
    {
        std::cout << "m[" << k << "]=" << v << std::endl;
    }
}