#ifndef __TRUTH_TABLES__
#define __TRUTH_TABLES__

#include <cmath>
#include <vector>
#include <unordered_set>
#include "core/utils.h"
#include <functional>
#include <stdexcept>
#include <set>

namespace ltsy {

    /**
     * Represents a determinant. 
     *
     * Stores the arguments as an int, from which
     * we can compute the corresponding tuple
     * on demand.
     *
     * @author Vitor Greati
     * */
    template<typename CellType = int>
    class Determinant {
        private:
            int _nvalues;   /*< arg values take values from 0..(_nvalues-1)*/
            int _arity;     /*< arg size is _arity (so the determinant is _arity + 1) */
            std::pair<int, CellType> _data;

        public:

            Determinant(int nvalues, int arity, int tuple_pos, CellType last) : 
                _nvalues {nvalues}, _arity {arity}
            {
                _data = {tuple_pos, last};
            }

            Determinant(int nvalues, std::vector<int> args, CellType last) 
                : _nvalues {nvalues}, _arity {args.size()} {
                _data = {utils::position_from_tuple(_nvalues, _arity, args), last};
            }

            inline CellType get_last() const { return _data.second; }

            inline std::vector<int> get_args() const { 
                return utils::tuple_from_position(_nvalues, _arity, _data.first); 
            }

            bool operator==(const Determinant<CellType>& other) const {
                return _data == other._data;
            }

            bool operator<(const Determinant<CellType>& other) const;

            friend std::ostream& operator<<(std::ostream& os, const Determinant<CellType>& det) {
                auto args = det.get_args();
                os << std::string("< ");
                for (auto it = args.begin(); it != args.end(); ++it)
                    os << (*it) << std::string(" ");
                os << det.get_last();
                os << std::string(" >");
                return os;  
            }
    };



    /**
     * Represents an `NValues`-valued `Arity`-ary truth table. 
     * Values are taken to be 0,...,`NValues`-1.
     *
     * Only the images are stored. Each input
     * tuple is represented by a natural number
     * corresponding to the position of the tuple
     * in the lexicographical order.
     *
     * @author Vitor Greati
     * */
    template<typename CellType = int>
    class TruthTable {
        
        private:

            int _nvalues;
            int _number_of_rows;
            int _arity = -1;
            std::vector<CellType> _images;

            using TruthTableRow = std::pair<std::vector<int>, CellType>;

            static constexpr auto compute_number_of_rows = [](int nvalues, int arity) { return int(std::pow(nvalues, arity)); };

        public:

            TruthTable(int _nvalues, int _arity) : 
                _nvalues {_nvalues},
                _arity {_arity},
                _number_of_rows { this->compute_number_of_rows(_nvalues, _arity) },
                _images {std::vector<CellType>(_number_of_rows)}
                {/* empty */}

            /**
             * Build truth table giving its images array.
             * */
            TruthTable(int _nvalues, int _arity, const decltype(_images)& _images) : 
                TruthTable {_nvalues, _arity}
                { this->_images = _images; } 

            /**
             * Build a truth table from its rows.
             * */
            TruthTable(int nvalues, const std::initializer_list<TruthTableRow>& rows);

            /* Gives the image at the given position.
             * */
            inline CellType at(int i) const { return _images[i]; }

            /* Return the image at a given input tuple.
             * */
            CellType at(const std::vector<int>& input) const;

            inline int arity() const { return _arity; }

            inline int nvalues() const { return _nvalues; }

            std::set<Determinant<CellType>> get_determinants() const {
                std::set<Determinant<CellType>> result;
                for (auto i {0}; i < _number_of_rows; ++i){
                    Determinant<CellType> d (_nvalues, _arity, i, at(i));
                    result.insert(d);
                }
                return result;
            }

            std::stringstream print(std::function<void(std::stringstream&, const CellType&)> cell_printer) const;
    };

    template class TruthTable<int>;
    template class TruthTable<std::unordered_set<int>>;
        
    class TruthTableGenerator {
        
        private:
            int _current_index = 0;
            int _nvalues;
            int _arity;
            int _quantity;
            int _number_of_rows;

        public:

            TruthTableGenerator(int nvalues, int arity) 
                : _nvalues {nvalues}, _arity {arity} {
                _number_of_rows = utils::compute_number_of_rows(nvalues, arity);
                _quantity = utils::compute_number_of_functions(nvalues, arity);
            }

            TruthTable<int> next() {
                if (has_next()) {
                    auto images = utils::tuple_from_position(_nvalues, _number_of_rows, _current_index);
                    ++_current_index;
                    return TruthTable<int> {_nvalues, _arity, images};
                } else {
                    throw std::logic_error("no truth table available to generate");
                }
            }

            bool has_next() {
                return _current_index < _quantity;
            }

    };
};

#endif
