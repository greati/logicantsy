#ifndef __SYNTAX__
#define __SYNTAX__

#include <string>
#include <memory>
#include "core/exception.h"
#include "core/common.h"

namespace ltsy {
    
    /**
     * Represents a connective, with a symbol
     * and an arity.
     *
     * @author Vitor Greati
     * */
    class Connective {
        private:
            Symbol _symbol;       //< Connective symbol
            Arity _arity;         //< Connective arity
        public:
            Connective() {}
            Connective(const Symbol& _symbol, Arity _arity) : _symbol {_symbol} {
                if (_arity < 0)
                   throw std::invalid_argument(NEGATIVE_ARITY_EXCEPTION); 
                this->_arity = _arity;
            }
            
            inline Symbol symbol() const { return _symbol; }
            inline Arity arity() const { return _arity; }

            bool operator==(const Connective& other) const {
                return (_symbol == other._symbol) and (_arity == other._arity);
            }
    };

    /**
     * Signature supporting indexing by arity and connective.
     *
     * @author Vitor Greati
     * */
    class Signature {

        private:

            std::map<Symbol, std::shared_ptr<Connective>> _signature;

        public:

            Signature() {}
            Signature(const std::initializer_list<std::shared_ptr<Connective>> connectives) {
                for (const auto& c : connectives)
                    this->add(c);
            }

            Signature(const std::initializer_list<Connective> connectives) {
                for (const auto& c : connectives)
                    this->add(std::make_shared<Connective>(c.symbol(), c.arity()));
            }

            void add(const std::shared_ptr<Connective>& connective) {
                _signature.insert({connective->symbol(), connective});
            }

            /**
             * Join two signatures, preserving the elements of the
             * left-hand side.
             * */
            void join(const Signature& signature) {
                _signature.insert(signature._signature.begin(), signature._signature.end());
            }

            Signature operator()(Arity arity) const {
                Signature connectives_by_arity;
                if (arity < 0)
                   throw std::invalid_argument(NEGATIVE_ARITY_EXCEPTION);
                for (auto& [key,value] : _signature)
                    if (value->arity() == arity)
                        connectives_by_arity.add(value);
                return connectives_by_arity;
            }

            std::shared_ptr<Connective> operator[](const Symbol& symbol) {
                auto it = _signature.find(symbol);
                if (it == _signature.end())
                    throw ltsy::ConnectiveNotPresentException(symbol);
                return it->second;
            }

    };

    class Compound;
    class Prop;
 
    /**
     * A visitor for the formula structure.
     *
     * @author Vitor Greati
     * */
    template<typename ReturnT>
    class FormulaVisitor {
        public:
            virtual ReturnT visit_prop(Prop* prop) = 0;
            virtual ReturnT visit_compound(Compound* compound) = 0;
    };



    /**
     * Represents a general formula.
     *
     * @author Vitor Greati
     * */
    class Formula {
        public:
            virtual int accept(FormulaVisitor<int>& visitor) = 0;
            virtual void accept(FormulaVisitor<void>& visitor) = 0;
    };

    /**
     * Represents a propositional symbol.
     *
     * @author Vitor Greati
     * */
    class Prop : public Formula {
        private:
            Symbol _symbol;
        public:
            Prop(const Symbol& _symbol) : _symbol {_symbol} {}

            inline Symbol symbol() const { return _symbol; };

            bool operator<(const Prop& p1) const {
                return _symbol < p1._symbol;
            }

            inline int accept(FormulaVisitor<int>& visitor) {
                return visitor.visit_prop(this);
            }

            inline void accept(FormulaVisitor<void>& visitor) {
                visitor.visit_prop(this);
            }
    };


    /**
     * Represents a compound formula.
     *
     * @author Vitor Greati
     * */
    class Compound : public Formula {
        private:
            std::shared_ptr<Connective> _connective;
            std::vector<std::shared_ptr<Formula>> _components;
        public:
            Compound(const decltype(_connective)& _connective, const decltype(_components)& _components) {
                if (_components.size() != _connective->arity())
                    throw std::invalid_argument(WRONG_NUMBER_ARGUMENTS_FMLA_EXCEPTION);
                else {
                    this->_connective = _connective;
                    this->_components = _components;
                }
            }

            const decltype(_connective)& connective() const {
                return _connective;
            }

            const decltype(_components)& components() const {
                return _components;
            }

            inline int accept(FormulaVisitor<int>& visitor) {
                return visitor.visit_compound(this);
            }
            inline void accept(FormulaVisitor<void>& visitor) {
                visitor.visit_compound(this);
            }
    };

    /**
     * A visitor to print a formula.
     *
     * @author Vitor Greati
     * */
    class FormulaPrinter : public FormulaVisitor<void> {
        private:
            std::string str = "";
        public:
            virtual void visit_prop(Prop* prop) override {
                std::cout << " " << prop->symbol() << " ";
            }
            virtual void visit_compound(Compound* compound) override {
                std::cout << "(" << compound->connective()->symbol() << "(";
                auto components = compound->components();
                std::for_each(components.begin(), components.end(),
                        [&](std::shared_ptr<Formula> fmla) {
                           return fmla->accept(*this); 
                        });
                std::cout <<"))";
            }
    };

}
#endif
