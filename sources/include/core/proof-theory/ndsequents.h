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
            std::vector<FmlaContainerT<std::shared_ptr<Formula>, utils::DeepSharedPointerComp<Formula>>> _sequent_fmlas;
            size_t _dimension;

        public:

            NdSequent(size_t dimension) : _dimension {dimension} {
                for (auto i = 0; i < dimension; ++i)
                    _sequent_fmlas.push_back({});
            }

            NdSequent(const decltype(_sequent_fmlas)& sequent_fmlas) : _sequent_fmlas {sequent_fmlas}
            {
                _dimension = sequent_fmlas.size();
            }

            NdSequent(size_t dimension, const FmlaContainerT<std::shared_ptr<Formula>, utils::DeepSharedPointerComp<Formula>>& def) 
                : _dimension {dimension} {
                _sequent_fmlas = decltype(_sequent_fmlas) {_dimension, def};
                _dimension = _sequent_fmlas.size();
            }

            inline int dimension() const { return _dimension; }
            inline decltype(_sequent_fmlas) sequent_fmlas() const { return _sequent_fmlas; }

            FmlaContainerT<std::shared_ptr<Formula>, utils::DeepSharedPointerComp<Formula>>& operator[](int i) { return _sequent_fmlas[i]; }

            FmlaContainerT<std::shared_ptr<Formula>, utils::DeepSharedPointerComp<Formula>> operator[](int i) const { return _sequent_fmlas[i]; }

            const FmlaContainerT<std::shared_ptr<Formula>, utils::DeepSharedPointerComp<Formula>>& at(int i) const { return _sequent_fmlas[i]; }

            /* Collect the propositional variables that appear in the
             * formulas present in the sequent.
             * */
            PropSet collect_props() const {
                PropSet collected_variables;
                for (const auto& fmla_set : _sequent_fmlas) {
                    for (const auto& fmla : fmla_set) {
                        VariableCollector var_collector;
                        fmla->accept(var_collector);
                        auto collected_vars = var_collector.get_collected_variables();
                        collected_variables.insert(collected_vars.begin(), collected_vars.end());
                    }
                } 
                return collected_variables;
            };

            bool is_in(int i, Formula& fmla) const {
                for (auto fpointer : _sequent_fmlas[i]){
                    EqualityFormulaVisitor eq {fpointer.get()};
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

            /* Infer a signature from the formulas present
             * in the sequent.
             *
             * @return a signature having connectives appearing in the
             * sequent
             * */
            Signature infer_signature() const {
                SignatureCollector collector;
                for (const auto& fmls : _sequent_fmlas) {
                    for (const auto& fmla : fmls) {
                        fmla->accept(collector);
                    }      
                }
                return collector.get_collected_signature();
            }

            FmlaSet collect_fmlas() const {
                FmlaSet fmlaset;
                for (const auto& seqpos : _sequent_fmlas) {
                    for (const auto& fm : seqpos) {
                        fmlaset.insert(fm); 
                    }
                }
                return fmlaset;
            }


            bool operator==(const NdSequent<FmlaContainerT>& other) const {
                for (int i = 0; i < _sequent_fmlas.size(); ++i) {
                    auto other_fmlas = other.sequent_fmlas()[i];        
                    auto current_fmlas = _sequent_fmlas[i];        
                    if (other_fmlas != current_fmlas)
                        return false;
                }
                return true;
            }

            bool is_dilution_of(const NdSequent<FmlaContainerT>& other) const {
                for (int i = 0; i < _sequent_fmlas.size(); ++i) {
                    auto other_fmlas = other.sequent_fmlas()[i];        
                    auto current_fmlas = _sequent_fmlas[i];        
                    for (const auto& f : other_fmlas) {
                        if (current_fmlas.find(f) == current_fmlas.end())
                            return false;
                    }
                }
                return true;
            }

            void set(int i, FmlaContainerT<std::shared_ptr<Formula>, utils::DeepSharedPointerComp<Formula>> container) {
                _sequent_fmlas[i] = container;
            }

            NdSequent<FmlaContainerT> apply_substitution(const FormulaVarAssignment& ass) const {
                std::vector<FmlaContainerT<std::shared_ptr<Formula>, utils::DeepSharedPointerComp<Formula>>> res_sequent_fmlas;
                for (auto i {0}; i < _sequent_fmlas.size(); ++i) {
                    FmlaContainerT<std::shared_ptr<Formula>, utils::DeepSharedPointerComp<Formula>> container;
                    for (const auto& fm : _sequent_fmlas[i]) {
                        SubstitutionEvaluator seval {ass};
                        auto sres = fm->accept(seval);
                        container.insert(container.end(), sres); 
                    }
                    res_sequent_fmlas.push_back(container);
                }
                return NdSequent<FmlaContainerT>{res_sequent_fmlas};
            }

            NdSequent<FmlaContainerT> transform(const std::vector<std::pair<int, int>>& transformation,
                std::function<bool(std::shared_ptr<Formula>)> pred = [](auto f) -> bool { return true; }) const {
                NdSequent<FmlaContainerT> result {*this};
                for (const auto [i,j] : transformation) {
                    auto oldi = result._sequent_fmlas[i];
                    auto oldj = result._sequent_fmlas[j];
                    for (auto fmla : oldi) {
                        if (pred(fmla)) {
                            result._sequent_fmlas[i].erase(fmla);
                            result._sequent_fmlas[j].insert(fmla);
                        }
                    }
                    for (auto fmla : oldj) {
                        if (pred(fmla)) {
                            result._sequent_fmlas[j].erase(fmla);
                            result._sequent_fmlas[i].insert(fmla);
                        }
                    }
                }
                return result;
            }
    };

    template class NdSequent<std::set>;

    /* NdSequent-to-NdSequent bidimensional rule representation.
     * It contains a set of NdSequent-premises,
     * and a set of NdSequent-conclusions.
     * */
    template<template<class...> typename FmlaContainerT>
    class NdSequentRule {
    
        private:
            std::vector<NdSequent<FmlaContainerT>> _premises;
            std::vector<NdSequent<FmlaContainerT>> _conclusions;
            std::string _name;

        public:

            NdSequentRule(const std::string& name, const decltype(_premises)& premises,
                    const decltype(_conclusions)& conclusions) : 
                _name {name},
                _premises {premises}, _conclusions {conclusions} {/* empty */}

            NdSequentRule(const decltype(_premises)& premises,
                    const decltype(_conclusions)& conclusions) :
                _premises {premises}, _conclusions {conclusions} {/* empty */}

            inline decltype(_premises) premises() const { return _premises; };
            inline decltype(_conclusions) conclusions() const { return _conclusions; };
            inline decltype(_name) name() const { return _name; };

            std::set<std::shared_ptr<Prop>, utils::DeepSharedPointerComp<Prop>> collect_props() const {
                std::set<std::shared_ptr<Prop>, utils::DeepSharedPointerComp<Prop>> props;
                for (const auto& p : _premises) {
                    auto collected_props = p.collect_props();
                    props.insert(collected_props.begin(), collected_props.end());
                }
                for (const auto& c : _conclusions) {
                    auto collected_props = c.collect_props();
                    props.insert(collected_props.begin(), collected_props.end());
                }
                //std::set<std::shared_ptr<Prop>> result {props.begin(), props.end()};
                return props;
            }

            /* Infer a signature from the formulas present
             * in the rule.
             *
             * @return a signature having connectives appearing in the
             * rule
             * */
            Signature infer_signature() const {
                Signature sig;
                for (const auto& p : _premises)
                    sig.join(p.infer_signature());
                for (const auto& c : _conclusions) 
                    sig.join(c.infer_signature());
                return sig;
            }

    };

    template class NdSequentRule<std::set>;

};
#endif
