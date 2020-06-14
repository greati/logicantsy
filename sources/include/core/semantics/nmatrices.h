#ifndef __NMATRICES__
#define __NMATRICES__

#include "core/syntax.h"
#include "core/semantics/truth_tables.h"
#include <set>

namespace ltsy {

    /**
     * Holds the interpreation relation between a connective
     * and a truth table.
     *
     * Main purpose is to guarantee that this relation is
     * well set up.
     * */
    template<typename CellType = int>
    class TruthInterp {
        private:
            std::shared_ptr<Connective> _connective;
            std::shared_ptr<TruthTable<CellType>> _truth_table;
        public:
            TruthInterp(
                    decltype(_connective) connective,
                    decltype(_truth_table) truth_table) 
            : _connective {connective}, _truth_table {truth_table} {
                if (_connective == nullptr or _truth_table == nullptr)
                    throw std::invalid_argument(NULL_IN_CONNECTIVE_INTERP_EXCEPTION);
                if (_connective->arity() != _truth_table->arity())
                    throw std::invalid_argument(INVALID_CONNECTIVE_INTERP_EXCEPTION);
            }

            inline decltype(_connective) connective() const { return _connective; }

            inline int at(const std::vector<int>& args) const {
                return _truth_table->at(args);
            }

            friend std::ostream& operator<<(std::ostream& os, const TruthInterp<CellType>& ti) {
                os << ti._connective->symbol();
                os << std::string(":") << std::endl;
                os << (*ti._truth_table);
                return os;  
            }
    };

    class TruthInterpGenerator {
    
        private:

            std::shared_ptr<Connective> _connective;
            int _nvalues;
            TruthTableGenerator _ttgen;
            std::shared_ptr<TruthInterp<int>> _current;

        public:

            TruthInterpGenerator(int nvalues, decltype(_connective) connective)
            : _connective {connective}, _nvalues {nvalues} {
                _ttgen = TruthTableGenerator{_nvalues, _connective->arity()};
                reset();
            };

            decltype(_current) next() {
                if (not has_next())
                    throw std::logic_error("no next truth interpretation to produce");
                auto tt = _ttgen.next();
                _current = std::make_shared<TruthInterp<int>>(_connective, tt);
                return _current;
            }

            bool has_next() {
                return _ttgen.has_next();
            }

            void reset() {
                _ttgen.reset();
                _current = std::make_shared<TruthInterp<int>>(_connective, _ttgen.current());
            }

            decltype(_current) current() const { return _current; }
    };

    /**
     * Holds the truth interpretation of a whole signature.
     *
     * @author Vitor Greati
     * */
    template<typename CellType>
    class SignatureTruthInterp {
        private:
            std::shared_ptr<Signature> _signature;
            std::map<Symbol, std::shared_ptr<TruthInterp<CellType>>> _truth_interps;
        public:
            SignatureTruthInterp(decltype(_signature) _signature)
                : _signature {_signature} {/* empty */}

            SignatureTruthInterp(decltype(_signature) _signature, 
                    std::initializer_list<std::shared_ptr<TruthInterp<CellType>>> _interps)
                : _signature {_signature} {
                for (auto& ti : _interps)
                    this->try_interpret(ti);
            }

            void try_interpret(std::shared_ptr<TruthInterp<CellType>> truth_interp, bool allow_overwrite=false) {
                auto connective_symbol = truth_interp->connective()->symbol();
                try {
                    auto connective_in_sig_symbol = (*_signature)[connective_symbol]->symbol();
                    auto it = _truth_interps.find(connective_in_sig_symbol);
                    if (allow_overwrite) {
                        _truth_interps[connective_in_sig_symbol] = truth_interp;
                    } else {
                        if (it == _truth_interps.end())
                            _truth_interps.insert({connective_in_sig_symbol, truth_interp});
                        else
                            throw std::invalid_argument(CONNECTIVE_ALREADY_INTERP_EXCEPTION);
                    }
                } catch (ConnectiveNotPresentException e) {
                    throw; 
                }
            }

            std::shared_ptr<TruthInterp<CellType>> get_interpretation(const Symbol& symbol) const {
                return this->_truth_interps.at(symbol);
            }

            friend std::ostream& operator<<(std::ostream& os, const SignatureTruthInterp<CellType>& ti) {
                for(auto& [s, t] : ti._truth_interps) {
                    os << (*t) << std::endl;
                }
                return os;  
            }
    };

    class SignatureTruthInterpGenerator {

        private:

            std::shared_ptr<Signature> _signature;
            std::map<Symbol, TruthInterpGenerator> _generators;
            int _nvalues;
            std::shared_ptr<SignatureTruthInterp<int>> _current;

            void initialize_generators() {
                for (auto [symbol, connective] : (*_signature)) {
                    std::cout << "generators for " << symbol << std::endl;
                    _generators.insert({symbol, TruthInterpGenerator(this->_nvalues, connective)}); 
                } 
            }
            
            std::shared_ptr<SignatureTruthInterp<int>> truth_interp_from_generators() {
                auto sti = std::make_shared<SignatureTruthInterp<int>>(_signature);
                for (auto& [symbol, gen] : _generators) {
                    std::cout << "interpreting " << symbol << std::endl;
                    sti->try_interpret(gen.next());
                }
                (*(_generators.begin())).second.reset();
                return sti;
            }
    
        public:

            SignatureTruthInterpGenerator(int nvalues, decltype(_signature) signature) 
                : _signature {signature}, _nvalues {nvalues} {
                initialize_generators();    
                reset();
            }

            decltype(_current) next() {
                if (not has_next())
                    throw std::logic_error("no truth interpretation available");

                for (auto& [symbol, generator] : _generators) {
                    if (generator.has_next()) {
                        auto tt = generator.next();
                        _current->try_interpret(tt, true);
                        break;
                    } 
                    generator.reset();
                    _current->try_interpret(generator.next(), true);
                }
                return _current;
            }

            decltype(_current) current() { return _current; }

            void reset() {
                for (auto& [s, g] : _generators)
                    g.reset();
                _current = truth_interp_from_generators();
            }

            bool has_next() {
                for (auto& [s, g] : _generators) {
                    if (g.has_next()) 
                        return true;
                }
                return false;
            }
    
    };


    /**
     * Represents an logical matrix.
     *
     * @author Vitor Greati
     * */
    class NMatrix {
        
        private:

            int _nvalues;
            std::set<int> _dvalues;
            std::shared_ptr<Signature> _signature;
            std::shared_ptr<SignatureTruthInterp<int>> _interpretation;

        public:

            NMatrix(int _nvalues, decltype(_dvalues), decltype(_signature) _signature,
                    decltype(_interpretation) _interpretation)
                : _nvalues {_nvalues}, _dvalues {_dvalues}, 
                _signature {_signature}, _interpretation {_interpretation} {
            }

            inline int nvalues() const { return _nvalues; }

            inline decltype(_dvalues) dvalues() const { return _dvalues; }

            inline decltype(_interpretation) interpretation() const {
                return _interpretation;
            }
    };

    /**
     * A valuation over an NMatrix, determined
     * by assignments to each propositional variable.
     *
     * @author Vitor Greati
     * */
    class NMatrixValuation {
        private:

            std::shared_ptr<NMatrix> _nmatrix_ptr;
            std::map<Prop, int> _valuation_map;
            
        public:
            
            NMatrixValuation(decltype(_nmatrix_ptr) nmatrix_ptr,
                    const std::vector<std::pair<Prop, int>>& mappings)
                {

                std::atomic_store(&(this->_nmatrix_ptr), std::move(nmatrix_ptr));

                int nvalues = _nmatrix_ptr->nvalues();
               
                for (auto& mapping : mappings) {
                    auto [p, v] = mapping;
                    if (v >= 0 and v < nvalues) {
                        _valuation_map.insert(mapping);
                    } else {
                        std::cout << v;
                        throw std::invalid_argument(INVALID_TRUTH_VALUE_EXCEPTION);
                    }
                } 
            }

            std::stringstream print() {
                std::stringstream ss;
                for (auto& [k, v] : _valuation_map) {
                    ss << k.symbol() << " -> " << v << std::endl;
                }
                return ss;
            }

            inline const decltype(_nmatrix_ptr) nmatrix_ptr() const {
                return _nmatrix_ptr;
            }
            
            inline int operator()(const Prop& p) {
                return _valuation_map[p];
            }
    };

    /* Based on a valuation, visit a formula to
     * determine its truth value (it is the 
     * unique homomorphic extension of the 
     * given valuation).
     *
     * @author Vitor Greati
     * */
    class NMatrixEvaluator : public FormulaVisitor<int> {
        private:
            std::shared_ptr<NMatrixValuation> _nmatrix_valuation_ptr;

        public:

            NMatrixEvaluator(decltype(_nmatrix_valuation_ptr) nmatrix_valuation_ptr) :
                _nmatrix_valuation_ptr {nmatrix_valuation_ptr} {/* empty */}

            int visit_prop(Prop* prop) override {
                if (prop != nullptr) {
                    return (*_nmatrix_valuation_ptr)(*prop);
                } else throw std::logic_error("proposition points to null");
            }

            int visit_compound(Compound* compound) override {
                if (compound != nullptr) {
                    auto connective = compound->connective();
                    auto conn_interp = 
                        _nmatrix_valuation_ptr->nmatrix_ptr()->interpretation()
                            ->get_interpretation(connective->symbol());
                    auto components = compound->components();
                    std::vector<int> args;
                    for (auto component : components) {
                        return component->accept(*this);
                    }
                    return conn_interp->at(args);
                } else throw std::logic_error("compound points to null");
            }
    };

    /**
     * Generator of NMatrix valuations.
     * */
    class NMatrixValuationGenerator {
        
        private:
            std::shared_ptr<NMatrix> _nmatrix; 
            std::vector<Prop> _props;
            int _current_index = 0;
            int _total_valuations;

        public:

            NMatrixValuationGenerator(decltype(_nmatrix) nmatrix, decltype(_props) props) : _props {props} {
                std::atomic_store(&_nmatrix, nmatrix);
                auto number_props = _props.size();
                auto nvalues = _nmatrix->nvalues();
                _total_valuations = std::pow(nvalues, number_props); 
            }

            bool has_next() {
                return _current_index < _total_valuations;
            }

            NMatrixValuation next() {
                if (not has_next())
                    throw std::logic_error("no valuations to generate");
                auto images = utils::tuple_from_position(_nmatrix->nvalues(), _props.size(), _current_index);
                std::vector<std::pair<Prop, int>> val_map;
                for (int i = 0; i < _props.size(); ++i)
                    val_map.push_back({_props[i], images[i]});
                ++_current_index;
                return NMatrixValuation(std::atomic_load(&_nmatrix), val_map); 
            }
    };

};

#endif
