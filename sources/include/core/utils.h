#ifndef __CORE_UTILS__
#define __CORE_UTILS__

#include <array>
#include <vector>
#include <unordered_set>
#include <set>

namespace ltsy::utils {

    template<typename T>
    struct DeepPointerComp {
        bool operator()(const T* lhs, const T* rhs) const {
            return *lhs < *rhs; 
        }
    };

    std::vector<int> tuple_from_position(int nvalues, int arity, int position);

    int position_from_tuple(int nvalues, int arity, const std::vector<int>& tuple);

    int compute_number_of_functions(int nvalues, int arity);

    int compute_number_of_rows(int nvalues, int arity);

    template<typename T>
    bool is_subset(const std::unordered_set<T>& s1, const std::unordered_set<T>& s2);
    extern template bool is_subset<int>(const std::unordered_set<int>& s1, const std::unordered_set<int>& s2);

    template<typename T>
    bool is_subset(const std::set<T>& s1, const std::set<T>& s2);
    extern template bool is_subset<int>(const std::set<int>& s1, const std::set<int>& s2);

    template<typename T>
    std::unordered_set<T> set_difference(const std::unordered_set<T>& s1, const std::unordered_set<T>& s2);
    extern template std::unordered_set<int> set_difference<int>(const std::unordered_set<int>& s1, const std::unordered_set<int>& s2);

    template<typename T>
    std::set<T> set_difference(const std::set<T>& s1, const std::set<T>& s2);
    extern template std::set<int> set_difference<int>(const std::set<int>& s1, const std::set<int>& s2);

};

#endif
