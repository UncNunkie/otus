#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdarg>
#include <algorithm>
#include <span>
#include <tuple>

std::vector<std::string> split(const std::string &str, char d = '.')
{
    std::vector<std::string> r;

    std::string::size_type start = 0;
    std::string::size_type stop = str.find_first_of(d);
    while(stop != std::string::npos)
    {
        r.push_back(str.substr(start, stop - start));

        start = stop + 1;
        stop = str.find_first_of(d, start);
    }

    r.push_back(str.substr(start));

    return r;
}

template<typename Iterable>
void print(const Iterable& ip_pool)
{
    for(auto ip = ip_pool.begin(); ip != ip_pool.end(); ++ip)
    {
        for(auto ip_part = ip->begin(); ip_part != ip->end(); ++ip_part)
        {
            if (ip_part != ip->begin())
            {
                std::cout << ".";
            }
            std::cout << static_cast<int>(*ip_part);
        }
        std::cout << std::endl;
    } 
}

std::span<const std::vector<uint8_t>> filter(std::vector<std::vector<uint8_t>>& ips, int count, ...)
{
    assert(count != 0);

    va_list args;
    uint8_t args_idx = 0;
    auto curr_end = ips.end();
    va_start(args, count);

    while (count--)
    {
      curr_end = std::stable_partition(ips.begin(), curr_end,
            [filter_val = va_arg(args, int), args_idx](const std::vector<uint8_t>& ip)
            {
                return ip[args_idx] == filter_val;
            }
        );

        args_idx++;
    }

    va_end(args);
    return {ips.begin(), curr_end};
}

std::span<std::vector<uint8_t>> filter_any(std::vector<std::vector<uint8_t>>& ips, int filter_val)
{
    auto curr_end = ips.end();
    
    curr_end = std::stable_partition(ips.begin(), curr_end,
        [filter_val](const std::vector<uint8_t>& ip)
        {
    	    return std::any_of(ip.begin(), ip.end(),
                [filter_val](const auto& ip_part)
                {
                        return ip_part == filter_val;
                }
    	    );
        }
    );

    return {ips.begin(), curr_end};
}

int main()
{
    try
    {
        std::vector<std::vector<uint8_t>> ip_pool;
        std::vector<std::vector<std::string>> ip_pool_s;

        for(std::string line; std::getline(std::cin, line);)
        {
            auto v = split(line, '\t');
            auto vs = split(v.at(0));
            ip_pool_s.push_back(vs);
        }

        std::sort(ip_pool_s.begin(), ip_pool_s.end(), [](const auto& lhs, const auto& rhs) {
            for (uint8_t cmp_idx = 0; cmp_idx < 4; ++cmp_idx)
            {
                if (lhs[cmp_idx] == rhs[cmp_idx])
                {
                    continue;
                }
                else
                {
                  return lhs[cmp_idx] > rhs[cmp_idx];
                }
            }
            return false;
        });

        // TODO reverse lexicographically sort

        std::for_each(ip_pool_s.begin(), ip_pool_s.end(),
            [&ip_pool](const auto& ip){
                std::vector<uint8_t> ip_i;
                for (const auto& ip_part : ip)
                {
                    ip_i.push_back(std::atoi(ip_part.c_str()));
                }
                ip_pool.push_back(ip_i);
            }
        );
        print(ip_pool);
        // 222.173.235.246
        // 222.130.177.64
        // 222.82.198.61
        // ...
        // 1.70.44.170
        // 1.29.168.152
        // 1.1.234.8

        // TODO filter by first byte and output
        auto ip = filter(ip_pool, 1, 1);
        print(ip);
        // 1.231.69.33
        // 1.87.203.225
        // 1.70.44.170
        // 1.29.168.152
        // 1.1.234.8

        // TODO filter by first and second bytes and output

        ip = filter(ip_pool, 2, 46, 70);
        print(ip);

        // 46.70.225.39
        // 46.70.147.26
        // 46.70.113.73
        // 46.70.29.76


        ip = filter(ip_pool, 3, 185, 46, 86);
        print(ip);

        // TODO filter by any byte and output
        // ip = filter_any(46)

        ip = filter_any(ip_pool, 46);
        print(ip);
        // 186.204.34.46
        // 186.46.222.194
        // 185.46.87.231
        // 185.46.86.132
        // 185.46.86.131
        // 185.46.86.131
        // 185.46.86.22
        // 185.46.85.204
        // 185.46.85.78
        // 68.46.218.208
        // 46.251.197.23
        // 46.223.254.56
        // 46.223.254.56
        // 46.182.19.219
        // 46.161.63.66
        // 46.161.61.51
        // 46.161.60.92
        // 46.161.60.35
        // 46.161.58.202
        // 46.161.56.241
        // 46.161.56.203
        // 46.161.56.174
        // 46.161.56.106
        // 46.161.56.106
        // 46.101.163.119
        // 46.101.127.145
        // 46.70.225.39
        // 46.70.147.26
        // 46.70.113.73
        // 46.70.29.76
        // 46.55.46.98
        // 46.49.43.85
        // 39.46.86.85
        // 5.189.203.46
    }
    catch(const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
