#include "core/utils.h"
#include "core/exception.h"
#include <algorithm>
#include <iostream>
#include <cmath>

namespace ltsy::utils {

    std::vector<int> tuple_from_position(int nvalues, int arity, int position) {
        if (position < 0 or position > int(std::pow(nvalues, arity)))
            throw std::invalid_argument(ltsy::WRONG_TUPLE_POSITION_EXCEPTION);
        auto i = arity - 1;
        std::vector<int> tuple (arity, 0);
        while (i >= 0) {
            auto power = int(std::pow(nvalues, i));
            tuple[arity - i - 1] = position / power;
            position %= power;
            i -= 1;
        }
        return tuple;
    }

    int position_from_tuple(int nvalues, int arity, const std::vector<int>& tuple) {
        int i = 0;
        int position = 0;
        while (i < arity) {
            auto power = int(std::pow(nvalues, arity - i - 1));
            position += tuple[i] * power;
            ++i;
        }
        return position;
    }

    int compute_number_of_functions(int nvalues, int arity) {
        return std::pow(nvalues, compute_number_of_rows(nvalues, arity)); 
    }

    int compute_number_of_rows(int nvalues, int arity) { 
        return int(std::pow(nvalues, arity)); 
    };

    template<typename T>
    bool is_subset(const std::unordered_set<T>& s1, const std::unordered_set<T>& s2){
        if (s1.size() > s2.size())
            return false;
        for (auto& e : s1) 
            if (s2.find(e) == s2.end()) return false;
        return true;
    }

    template bool is_subset<int>(const std::unordered_set<int>& s1, const std::unordered_set<int>& s2);

    template<typename T>
    bool is_subset(const std::set<T>& s1, const std::set<T>& s2){
        return std::includes(s1.begin(), s1.end(), s2.begin(), s2.end());
    }

    template bool is_subset<int>(const std::set<int>& s1, const std::set<int>& s2);

    template<typename T>
    std::unordered_set<T> set_difference(const std::unordered_set<T>& s1, const std::unordered_set<T>& s2){
        std::unordered_set<T> result;
        std::copy_if(s1.begin(), s1.end(), std::inserter(result, result.end()),
            [&s2] (int needle) { return s2.find(needle) == s2.end(); });
        return result;
    }

    template std::unordered_set<int> set_difference<int>(const std::unordered_set<int>& s1, const std::unordered_set<int>& s2);

    template<typename T>
    std::set<T> set_difference(const std::set<T>& s1, const std::set<T>& s2){
        std::set<T> result;
        std::set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(), 
                std::inserter(result, result.begin()));
        return result;
    }

    template std::set<int> set_difference<int>(const std::set<int>& s1, const std::set<int>& s2);
};
