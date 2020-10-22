#ifndef __SYNTAX__
#define __SYNTAX__

#include <string>
#include <atomic>
#include <memory>
#include <map>
#include "core/exception.h"
#include "core/common.h"
#include "core/utils.h"
#include <unordered_set>
#include <algorithm>
#include <sstream>

namespace ltsy {
    
    /* Represents a connective, with a symbol
     * and an arity.
     *
     * @author Vitor Greati
     * */
    class Connective {
        private:
            Symbol _symbol;       //< Connective symbol
            Arity _arity;         //< Connective arity
        public:

            /* Empty constructor.
             * */
            Connective() {}

            /* Construct a connective from its symbol and its
             * arity.
             *
             * @param symbol
             * @param arity
             * */
            Connective(const Symbol& _symbol, Arity _arity) : _symbol {_symbol} {
                if (_arity < 0)
                   throw std::invalid_argument(NEGATIVE_ARITY_EXCEPTION); 
                this->_arity = _arity;
            }
            
            /* Get symbol.
             *
             * @return the symbol of the connective
             * */
            inline Symbol symbol() const { return _symbol; }

            /* Get connective arity.
             *
             * @return the arity of the connective
             * */
            inline Arity arity() const { return _arity; }

            /* Equality occurs with respect to the symbol
             * and the arity.
             * */
            inline bool operator==(const Connective& other) const {
                return (_symbol == other._symbol) and (_arity == other._arity);
            }

            /* Different is not equal.
             * */
            inline bool operator!=(const Connective& other) const {
                return not (*this == other);
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
            Signature(std::initializer_list<std::shared_ptr<Connective>> connectives) {
                for (const auto& c : connectives)
                    this->add(std::atomic_load(&c));
            }

            Signature(std::initializer_list<Connective> connectives) {
                for (const auto& c : connectives)
                    this->add(std::make_shared<Connective>(c.symbol(), c.arity()));
            }

            auto begin() { return _signature.begin(); }
            auto end() { return _signature.end(); }
            auto cbegin() { return _signature.cbegin(); }
            auto cend() { return _signature.cend(); }

            void add(std::shared_ptr<Connective> connective) {
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
                return std::atomic_load(&it->second);
            }

            bool operator==(const Signature& other) const {
                return other._signature == this->_signature;
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

    class EqualityFormulaVisitor;

    /**
     * Represents a general formula.
     *
     * @author Vitor Greati
     * */
    class Formula {

        public:

            enum class FmlaType {
                PROP,
                COMPOUND,
                UNKNOWN
            };

        protected:

            FmlaType _type = FmlaType::UNKNOWN;

        public:
            Formula() {}
            virtual ~Formula() {}
            virtual int accept(FormulaVisitor<int>& visitor) = 0;
            virtual std::set<int> accept(FormulaVisitor<std::set<int>>& visitor) = 0;
            virtual void accept(FormulaVisitor<void>& visitor) = 0;
            virtual bool accept(FormulaVisitor<bool>& visitor) = 0;
            inline FmlaType type() { return _type; }
            friend std::ostream& operator<<(std::ostream& os, Formula& f);
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
            Prop() : _symbol {"?"} {
                _type = FmlaType::PROP;
            }
            Prop(const Symbol& _symbol) : _symbol {_symbol} {
                _type = FmlaType::PROP;
            }
            ~Prop() {}

            inline Symbol symbol() const { return _symbol; };

            bool operator<(const Prop& p1) const {
                return _symbol < p1._symbol;
            }

            bool operator==(const Prop& p2) const {
                return _symbol == p2._symbol;
            }

            inline int accept(FormulaVisitor<int>& visitor) {
                return visitor.visit_prop(this);
            }

            inline bool accept(FormulaVisitor<bool>& visitor) {
                return visitor.visit_prop(this);
            }

            inline void accept(FormulaVisitor<void>& visitor) {
                visitor.visit_prop(this);
            }

            inline std::set<int> accept(FormulaVisitor<std::set<int>>& visitor) {
                return visitor.visit_prop(this);
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
            Compound(decltype(_connective) _connective, const decltype(_components)& _components) {
                if (_components.size() != _connective->arity())
                    throw std::invalid_argument(WRONG_NUMBER_ARGUMENTS_FMLA_EXCEPTION);
                else {
                    std::atomic_store(&this->_connective, std::move(_connective));
                    this->_components = _components;
                }
                _type = FmlaType::COMPOUND;
            }
            ~Compound() {}

            decltype(_connective) connective() const {
                return _connective;
            }

            decltype(_components) components() const {
                return _components;
            }

            inline int accept(FormulaVisitor<int>& visitor) {
                return visitor.visit_compound(this);
            }
            inline bool accept(FormulaVisitor<bool>& visitor) {
                return visitor.visit_compound(this);
            }
            inline void accept(FormulaVisitor<void>& visitor) {
                visitor.visit_compound(this);
            }
            inline std::set<int> accept(FormulaVisitor<std::set<int>>& visitor) {
                return visitor.visit_compound(this);
            }
    };

    /**
     * A visitor that tests formula equality.
     */
    class EqualityFormulaVisitor : public FormulaVisitor<bool> {
        private:

            std::shared_ptr<Formula> _left;

        public:
            EqualityFormulaVisitor(decltype(_left) left) : _left {left} {};

            virtual bool visit_prop(Prop* prop) override {
                if (_left->type() != Formula::FmlaType::PROP)
                    return false;
                auto left_prop = std::dynamic_pointer_cast<Prop>(_left);
                return left_prop->symbol() == prop->symbol();
            }

            virtual bool visit_compound(Compound* compound) override {
                if (_left->type() != Formula::FmlaType::COMPOUND)
                   return false; 
                auto left = std::dynamic_pointer_cast<Compound>(_left);
                auto left_conn = left->connective();
                if (*(compound->connective()) != *(left_conn))
                    return false;
                auto left_comps = left->components();
                auto right_comps = compound->components();
                for (int i = 0; i < left_comps.size(); ++i) {
                    EqualityFormulaVisitor eqvis {left_comps[i]}; 
                    if (not right_comps[i]->accept(eqvis))
                        return false;
                }
                return true;
            }
    };


    /**
     * A visitor that collects all variables in a formula.
     */
    class VariableCollector : public FormulaVisitor<void> {
        private:
            std::set<Prop*, utils::DeepPointerComp<Prop>> collected_variables;

        public:
            virtual void visit_prop(Prop* prop) override {
                this->collected_variables.insert(prop);
            }
            virtual void visit_compound(Compound* compound) override {
                auto components = compound->components();
                std::for_each(components.begin(), components.end(),
                        [&](std::shared_ptr<Formula> fmla) {
                           return fmla->accept(*this); 
                        });
            }
            virtual decltype(collected_variables) get_collected_variables() {
                return this->collected_variables;
            }
    };

    /**
     * A visitor that collects the least signature of a given formula.
     */
    class SignatureCollector : public FormulaVisitor<void> {
        private:
            Signature collected_signature;

        public:
            virtual void visit_prop(Prop* prop) override {/* empty */}
            virtual void visit_compound(Compound* compound) override {
                collected_signature.add(compound->connective());
                auto components = compound->components();
                std::for_each(components.begin(), components.end(),
                        [&](std::shared_ptr<Formula> fmla) {
                           return fmla->accept(*this); 
                        });
            }
            virtual Signature get_collected_signature() {
                return this->collected_signature;
            }
    };

    /**
     * A visitor to print a formula.
     *
     * @author Vitor Greati
     * */
    class FormulaPrinter : public FormulaVisitor<void> {
        private:
            std::stringstream buffer;
        public:
            virtual void visit_prop(Prop* prop) override {
                buffer << prop->symbol();
            }
            virtual void visit_compound(Compound* compound) override {
                auto arity = compound->connective()->arity();
                auto symbol = compound->connective()->symbol();
                if (arity != 2) {
                    buffer << symbol << "(";
                    auto components = compound->components();
                    for (auto it = components.cbegin(); it != components.cend(); ++it) {
                        (*it)->accept(*this);
                        if (std::next(it) != components.cend())
                            buffer << ",";
                    }
                    buffer <<")";
                } else {
                    auto components = compound->components();
                    components[0]->accept(*this);
                    buffer << " " << symbol << " ";
                    components[1]->accept(*this);
                }
            }
            std::string get_string() { return buffer.str(); }
    };
}
#endif
