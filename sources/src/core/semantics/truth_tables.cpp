#include "core/semantics/truth_tables.h"
#include "core/utils.h"
#include "core/exception.h"
#include <iostream>

namespace ltsy {

    //template<typename CellType>
    //TruthTable<CellType>::TruthTable(int nvalues, int arity, int position) : TruthTable {nvalues, arity} {
        //TODO validate position
    //    this->_images = utils::tuple_from_position(nvalues, arity, position);
    //}

    template<typename CellType>
    TruthTable<CellType>::TruthTable(int _nvalues, const std::initializer_list<TruthTableRow>& rows) {
        this->_nvalues = _nvalues;
        for (auto& row : rows) {
            auto [input_tuple, value] = row;

            if (_arity == -1) {
                this->_arity = input_tuple.size();
                this->_number_of_rows = this->compute_number_of_rows(this->_nvalues, this->_arity); 
                this->_images = std::vector<CellType>(this->_number_of_rows);
            }
            else if (_arity != input_tuple.size())
                throw std::invalid_argument(ltsy::WRONG_ARITY_INPUT_EXCEPTION);

            auto pos = utils::position_from_tuple(_nvalues, _arity, input_tuple);
            this->_images[pos] = value;
        }
    }

    template<typename CellType>
    CellType TruthTable<CellType>::at(const std::vector<int>& input) const {
        if (input.size() != this->_arity)
            throw std::invalid_argument(ltsy::WRONG_ARITY_INPUT_EXCEPTION);
        auto position = utils::position_from_tuple(_nvalues, _arity, input);
        return _images[position];
    }
};
