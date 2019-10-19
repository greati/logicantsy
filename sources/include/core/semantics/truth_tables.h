#ifndef __TRUTH_TABLES__
#define __TRUTH_TABLES__

#include <cmath>

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
    template<int NValues, int Arity>
    class TruthTable {
        
        private:

            static const int _number_of_lines = int(std::pow(NValues, Arity));
            std::array<int, _number_of_lines> _images;

            using InputTuple = std::array<int, Arity>;
            using TableRow = std::pair<InputTuple, int>;

        public:

            /**
             * Build truth table according to its lexicographic
             * position.
             * */
            TruthTable(int i);

            /**
             * Build truth table giving its images array.
             * */
            TruthTable(const decltype(_images)& _images) : _images {_images} { /* empty */ } 

            /**
             * Build a truth table from its rows.
             * */
            TruthTable(const std::initializer_list<TableRow>& rows);

            /* Gives the image at the given position.
             * */
            inline int at(int i) const { return _images[i]; }

            /* Return the image at a given input tuple.
             * */
            int at(const std::array<int, Arity>& input) const;
    };

};

#include "../../../src/core/semantics/truth_tables.cpp"

#endif
