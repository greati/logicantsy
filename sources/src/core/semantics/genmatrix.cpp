#include "core/semantics/genmatrix.h"

namespace ltsy {
    std::pair<bool, std::optional<std::set<std::shared_ptr<Formula>>>> 
    GenMatrixVarAssignment::is_model(const std::set<std::shared_ptr<Formula>>& fmls,
            const std::set<int>& dset) {
        std::set<std::shared_ptr<Formula>> fail_fmls;
        for (const auto& f : fmls) {
           GenMatrixPossibleValuesCollector collector {shared_from_this()};
           auto fmla_values = f->accept(collector);     
           std::set<int> inters;
           std::set_intersection(dset.begin(), dset.end(), fmla_values.begin(), fmla_values.end(), std::inserter(inters,inters.begin()));
           if (inters.empty())
               fail_fmls.insert(f);
        }
        if (fail_fmls.empty()) return {true, std::nullopt};
        else return std::pair{false, fail_fmls};
    }
};
