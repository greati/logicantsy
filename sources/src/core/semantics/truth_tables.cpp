#include "core/semantics/truth_tables.h"
#include "core/utils.h"
#include "core/exception.h"
#include "core/common.h"
#include <iostream>
#include <sstream>

namespace ltsy {

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

    template<typename CellType>
    std::stringstream TruthTable<CellType>::print(std::function<void(std::stringstream&, const CellType&)> cell_printer) const {
        std::stringstream ss;
        for (auto i = 0; i < _images.size(); ++i) {
            auto row = utils::tuple_from_position(_nvalues, _arity, i);
            for (auto it = row.cbegin(); it != row.cend(); it++) {
                ss << (*it);
                if (std::next(it) != row.cend())
                    ss << " ";
            }
            ss << " -> ";
            cell_printer(ss, _images[i]);
            if (i < _number_of_rows - 1)
                ss << std::endl;
        }
        return ss; 
    }

    template<>
    bool Determinant<std::unordered_set<int>>::operator<(const Determinant<std::unordered_set<int>>& other) const {
        if (_data.first != other._data.first)
            return _data.first < other._data.first;
        else {
           return utils::is_subset<int>(this->_data.second, other._data.second);  
        }
        return false;
    }

    template<typename CellType>
    bool Determinant<CellType>::operator<(const Determinant<CellType>& other) const {
        if (_data.first == other._data.first)
            return _data.second < other._data.second;
        else
            return _data.first < other._data.first;
    }

    TruthTable<std::unordered_set<int>> generate_fully_nd_table(int nvalues, int arity) {
        auto nrows = utils::compute_number_of_rows(nvalues, arity);
        TruthTable<std::unordered_set<int>> tt {nvalues, arity};
        std::unordered_set<int> s;
        for (int i = 0; i < nvalues; ++i)
            s.insert(i);
        for (int i = 0; i < nrows; ++i)
            tt.set(i, s);
        return tt;
    }

    TruthTable<std::unordered_set<int>> generate_fully_partial_table(int nvalues, int arity) {
        auto nrows = utils::compute_number_of_rows(nvalues, arity);
        TruthTable<std::unordered_set<int>> tt {nvalues, arity};
        for (int i = 0; i < nrows; ++i)
            tt.set(i, std::unordered_set<int>{});
        return tt;
    }
};
