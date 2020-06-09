#ifndef __SEQUENTS__
#define __SEQUENTS__

#include <unordered_set>
#include "core/syntax.h"

namespace ltsy {
    /**
     * Represents a sequent. The framework (Set-Set, Set-Fmla, Fmla-Fmla...)
     * is determined by the chosen template parameters.
     * */
    template<typename LeftType, typename RightType>
    class Sequent {

        protected:
            LeftType _left_side;
            RightType _right_side;

        public:

            Sequent() {/*empty*/}

            Sequent(LeftType left_side, RightType right_side) 
                : _left_side {left_side}, _right_side {right_side}
            {/* empty */}

            /**
             * Apply function to the left side of the sequent.
             */
            void apply_left(std::function<void(const LeftType&)> f) {
                f(_left_side);
            }

            /**
             * Apply function to the right side of the sequent.
             */
            void apply_right(std::function<void(const RightType&)> f) {
                f(_right_side);
            }

            /**
             * Return the left side of the sequent.
             */
            LeftType& left() { return this->_left_side; }

            /**
             * Return the right side of the sequent.
             */
            RightType& right() { return this->_right_side; };

            /**
             * Print the sequent.
             * */
            virtual void print() const = 0;

    };


    class SetSetSequent : public Sequent<std::unordered_set<std::shared_ptr<Formula>>, std::unordered_set<std::shared_ptr<Formula>>> {
        public:
            SetSetSequent() : Sequent {} {/*empty*/};
            SetSetSequent(std::unordered_set<std::shared_ptr<Formula>> left, std::unordered_set<std::shared_ptr<Formula>> right) :
                Sequent {left, right} {/*empty*/};    

            void print() const override {
                std::stringstream buffer;
                for (auto it = _left_side.cbegin(); it != _left_side.cend(); it++) {
                    FormulaPrinter printer;
                    (*it)->accept(printer);
                    buffer << printer.get_string();
                    if (std::next(it) != _left_side.cend())
                        buffer << ", ";
                }
                buffer << " >- ";
                for (auto it = _right_side.cbegin(); it != _right_side.cend(); it++) {
                    FormulaPrinter printer;
                    (*it)->accept(printer);
                    buffer << printer.get_string();
                    if (std::next(it) != _right_side.cend())
                        buffer << ", ";
                }
                std::cout << buffer.str();
            };
    };

    class SetFmlaSequent : public Sequent<std::unordered_set<std::shared_ptr<Formula>>, std::shared_ptr<Formula>> {
        public:
            SetFmlaSequent() : Sequent {} {/*empty*/};
            SetFmlaSequent(std::unordered_set<std::shared_ptr<Formula>> left, std::shared_ptr<Formula> right) :
                Sequent {left, right} {/*empty*/};    

            void print() const override {
                std::stringstream buffer;
                for (auto it = _left_side.cbegin(); it != _left_side.cend(); it++) {
                    FormulaPrinter printer;
                    (*it)->accept(printer);
                    buffer << printer.get_string();
                    if (std::next(it) != _left_side.cend())
                        buffer << ", ";
                }
                buffer << " >- ";
                FormulaPrinter printer;
                _right_side->accept(printer);
                buffer << printer.get_string();
                std::cout << buffer.str();
            };
    };

    class FmlaFmlaSequent : public Sequent<std::shared_ptr<Formula>, std::shared_ptr<Formula>> {
        public:
            FmlaFmlaSequent() : Sequent {} {/*empty*/};
            FmlaFmlaSequent(std::shared_ptr<Formula> left, std::shared_ptr<Formula> right) :
                Sequent {left, right} {/*empty*/};    

            void print() const override {
                std::stringstream buffer;
                FormulaPrinter left_printer;
                _left_side->accept(left_printer);
                buffer << left_printer.get_string();
                buffer << " >- ";
                FormulaPrinter printer;
                _right_side->accept(printer);
                buffer << printer.get_string();
                std::cout << buffer.str();
            };
    };

};
#endif
