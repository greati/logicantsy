#ifndef __NMATRICES__
#define __NMATRICES__

#include "core/syntax.h"
#include "core/semantics/truth_tables.h"
#include <set>

namespace ltsy {

    /**
     * Abstracts a truth interpretation of a connective.
     * */
    class TruthInterp {
        protected:
            std::shared_ptr<Connective> _connective_ptr;
        public:
            TruthInterp (decltype(_connective_ptr) _connective_ptr)
                : _connective_ptr {_connective_ptr}{/*empty*/}

            inline decltype(_connective_ptr) connective_ptr() const { return _connective_ptr; }

            virtual int at(const std::vector<int>& args) const = 0;
    };

    /**
     * Holds an interpretation of a constant.
     * */
    template<typename CellType = int>
    class ConstantTruthInterp : public TruthInterp {
        private:
            CellType _value;
        public:
            ConstantTruthInterp (decltype(_connective_ptr) _connective_ptr, CellType _value) 
                : TruthInterp {_connective_ptr}, _value {_value} {
            }

            inline int at(const std::vector<int>& args) const override {
                return _value;
            }
    };

    /**
     * Holds the interpreation relation between a connective
     * and a truth table.
     *
     * Main purpose is to guarantee that this relation is
     * well set up.
     * */
    template<typename CellType = int>
    class ConnectiveTruthInterp : public TruthInterp {
        private:
            std::shared_ptr<TruthTable<CellType>> _truth_table_ptr;
        public:
            ConnectiveTruthInterp(
                    decltype(_connective_ptr) _connective_ptr,
                    decltype(_truth_table_ptr) _truth_table_ptr) 
            : TruthInterp {_connective_ptr}, _truth_table_ptr {_truth_table_ptr} {
                if (_connective_ptr == nullptr or _truth_table_ptr == nullptr)
                    throw std::invalid_argument(NULL_IN_CONNECTIVE_INTERP_EXCEPTION);
                if (_connective_ptr->arity() != _truth_table_ptr->arity())
                    throw std::invalid_argument(INVALID_CONNECTIVE_INTERP_EXCEPTION);
            }

            inline int at(const std::vector<int>& args) const override {
                return _truth_table_ptr->at(args);
            }

    };

    /**
     * Holds the truth interpretation of a whole signature.
     *
     * @author Vitor Greati
     * */
    class SignatureTruthInterp {
        private:
            std::shared_ptr<Signature> _signature;
            std::map<Symbol, std::shared_ptr<TruthInterp>> _truth_interps;
        public:
            SignatureTruthInterp(decltype(_signature) _signature)
                : _signature {_signature} {/* empty */}

            SignatureTruthInterp(decltype(_signature) _signature, 
                    std::initializer_list<std::shared_ptr<TruthInterp>> _interps)
                : _signature {_signature} {
                for (auto& ti : _interps)
                    this->try_interpret(ti);
            }

            void try_interpret(std::shared_ptr<TruthInterp> truth_interp) {
                auto connective_symbol = truth_interp->connective_ptr()->symbol();
                try {
                    auto connective_in_sig_symbol = (*_signature)[connective_symbol]->symbol();
                    auto it = _truth_interps.find(connective_in_sig_symbol);
                    if (it == _truth_interps.end())
                        _truth_interps.insert({connective_in_sig_symbol, truth_interp});
                    else
                        throw std::invalid_argument(CONNECTIVE_ALREADY_INTERP_EXCEPTION);
                } catch (ConnectiveNotPresentException e) {
                    throw; 
                }
            }

            std::shared_ptr<TruthInterp> get_interpretation(const Symbol& symbol) const {
                return this->_truth_interps.at(symbol);
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
            std::shared_ptr<SignatureTruthInterp> _interpretation;

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
