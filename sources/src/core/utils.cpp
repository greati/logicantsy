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
    std::vector<std::vector<T>> cartesian_product(const std::set<T>& s1, const std::set<T>& s2){
        std::vector<std::vector<T>> result;
        for (auto it1 = s1.begin(); it1 != s1.end(); ++it1) {
            for (auto it2 = s2.begin(); it2 != s2.end(); ++it2) {
                result.push_back({*it1, *it2});
            } 
        }
        return result; 
    }
    template std::vector<std::vector<int>> cartesian_product(const std::set<int>& s1, const std::set<int>& s2);

    template<typename T>
    std::vector<std::vector<T>> cartesian_product(const std::vector<std::vector<T>>& s1, const std::set<T>& s2){
        std::vector<std::vector<T>> result;
        for (auto it1 = s1.begin(); it1 != s1.end(); ++it1) {
            for (auto it2 = s2.begin(); it2 != s2.end(); ++it2) {
                auto v = *it1;
                v.push_back(*it2);
                result.push_back(v);
            } 
        }
        return result; 
    }
    template std::vector<std::vector<int>> cartesian_product(const std::vector<std::vector<int>>& s1, const std::set<int>& s2);

    template<typename T>
    std::vector<std::vector<T>> cartesian_product(const std::vector<std::set<T>>& sets){
        std::vector<std::vector<T>> result;
        auto sets_amount = sets.size();
        if (sets_amount == 1) {
            for (auto s : sets[0]) result.push_back({s});
        } else if (sets_amount >= 2) {
            result = cartesian_product(sets[0], sets[1]);
            for (int i = 2; i < sets_amount; ++i)
                result = cartesian_product(result, sets[i]);
        }
        return result;
    }
    template std::vector<std::vector<int>> cartesian_product(const std::vector<std::set<int>>& sets);

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
        return std::includes(s2.begin(), s2.end(), s1.begin(), s1.end());
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
