#ifndef __TRUTH_TABLES__
#define __TRUTH_TABLES__

#include <cmath>
#include <vector>

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
    class TruthTable {
        
        private:

            int _nvalues;
            int _number_of_rows;
            int _arity = -1;
            std::vector<int> _images;

            using TruthTableRow = std::pair<std::vector<int>, int>;

            static constexpr auto compute_number_of_rows = [](int nvalues, int arity) { return int(std::pow(nvalues, arity)); };

        public:

            TruthTable(int _nvalues, int _arity) : 
                _nvalues {_nvalues},
                _arity {_arity},
                _number_of_rows { this->compute_number_of_rows(_nvalues, _arity) },
                _images {std::vector<int>(_number_of_rows, 0)}
                {/* empty */}

            /**
             * Build truth table according to its lexicographic
             * position.
             * */
            TruthTable(int _nvalues, int _arity, int i);

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
            inline int at(int i) const { return _images[i]; }

            /* Return the image at a given input tuple.
             * */
            int at(const std::vector<int>& input) const;

            inline int arity() const { return _arity; }

            inline int nvalues() const { return _nvalues; }
    };

};

#endif
