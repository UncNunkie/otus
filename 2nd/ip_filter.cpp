#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

// ("",  '.') -> [""]
// ("11", '.') -> ["11"]
// ("..", '.') -> ["", "", ""]
// ("11.", '.') -> ["11", ""]
// (".11", '.') -> ["", "11"]
// ("11.22", '.') -> ["11", "22"]
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

void print(const std::vector<std::vector<std::string>>& ip_pool) {

    for(auto ip = ip_pool.cbegin(); ip != ip_pool.cend(); ++ip)
    {
        for(auto ip_part = ip->cbegin(); ip_part != ip->cend(); ++ip_part)
        {
            if (ip_part != ip->cbegin())
            {
                std::cout << ".";

            }
            std::cout << *ip_part;
        }
        std::cout << std::endl;
    } 
}

std::vector<std::vector<std::string>> filter(std::vector<std::vector<std::string>> ips, int count, ...) {
    assert(count != 0);

    va_list args;
    uint8_t args_idx = 0;
    auto curr_end = ips.end(); 
    va_start(args, count);

    while (count--) {
        curr_end = std::remove_if(ips.begin(), curr_end, 
            [filter_val = va_arg(args, int), args_idx](const std::vector<std::string>& ip)
            {
                return ip[args_idx] != std::to_string(filter_val);
            }
        );

        args_idx++;
    }

    va_end(args);
    ips.erase(curr_end, ips.end());

    return ips;
}


std::vector<std::vector<std::string>> filter_any(std::vector<std::vector<std::string>> ips, int filter_val) {
    auto curr_end = ips.end();
    
    curr_end = std::remove_if(ips.begin(), curr_end, 
        [filter_val](const std::vector<std::string>& ip)
        {
            bool has_filter = false;
            for (const auto& val : ip) {
                if (val == std::to_string(filter_val)) {
                    has_filter = true;
                }
            }
            return not has_filter;
        }
    );

    ips.erase(curr_end, ips.end());
    return ips;
}

int main(/*int argc, char const *argv[]*/)
{
    try
    {
        std::vector<std::vector<std::string>> ip_pool;

        for(std::string line; std::getline(std::cin, line);)
        {
            std::vector<std::string> v = split(line, '\t');
            ip_pool.push_back(split(v.at(0)));
        }

        std::sort(ip_pool.begin(), ip_pool.end(), [](const auto& lhs, const auto& rhs){
            for (uint8_t cmp_idx = 0; cmp_idx < 4; ++cmp_idx) {
                auto lhs_part = std::atoi(lhs[cmp_idx].c_str());
                auto rhs_part = std::atoi(rhs[cmp_idx].c_str());
                if (lhs_part == rhs_part)
                {
                    continue;
                } else {
                    return lhs_part > rhs_part;
                }
            }
            return false;
        });

        // TODO reverse lexicographically sort

        print(ip_pool);
        // 222.173.235.246
        // 222.130.177.64
        // 222.82.198.61
        // ...
        // 1.70.44.170
        // 1.29.168.152
        // 1.1.234.8

        // TODO filter by first byte and output
        std::cout << "\nafter filt!!! 1" << std::endl; 

        auto ip = filter(ip_pool, 1, 1);
        print(ip);

        // 1.231.69.33
        // 1.87.203.225
        // 1.70.44.170
        // 1.29.168.152
        // 1.1.234.8

        // TODO filter by first and second bytes and output
        std::cout << "\nafter filt!!! 46, 70" << std::endl; 

        ip = filter(ip_pool, 2, 46, 70);
        print(ip);

        // 46.70.225.39
        // 46.70.147.26
        // 46.70.113.73
        // 46.70.29.76

        std::cout << "\nafter filt!!! 185, 46, 86" << std::endl; 

        ip = filter(ip_pool, 3, 185, 46, 86);
        print(ip);

        // TODO filter by any byte and output
        // ip = filter_any(46)


        std::cout << "\nafter filt_any!!! 46" << std::endl; 

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
