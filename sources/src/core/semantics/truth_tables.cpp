#include "core/semantics/truth_tables.h"
#include "core/utils.h"
#include "core/exception.h"
#include "core/common.h"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace ltsy {

    template<typename CellType>
    TruthTableBase<CellType>::TruthTableBase(int _nvalues, const std::initializer_list<TruthTableRow>& rows) {
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
    CellType TruthTableBase<CellType>::at(const std::vector<int>& input) const {
        if (input.size() != this->_arity)
            throw std::invalid_argument(ltsy::WRONG_ARITY_INPUT_EXCEPTION);
        auto position = utils::position_from_tuple(_nvalues, _arity, input);
        return _images[position];
    }


    template<>
    std::stringstream TruthTableBase<std::set<int>>::print(const std::map<int, std::string>& values_map) const {
        auto get_or_default = [&](const int& v){ 
            return values_map.find(v) != values_map.end() ? values_map.find(v)->second : std::to_string(v); 
        };
        std::stringstream ss;
        for (auto i = 0; i < _images.size(); ++i) {
            auto row = utils::tuple_from_position(_nvalues, _arity, i);
            for (auto it = row.cbegin(); it != row.cend(); it++) {
                ss << std::setw(5) << get_or_default(*it);
                if (std::next(it) != row.cend())
                    ss << " ";
            }
            ss << " -> ";
            for (const auto& img : _images[i])
                ss << get_or_default(img) << " ";
            if (i < _number_of_rows - 1)
                ss << std::endl;
        }
        return ss;   
    }

    template<>
    std::stringstream TruthTableBase<std::set<int>>::print() const {
        return this->print(std::map<int, std::string>{});
    }

    template<typename CellType>
    std::stringstream TruthTableBase<CellType>::print(std::function<void(std::stringstream&, const CellType&)> cell_printer) const {
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
    bool Determinant<std::set<int>>::operator<(const Determinant<std::set<int>>& other) const {
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

    NDTruthTable generate_fully_nd_table(int nvalues, int arity) {
        auto nrows = utils::compute_number_of_rows(nvalues, arity);
        NDTruthTable tt (nvalues, arity);
        std::set<int> s;
        for (int i = 0; i < nvalues; ++i)
            s.insert(i);
        for (int i = 0; i < nrows; ++i)
            tt.set(i, s);
        return tt;
    }

    NDTruthTable generate_fully_partial_table(int nvalues, int arity) {
        auto nrows = utils::compute_number_of_rows(nvalues, arity);
        NDTruthTable tt {nvalues, arity};
        for (int i = 0; i < nrows; ++i)
            tt.set(i, std::set<int>{});
        return tt;
    }
};
