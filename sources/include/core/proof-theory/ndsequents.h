#ifndef __N_DIM_SEQUENTS__
#define __N_DIM_SEQUENTS__

#include <vector>
#include <set>
#include <unordered_set>
#include "core/syntax.h"

namespace ltsy {

    /**
     * Represents an N-Dimensional sequent.
     * */
    template<template<class...> typename FmlaContainerT>
    class NdSequent {

        private:
            std::vector<FmlaContainerT<std::shared_ptr<Formula>>> _sequent_fmlas;
            size_t _dimension;

        public:

            NdSequent(const decltype(_sequent_fmlas)& sequent_fmlas) : _sequent_fmlas {sequent_fmlas}
            {
                _dimension = sequent_fmlas.size();
            }

            NdSequent(size_t dimension, const FmlaContainerT<std::shared_ptr<Formula>>& def) : _dimension {dimension} {
                _sequent_fmlas = decltype(_sequent_fmlas) {_dimension, def};
            }

            inline int dimension() const { return _dimension; }

            FmlaContainerT<std::shared_ptr<Formula>>& operator[](int i) { return _sequent_fmlas[i]; }

            FmlaContainerT<std::shared_ptr<Formula>> operator[](int i) const { return _sequent_fmlas[i]; }

            /* Collect the propositional variables that appear in the
             * formulas present in the sequent.
             * */
            std::set<std::shared_ptr<Prop>> collect_props() const {
                std::set<Prop*, utils::DeepPointerComp<Prop>> collected_variables;
                for (const auto& fmla_set : _sequent_fmlas) {
                    for (const auto& fmla : fmla_set) {
                        VariableCollector var_collector;
                        fmla->accept(var_collector);
                        auto collected_vars = var_collector.get_collected_variables();
                        collected_variables.insert(collected_vars.begin(), collected_vars.end());
                    }
                } 
                std::set<std::shared_ptr<Prop>> result;
                for (auto p : collected_variables)
                    result.insert(std::make_shared<Prop>(*p));
                return result;
            };

            bool is_in(int i, Formula& fmla) const {
                for (auto fpointer : _sequent_fmlas[i]){
                    EqualityFormulaVisitor eq {fpointer};
                    if (fmla.accept(eq))
                        return true;
                }
                return false; 
            }

            std::string to_string() const {
                std::stringstream os;
                auto& seq = *this;
                os << std::string("[");
                for (auto i = 0; i < seq._sequent_fmlas.size(); ++i) {
                    os << std::string("[");
                    for (auto it = seq._sequent_fmlas[i].begin(); it != seq._sequent_fmlas[i].end(); ++it) {
                        os << (*(*it));
                        if (std::next(it) != seq._sequent_fmlas[i].end())
                            os << ", ";
                    }
                    os << std::string("]");
                    if (i < seq._sequent_fmlas.size() - 1)
                        os << std::string(", ");
                }
                os << std::string("]");
                return os.str();           
            }

            friend std::ostream& operator<<(std::ostream& os, const NdSequent<FmlaContainerT>& seq) {
                os << std::string("[ ");
                for (auto i = 0; i < seq._sequent_fmlas.size(); ++i) {
                    for (auto it = seq._sequent_fmlas[i].begin(); it != seq._sequent_fmlas[i].end(); ++it) {
                        os << (*(*it));
                        if (std::next(it) != seq._sequent_fmlas[i].end())
                            os << ",";
                    }
                    if (i < seq._sequent_fmlas.size() - 1)
                        os << std::string(" | ");
                }
                os << std::string(" ]");
                return os; 
            }
    };

    template class NdSequent<std::vector>;
    template class NdSequent<std::set>;
    template class NdSequent<std::unordered_set>;

};
#endif
