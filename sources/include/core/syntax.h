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
#include "assert.h"
#include <iostream>

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

            //> Map relating a symbol with a connective (having the same symbol)
            std::map<Symbol, std::shared_ptr<Connective>> _signature;

        public:

            /* Empty constructor.
             */
            Signature() {}

            /* Constructor that accepts a list of (pointers to) connectives.
             * */
            Signature(std::initializer_list<std::shared_ptr<Connective>> connectives) {
                for (const auto& c : connectives)
                    this->add(std::atomic_load(&c));
            }

            /* Constructor that accepts a list of concrete connectives.
             * */
            Signature(std::initializer_list<Connective> connectives) {
                for (const auto& c : connectives)
                    this->add(std::make_shared<Connective>(c.symbol(), c.arity()));
            }

            /* Allows iterating over the signature.
             * */
            auto begin() { return _signature.begin(); }
            auto end() { return _signature.end(); }
            auto cbegin() { return _signature.cbegin(); }
            auto cend() { return _signature.cend(); }

            /* Add a connective to the signature. Overwrite
             * if already exists.
             * */
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

            /* Produce a subsignature with connectives of
             * the given arity.
             * */
            Signature operator()(Arity arity) const {
                Signature connectives_by_arity;
                if (arity < 0)
                   throw std::invalid_argument(NEGATIVE_ARITY_EXCEPTION);
                for (auto& [key,value] : _signature)
                    if (value->arity() == arity)
                        connectives_by_arity.add(value);
                return connectives_by_arity;
            }

            /* Recover the connective based on its symbol, using
             * square brackets as accessors.
             * */
            std::shared_ptr<Connective> operator[](const Symbol& symbol) {
                auto it = _signature.find(symbol);
                if (it == _signature.end())
                    throw ltsy::ConnectiveNotPresentException(symbol);
                return std::atomic_load(&it->second);
            }

            /* Signature equality.
             * */
            bool operator==(const Signature& other) const {
                return other._signature == this->_signature;
            }

            /* Indicates if the signature is empty.
             * */
            bool empty() const { return _signature.empty();};

    };

    // Some forward declarations
    class Compound;
    class Prop;
 
    /**
     * A non-const visitor for the formula structure. The goal is
     * to avoid growing the Formula class whenever we need
     * a new operation over formulas.
     *
     * @author Vitor Greati
     * */
    template<typename ReturnT>
    class FormulaVisitor {
        public:

            /* Performs an operation over propositional variables.
             * */
            virtual ReturnT visit_prop(Prop* prop) = 0;

            /* Performs an operation over compounds.
             * */
            virtual ReturnT visit_compound(Compound* compound) = 0;
    };

    /**
     * A const visitor for the formula structure. The goal is
     * to avoid growing the Formula class whenever we need
     * a new operation over formulas.
     *
     * @author Vitor Greati
     * */
    template<typename ReturnT>
    class FormulaConstVisitor {
        public:
            virtual ReturnT visit_prop(const Prop* prop) const = 0;
            virtual ReturnT visit_compound(const Compound* compound) const = 0;
    };

    class EqualityFormulaVisitor;

    /**
     * Represents a propositional formula. It is abstract,
     * its concretization are propositions and compounds.
     *
     * @author Vitor Greati
     * */
    class Formula : public std::enable_shared_from_this<Formula>{

        public:

            /* Type of the formula, in order
             * ease some checks and avoid casts.
             * */
            enum class FmlaType {
                PROP,
                COMPOUND,
                UNKNOWN
            };

        protected:

            FmlaType _type = FmlaType::UNKNOWN; //> Indicates the formula type
            unsigned int _complexity; //> The formula complexity (occurrence of connectives)

        public:
            /* Empty constructor.
             */
            Formula() {}

            /* Virtual destructor.
             */
            virtual ~Formula() {}

            /* Accepts a non-const formula visitor that returns an int.
             */
            virtual int accept(FormulaVisitor<int>& visitor) = 0;

            /* Accepts a non-const formula visitor that returns an int.
             */
            virtual std::set<int> accept(FormulaVisitor<std::set<int>>& visitor) = 0;

            /* Accepts a non-const formula visitor that returns nothing.
             */
            virtual void accept(FormulaVisitor<void>& visitor) = 0;

            /* Accepts a const formula visitor that returns nothing.
             */
            virtual void accept(FormulaConstVisitor<void>& visitor) const = 0;

            /* Accepts a non-const formula visitor that returns a boolean.
             */
            virtual bool accept(FormulaVisitor<bool>& visitor) = 0;

            /* Accepts a const formula visitor that returns a boolean.
             */
            virtual bool accept(FormulaConstVisitor<bool>& visitor) const = 0;

            /* Accepts a non-const formula visitor that returns a pointer to a formula.
             */
            virtual std::shared_ptr<Formula> accept(FormulaVisitor<std::shared_ptr<Formula>>& visitor) = 0;

            /* Return the formula type.
             */
            inline FmlaType type() const { return _type; }

            /* Strict less comparator.
             * */
            bool operator<(const Formula& p1) const;

            /* Equality comparator.
             * */
            bool operator==(const Formula& p1) const;

            /* Outstream operator.
             * */
            friend std::ostream& operator<<(std::ostream& os, Formula& f);

            /* Return the complexity of the formula.
             * */
            decltype(_complexity) complexity() const { return _complexity; }
            
            /* Produce a copy of this formula.
             * */
            virtual std::shared_ptr<Formula> get_formula_copy() const = 0;

            /* Return the main connective of a formula.
             * If it is not a compound, no nullptr is expected.
             * */
            virtual std::shared_ptr<Connective> connective() const = 0;
    };

    /**
     * Represents a propositional variable.
     *
     * @author Vitor Greati
     * */
    class Prop : public Formula {
        private:
            Symbol _symbol; //> The propositional symbol
        public:
            
            /* Constructor that accepts the symbol.
             */
            Prop(const Symbol& _symbol) : _symbol {_symbol} {
                _type = FmlaType::PROP;
                _complexity = 0;
            }
            /* Empty constructor.
             * */
            Prop() : Prop{"p"} {}

            /* Destructor.
             */
            ~Prop() {}

            /* Returns the propositional symbol.
             * */
            inline Symbol symbol() const { return _symbol; };

            inline int accept(FormulaVisitor<int>& visitor) {
                return visitor.visit_prop(this);
            }

            inline bool accept(FormulaVisitor<bool>& visitor) {
                return visitor.visit_prop(this);
            }

            inline bool accept(FormulaConstVisitor<bool>& visitor) const {
                return visitor.visit_prop(this);
            }

            inline void accept(FormulaVisitor<void>& visitor) {
                visitor.visit_prop(this);
            }

            inline void accept(FormulaConstVisitor<void>& visitor) const {
                visitor.visit_prop(this);
            }

            inline std::shared_ptr<Formula> accept(FormulaVisitor<std::shared_ptr<Formula>>& visitor) {
                return visitor.visit_prop(this);
            }

            inline std::set<int> accept(FormulaVisitor<std::set<int>>& visitor) {
                return visitor.visit_prop(this);
            }

            std::shared_ptr<Formula> get_formula_copy() const override {
                return std::make_shared<Prop>(*this);
            }

            inline std::shared_ptr<Connective> connective() const override { return nullptr; }
    };


    /**
     * Represents a compound formula. It basically holds
     * a reference to a connective and a list of 
     * component formulas.
     *
     * @author Vitor Greati
     * */
    class Compound : public Formula {
        private:
            std::shared_ptr<Connective> _connective; //> Reference to a connective
            std::vector<std::shared_ptr<Formula>> _components; //> List of pointers to components.
        public:

            /* Constructor that accepts a connective and a list of
             * pointers to components.
             */
            Compound(decltype(_connective) _connective, const decltype(_components)& _components) {
                if (_components.size() != _connective->arity())
                    throw std::invalid_argument(WRONG_NUMBER_ARGUMENTS_FMLA_EXCEPTION);
                this->_connective = _connective;
                this->_components = _components;
                _type = FmlaType::COMPOUND;
                // compute complexity and check whether a compound is null
                // TODO is it worth keeping it here?
                _complexity = 1;
                for (const auto& c : _components)
                    if (c != nullptr)
                        _complexity += c->complexity();
                    else
                        throw std::invalid_argument(NULL_POINTER_TO_COMPOUND);
            }

            /* Destructor.
             */
            ~Compound() {}

            /* Retrieve the main connective.
             * */
            inline decltype(_connective) connective() const override {
                return _connective;
            }

            /* Retrieve the components.
             * */
            inline decltype(_components) components() const {
                return _components;
            }

            inline int accept(FormulaVisitor<int>& visitor) {
                return visitor.visit_compound(this);
            }

            inline bool accept(FormulaVisitor<bool>& visitor) {
                return visitor.visit_compound(this);
            }

            inline bool accept(FormulaConstVisitor<bool>& visitor) const {
                return visitor.visit_compound(this);
            }

            inline void accept(FormulaVisitor<void>& visitor) {
                visitor.visit_compound(this);
            }

            inline void accept(FormulaConstVisitor<void>& visitor) const {
                visitor.visit_compound(this);
            }

            inline std::shared_ptr<Formula> accept(FormulaVisitor<std::shared_ptr<Formula>>& visitor) {
                return visitor.visit_compound(this); 
            }

            inline std::set<int> accept(FormulaVisitor<std::set<int>>& visitor) {
                return visitor.visit_compound(this);
            }

            /* Strict less comparison.
             * TODO: this definition probably needs reformulation
             * */
            bool operator<(const Compound& p1) const {
                return _connective->arity() < p1._connective->arity();
            }

            /* Copy this formula.
             * */
            std::shared_ptr<Formula> get_formula_copy() const override {
                return std::make_shared<Compound>(*this);
            }
    };

    /* Set of pointers to formulas.
     * */
    using FmlaSet = std::set<std::shared_ptr<Formula>, utils::DeepSharedPointerComp<Formula>>;

    /* Set of pointers to propositional variables.
     * */
    using PropSet = std::set<std::shared_ptr<Prop>, utils::DeepSharedPointerComp<Prop>>;

    /* Compute the intersection of sets of formulas.
     * */
    FmlaSet intersection(const FmlaSet& s1, const FmlaSet& s2);

    /* Compute the difference of sets of formulas.
     * */
    FmlaSet difference(const FmlaSet& s1, const FmlaSet& s2);

    /* A visitor that tests formula equality.
     * It takes a left-hand side formula A and 
     * accepts by other formulas B, as right-hand side.
     */
    class EqualityFormulaVisitor : public FormulaConstVisitor<bool> {
        private:
            const Formula* _left; //> Left-hand side of comparison

        public:

            /* Constructor.
             * */
            EqualityFormulaVisitor(const decltype(_left) left) : _left {left} {};

            virtual bool visit_prop(const Prop* prop) const override {
                if (_left->type() != Formula::FmlaType::PROP)
                    return false;
                auto left_prop = dynamic_cast<const Prop*>(_left);
                return left_prop->symbol() == prop->symbol();
            }

            virtual bool visit_compound(const Compound* compound) const override {
                // check formula type
                if (_left->type() != Formula::FmlaType::COMPOUND)
                    return false; 
                // check formula complexity
                if (_left->complexity() != compound->complexity())
                    return false;
                auto left = dynamic_cast<const Compound*>(_left);
                // check head connective
                auto left_conn = left->connective();
                if (*(compound->connective()) != *(left_conn))
                    return false;
                // check components
                auto left_comps = left->components();
                auto right_comps = compound->components();
                for (int i = 0; i < left_comps.size(); ++i) {
                    EqualityFormulaVisitor eqvis {left_comps[i].get()}; 
                    if (not right_comps[i]->accept(eqvis))
                        return false;
                }
                return true;
            }
    };

    /* A visitor that tests formula strict weak ordering.
     * This is important for having sets of formulas in C++.
     * The main idea is lexycographical ordering on the components
     * of the formula.
     *
     * Strick weak ordering:
     * - irreflexive (not a < a for all a) 
     * - assymetric (if a < b then not b < a)
     * - transitive
     * - transitivity of incomparability
     * @author Vitor Greati
     */
    class StrictWeakOrderingFormulaVisitor : public FormulaConstVisitor<bool> {
        private:
            const Formula* _left; //> Left-hand side of comparison

        public:

            /* Constructor.
             * */
            StrictWeakOrderingFormulaVisitor(const decltype(_left) left) : _left {left} {};

            virtual bool visit_prop(const Prop* prop) const override {
                if (_left->type() != Formula::FmlaType::PROP)
                    return true;
                auto left_prop = dynamic_cast<const Prop*>(_left);
                return left_prop->symbol() < prop->symbol();
            }

            virtual bool visit_compound(const Compound* compound) const override {
                if (_left->type() != Formula::FmlaType::COMPOUND)
                   return false; 
                auto left = dynamic_cast<const Compound*>(_left);
                auto left_conn = left->connective();
                if (*(compound->connective()) != *(left_conn))
                    return  left_conn->symbol() < compound->connective()->symbol();
                auto left_comps = left->components();
                auto right_comps = compound->components();
                for (int i = 0; i < left_comps.size(); ++i) {
                    if ((*right_comps[i])==(*left_comps[i]))
                        continue;
                    StrictWeakOrderingFormulaVisitor eqvis {left_comps[i].get()}; 
                    return right_comps[i]->accept(eqvis);
                }
                return false;
            }
    };


    /* A visitor that collects all variables in a formula.
     * TODO: make this a const visitor
     *
     * @author Vitor Greati
     */
    class VariableCollector : public FormulaVisitor<void> {
        private:
            //< Stores the collected variables
            std::set<std::shared_ptr<Prop>, utils::DeepSharedPointerComp<Prop>> collected_variables;

        public:

            virtual void visit_prop(Prop* prop) override {
                this->collected_variables.insert(std::make_shared<Prop>(*prop));
            }

            virtual void visit_compound(Compound* compound) override {
                auto components = compound->components();
                std::for_each(components.begin(), components.end(),
                        [&](std::shared_ptr<Formula> fmla) {
                           return fmla->accept(*this); 
                        });
            }

            /* To be called after being accepted by a formula.
             */
            virtual decltype(collected_variables) get_collected_variables() {
                return this->collected_variables;
            }

            void clear() { collected_variables.clear(); };
    };

    /* A visitor that collects the least signature of a given formula, i.e.,
     * infer its signature using its connectives.
     *
     * @author Vitor Greati
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

            void clear() { collected_signature = Signature{}; };
    };

    /* A visitor that collects the subformulas of a given formula.
     */
    class SubFormulaCollector : public FormulaVisitor<void> {
        private:
            std::set<std::shared_ptr<Formula>, utils::DeepSharedPointerComp<Formula>> _subfmlas;

        public:
            void visit_prop(Prop* prop) override {
                this->_subfmlas.insert(std::make_shared<Prop>(*prop));
            }
            void visit_compound(Compound* compound) override {
                this->_subfmlas.insert(std::make_shared<Compound>(*compound));
                auto components = compound->components();
                auto connective = compound->connective();
                for (const auto& c : components) {
                    c->accept(*this);
                }
            }
            decltype(_subfmlas) subfmlas() {
                return this->_subfmlas;
            }
            void clear() { _subfmlas.clear(); };
    };

    /**
     * A visitor to print a formula.
     *
     * @author Vitor Greati
     * */
    class FormulaPrinter : public FormulaVisitor<void> {
        private:
            std::stringstream buffer; //> Stores the printed formula
        public:

            /* Empty constructor.
             */
            FormulaPrinter() {}

            virtual void visit_prop(Prop* prop) override {
                buffer << prop->symbol();
            }

            virtual void visit_compound(Compound* compound) override {
                auto arity = compound->connective()->arity();
                auto symbol = compound->connective()->symbol();

                if (arity == 0) {
                    buffer << symbol << "()";
                } else if (arity == 1 and 
                    (compound->components()[0]->type() == Formula::FmlaType::PROP or
                     compound->components()[0]->connective()->arity() == 1)) {
		    if (symbol == "neg") {
			    buffer << symbol << " ";
			    compound->components()[0]->accept(*this);
		    } else {
			    buffer << symbol << "(";
			    compound->components()[0]->accept(*this);
			    buffer << ")";
		    }
                } else if (arity == 2) {
		    if (symbol == "and" or symbol == "or" or symbol == "->") {
			    auto components = compound->components();
			    if (components[0]->type() != Formula::FmlaType::PROP)
				    buffer << "(";
			    components[0]->accept(*this);
			    if (components[0]->type() != Formula::FmlaType::PROP)
				    buffer << ")";
			    buffer << " " << symbol << " ";
			    if (components[1]->type() != Formula::FmlaType::PROP)
				    buffer << "(";
			    components[1]->accept(*this);
			    if (components[1]->type() != Formula::FmlaType::PROP)
				    buffer << ")";
		    } else {
			    auto components = compound->components();
			    buffer << symbol << "(";
			    components[0]->accept(*this);
			    buffer << ",";
			    components[1]->accept(*this);
			    buffer << ")";	    
		    }
                } else {
                    buffer << symbol << "(";
                    auto components = compound->components();
                    for (auto it = components.cbegin(); it != components.cend(); ++it) {
                        (*it)->accept(*this);
                        if (std::next(it) != components.cend())
                            buffer << ",";
                    }
                    buffer <<")";
                }
            }
            std::string get_string() { 
                std::string result = buffer.str();
                if ((result.size() >= 3) and (result[0] == '(') and (result[result.size()-1] == ')'))
                    result = result.substr(1, result.size() - 2);
                return result;
            }
    };


    /**
     * A visitor to print a formula.
     *
     * @author Vitor Greati
     * */
    class FormulaLaTeXPrinter : public FormulaVisitor<void> {
        private:
            std::stringstream buffer; //> Stores the printed formula
            std::map<std::string, std::string> translation_map; //> Translates the symbols
        public:

            /* Empty constructor.
             */
            FormulaLaTeXPrinter() {}

            /* Constructor based on the translation.
             */
            FormulaLaTeXPrinter(const decltype(translation_map)& tmap) : translation_map {tmap} {}

            /* Get the translation of a symbol, or the symbol itself if it
             * is not translatable.
             */
            std::string get_translation(const std::string& i) const {
                auto it = translation_map.find(i);
                if (it != translation_map.end())
                    return it->second;
                return i;
            }

            virtual void visit_prop(Prop* prop) override {
                buffer << get_translation(prop->symbol());
            }

	    virtual void visit_compound(Compound* compound) override {
                auto arity = compound->connective()->arity();
                auto symbol = compound->connective()->symbol();

                if (arity == 0) {
                    buffer << get_translation(symbol);
                } else if (arity == 1 and
                    (compound->components()[0]->type() == Formula::FmlaType::PROP or
                     compound->components()[0]->connective()->arity() == 1)) {
                    buffer << get_translation(symbol);
                    compound->components()[0]->accept(*this);
                } else if (arity == 2) {
                    auto components = compound->components();
		    if (components[0]->type() != Formula::FmlaType::PROP)
			    buffer << "(";
                    components[0]->accept(*this);
		    if (components[0]->type() != Formula::FmlaType::PROP)
			    buffer << ")";
                    buffer << " " << get_translation(symbol) << " ";
		    if (components[1]->type() != Formula::FmlaType::PROP)
			    buffer << "(";
                    components[1]->accept(*this);
		    if (components[1]->type() != Formula::FmlaType::PROP)
			    buffer << ")";
                } else {
                    buffer << get_translation(symbol) << "(";
                    auto components = compound->components();
                    for (auto it = components.cbegin(); it != components.cend(); ++it) {
                        (*it)->accept(*this);
                        if (std::next(it) != components.cend())
                            buffer << ",";
                    }
                    buffer <<")";
                }
            }

            std::string get_string() { 
                std::string result = buffer.str();
                if ((result.size() >= 3) and (result[0] == '(') and (result[result.size()-1] == ')'))
                    result = result.substr(1, result.size() - 2);
                return result;
            }
    };

    /* Implements an assignment of formulas to propositional variables.
     *
     * @author Vitor Greati
     * */
    class FormulaVarAssignment {
        
        private:
            //> Maps a propositional variable to a formula
            std::map<Prop, std::shared_ptr<Formula>> _assignment;

        public:

            /* Empty constructor.
             */
            FormulaVarAssignment() {}

            FormulaVarAssignment(const decltype(_assignment)& assignment)
                : _assignment {assignment} {};

            /* Equality test for assignments.
             * */
            bool operator==(const FormulaVarAssignment& other) const {
                return _assignment == other._assignment;
            }

            void set(const Prop& prop, std::shared_ptr<Formula> fmla) {
                _assignment[prop] = fmla;
            }

            std::shared_ptr<Formula>& operator[](const Prop& prop) {
                return _assignment[prop];
            }

            inline std::shared_ptr<Formula> operator()(const Prop& p) {
                return _assignment[p];
            }

            /* The number of variables being assigned.
             */
            int size() const {
                return _assignment.size();
            }

            /* Print the assignment.
             */
            std::stringstream print() const {
                std::stringstream ss;
                for (const auto& [k, v] : _assignment) {
                    ss << k.symbol() << " -> " << (*v) << std::endl;
                }
                return ss;
            }
    };

    /* Given a set of propositional variables P
     * and a set of formulas \Gamma, generate all
     * possible substitution assignments
     * of the variables in P for the formulas 
     * in \Gamma.
     *
     * @author Vitor Greati
     * */
    class FormulaVarAssignmentGenerator {
    
        private:

            std::vector<std::shared_ptr<Prop>> _props;
            FmlaSet _gamma;
            std::vector<FmlaSet::iterator> _current_iterators;
            std::shared_ptr<FormulaVarAssignment> _current;
            bool finished = false;
            bool first = true;
            int qtd_in_max = 0;

            void initialize_iterators() {
                _current_iterators.clear();
                for (int i = 0; i < _props.size(); ++i) {
                    _current_iterators.push_back(_gamma.begin());
                    if (_current_iterators[i] == _gamma.end() or std::next(_current_iterators[i]) == _gamma.end())
                        qtd_in_max++;
                    if (_current_iterators[i] != _gamma.end())
                        _current->set(*_props[i], *_current_iterators[i]);
                }
            }

        public:

            FormulaVarAssignmentGenerator() {};
            FormulaVarAssignmentGenerator(const PropSet& props,
                    const decltype(_gamma)& gamma)
                : _gamma {gamma} {
                _props = decltype(_props){props.begin(), props.end()};
                reset();    
            }

            FormulaVarAssignmentGenerator(const decltype(_props)& props,
                    const decltype(_gamma)& gamma)
                : _props {props}, _gamma {gamma} {
                _props = decltype(_props){props.begin(), props.end()};
                reset();    
            }

            decltype(_current) next() {
                if (not has_next()) {
                    throw std::logic_error("no substitutions to generate");
                }
                if (not first) {
                    int i = 0;
                    for (i = _props.size() - 1; i >= 0; --i) {
                        if (std::next(_current_iterators[i]) == _gamma.end()) {
                            _current_iterators[i] = _gamma.begin(); 
                            _current->set(*_props[i], *_current_iterators[i]);
                            qtd_in_max--;
                        } else {
                            ++_current_iterators[i];
                            _current->set(*_props[i], *_current_iterators[i]);
                            if (std::next(_current_iterators[i]) == _gamma.end())
                                qtd_in_max++;
                            break;
                        } 
                    }
                    if (qtd_in_max == _props.size())
                        finished = true;
                } else {
                    first = false;
                    if (qtd_in_max == _props.size())
                        finished = true;
                }
                assert(_current->size() == _props.size());
                return _current; 
            }

            bool has_next() const {
                return not finished and not _gamma.empty();
            }

            void reset() {
                qtd_in_max = 0;
                _current = std::make_shared<FormulaVarAssignment>();
                initialize_iterators();
                finished = false or _props.empty() or _gamma.empty();
                first = not finished;
            }
    
    };

    /* Given an assignment of formulas to
     * variables, apply this substitution to
     * a given formula.
     *
     * @author Vitor Greati
     * */
    class SubstitutionEvaluator : public FormulaVisitor<std::shared_ptr<Formula>> {
    
        private:
            FormulaVarAssignment _assignment; //> The assignment to be applied

        public:

            SubstitutionEvaluator(const decltype(_assignment)& assignment)
                : _assignment {assignment} {}

            std::shared_ptr<Formula> visit_prop(Prop* prop) override {
                return _assignment(*prop);
            }

            std::shared_ptr<Formula> visit_compound(Compound* compound) override {
                auto components = compound->components();
                auto connective = compound->connective();
                std::vector<std::shared_ptr<Formula>> new_components;
                for (const auto& c : components) {
                    auto cn = c->accept(*this); 
                    new_components.push_back(cn);
                }
                return std::make_shared<Compound>(connective, new_components);
            }

    };

    /* Check if a set of formulas is a subset of another.
     */
    bool is_subset(const FmlaSet& f1, const FmlaSet& f2);

}
#endif
