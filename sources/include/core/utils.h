#ifndef __CORE_UTILS__
#define __CORE_UTILS__

#include <array>
#include <vector>
#include <unordered_set>
#include <memory>
#include <set>

namespace ltsy::utils {

    template<typename T>
    struct DeepPointerComp {
        bool operator()(const T* lhs, const T* rhs) const {
            return *lhs < *rhs; 
        }
    };

    template<typename T>
    struct DeepSharedPointerComp {
        bool operator()(const std::shared_ptr<T> lhs, const std::shared_ptr<T> rhs) const {
            return *lhs < *rhs; 
        }
    };


    /* Compare two sets of pointers for equality.
     * */
    template<typename T>
    bool is_subset(const std::set<std::shared_ptr<T>, DeepSharedPointerComp<T>>& s1,
                const std::set<std::shared_ptr<T>, DeepSharedPointerComp<T>>& s2) {
        if (s1.size() > s2.size()) return false;
        for (const auto& s : s1)
            if (s2.find(s) == s2.end())
                return false;
        return true;
    }

    /* Compare two sets of pointers for equality.
     * */
    template<typename T>
    bool equals(const std::set<std::shared_ptr<T>, DeepSharedPointerComp<T>>& s1,
                const std::set<std::shared_ptr<T>, DeepSharedPointerComp<T>>& s2) {
        if (s1.size() != s2.size()) return false;
        return is_subset(s1, s2) and is_subset(s2,s1);
    }

    std::vector<int> tuple_from_position(int nvalues, int arity, int position);

    int position_from_tuple(int nvalues, int arity, const std::vector<int>& tuple);

    int compute_number_of_functions(int nvalues, int arity);

    int compute_number_of_rows(int nvalues, int arity);

    template<typename T>
    std::vector<std::vector<T>> cartesian_product(const std::set<T>& s1, const std::set<T>& s2);
    extern template std::vector<std::vector<int>> cartesian_product(const std::set<int>& s1, const std::set<int>& s2);

    template<typename T>
    std::vector<std::vector<T>> cartesian_product(const std::vector<std::vector<T>>& s1, const std::set<T>& s2);
    extern template std::vector<std::vector<int>> cartesian_product(const std::vector<std::vector<int>>& s1, const std::set<int>& s2);

    template<typename T>
    std::vector<std::vector<T>> cartesian_product(const std::vector<std::set<T>>& sets);
    extern template std::vector<std::vector<int>> cartesian_product(const std::vector<std::set<int>>& sets);

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

    template<typename T, typename Comp, typename... Args>
    void insert_set(std::set<T, Comp>& v, Args&&... args)
    {
        static_assert((std::is_constructible_v<T, Comp, Args&&> && ...));
        (v.insert(std::forward<Args>(args)), ...);
    }

};

template<typename T>
bool operator==(const std::set<std::shared_ptr<T>, ltsy::utils::DeepSharedPointerComp<T>>& s1,
            const std::set<std::shared_ptr<T>, ltsy::utils::DeepSharedPointerComp<T>>& s2) {
    return ltsy::utils::equals(s1,s2); 
}

#endif
