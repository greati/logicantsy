#ifndef __NMATRICES__
#define __NMATRICES__

#include "core/syntax.h"
#include "core/semantics/truth_tables.h"

namespace ltsy {

    /**
     * Abstracts a truth interpretation of a connective.
     * */
    class TruthInterp {
        protected:
            std::shared_ptr<Connective> _connective_ptr;
            int _max_truth_value;
        public:
            TruthInterp (const decltype(_connective_ptr)& _connective_ptr, int _max_truth_value = 2)
                : _connective_ptr {_connective_ptr}, _max_truth_value {_max_truth_value} {/*empty*/}
            inline const decltype(_connective_ptr)& connective_ptr() const { return _connective_ptr; }
    };

    /**
     * Holds an interpretation of a constant.
     * */
    class ConstantTruthInterp : public TruthInterp {
        private:
            int _value;
        public:
            ConstantTruthInterp (int _value) : TruthInterp {_connective_ptr}, _value {_value} {
                if (_value < 0 or _value >= this->_max_truth_value)
                    throw std::invalid_argument(INVALID_TRUTH_VALUE_EXCEPTION);
            }
    };

    /**
     * Holds the interpreation relation between a connective
     * and a truth table.
     *
     * Main purpose is to guarantee that this relation is
     * well set up.
     * */
    class ConnectiveTruthInterp : public TruthInterp {
        private:
            std::shared_ptr<TruthTable> _truth_table_ptr;
        public:
            ConnectiveTruthInterp(
                    const decltype(_connective_ptr)& _connective_ptr,
                    const decltype(_truth_table_ptr)& _truth_table_ptr) 
            : TruthInterp {_connective_ptr}, _truth_table_ptr {_truth_table_ptr} {
                if (_connective_ptr == nullptr or _truth_table_ptr == nullptr)
                    throw std::invalid_argument(NULL_IN_CONNECTIVE_INTERP_EXCEPTION);
                if (_connective_ptr->arity() != _truth_table_ptr->arity())
                    throw std::invalid_argument(INVALID_CONNECTIVE_INTERP_EXCEPTION);
                if (_truth_table_ptr->nvalues() != this->_max_truth_value)
                    throw std::invalid_argument(INVALID_TRUTH_VALUE_EXCEPTION);
            }
    };

    class SignatureTruthInterp {
        private:
            std::shared_ptr<Signature> _signature;
            std::map<Symbol, std::shared_ptr<TruthInterp>> _truth_interps;
        public:
            SignatureTruthInterp(const decltype(_signature)& _signature)
                : _signature {_signature} {/* empty */}

            void try_interpret(const std::shared_ptr<TruthInterp>& truth_interp) {
                auto connective_symbol = truth_interp->connective_ptr()->symbol();
                try {
                    auto connective_in_sig_symbol = (*_signature)[connective_symbol].symbol();
                    auto it = _truth_interps.find(connective_in_sig_symbol);
                    if (it == _truth_interps.end())
                        _truth_interps.insert({connective_in_sig_symbol, truth_interp});
                    else
                        throw std::invalid_argument(CONNECTIVE_ALREADY_INTERP_EXCEPTION);
                } catch (ConnectiveNotPresentException e) {
                    throw; 
                }
            }
    };


    class NMatrix {
        
        private:

            int _nvalues;

            int _dvalues;

            std::shared_ptr<Signature> _signature;

            std::shared_ptr<SignatureTruthInterp> _interpretation;

        public:

            NMatrix(int _nvalues, int _dvalues, const decltype(_signature)& _signature,
                    const decltype(_interpretation)& _interpretation)
                : _nvalues {_nvalues}, _dvalues {_dvalues}, _signature {_signature}, _interpretation {_interpretation} {
            }

            inline int nvalues() const { return _nvalues; }
    };

    class NMatrixValuation {
        private:

            std::shared_ptr<NMatrix> _nmatrix_ptr;
            std::map<Prop, int> _valuation_map;
            
        public:
            
            NMatrixValuation(const decltype(_nmatrix_ptr)& _nmatrix_ptr,
                    const std::vector<std::pair<Prop, int>> mappings)
                : _nmatrix_ptr {_nmatrix_ptr} {

                int nvalues = _nmatrix_ptr->nvalues();
               
                for (auto& mapping : mappings) {
                    auto [p, v] = mapping;
                    if (v >= 0 and v < nvalues)
                        _valuation_map.insert(mapping);
                    else
                        throw std::invalid_argument(INVALID_TRUTH_VALUE_EXCEPTION);
                } 
            }
            
            int operator()(const Prop& p) {
                //TODO check
                return _valuation_map[p];
            }
    };

    class NMatrixEvaluator : public FormulaVisitor<int> {
        private:
            std::shared_ptr<NMatrixValuation> _nmatrix_valuation_ptr;
        public:
            int visit_prop(Prop* prop) override {
                if (prop != nullptr) {
                    return (*_nmatrix_valuation_ptr)(*prop);
                }
            }
            int visit_compound(Compound* compound) override {
                if (compound != nullptr) {
                    for (auto& comp : compound->components()) {}
                }
            }
    };

};

#endif
