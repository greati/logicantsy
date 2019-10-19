#include "core/semantics/truth_tables.h"
#include "core/utils.h"

namespace ltsy {

    template<int NValues, int Arity>
    int TruthTable<NValues, Arity>::at(const std::array<int, Arity>& input) const {
        auto position = utils::position_from_tuple<NValues, Arity>(input);
        return _images[position];
    }

    template<int NValues, int Arity>
    TruthTable<NValues, Arity>::TruthTable(int position) {
        this->_images = utils::tuple_from_position<NValues, this->_number_of_lines>(position);
    }

    template<int NValues, int Arity>
    TruthTable<NValues, Arity>::TruthTable(const std::initializer_list<TableRow>& rows) {
        for (auto& row : rows) {
            auto [input_tuple, value] = row;
            auto pos = utils::position_from_tuple<NValues,Arity>(input_tuple);
            this->_images[pos] = value;
        }
    }
};
