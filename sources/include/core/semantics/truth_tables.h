#ifndef __TRUTH_TABLES__
#define __TRUTH_TABLES__

#include <cmath>
#include <vector>
#include <unordered_set>
#include "core/utils.h"
#include <functional>
#include <stdexcept>

namespace ltsy {

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
             * Build truth table according to its lexicographic
             * position.
             * */
            //TruthTable(int _nvalues, int _arity, int i);

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
