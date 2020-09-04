#ifndef __NDSEQ_NORMAL_FORM__
#define __NDSEQ_NORMAL_FORM__

#include "core/syntax.h"
#include "core/semantics/truth_tables.h"
#include "core/semantics/attitudes.h"
#include "core/proof-theory/ndsequents.h"
#include "core/utils.h"

namespace ltsy {

    /**
     * General correspondence between values and judgements.
     * */
    class JudgementValueCorrespondence {
        public:
            virtual std::set<CognitiveAttitude> judgements_from_value(int) = 0;
            virtual int value_from_judgements(const std::set<CognitiveAttitude>&) = 0;

            /**
             * Produces a set of lists of judgements. Each list
             * represent a configuration of a counter-model
             * in terms of judgements.
             * */
            virtual std::vector<std::set<CognitiveAttitude>> 
                judgements_from_set_values(const std::set<int>&) = 0;
    };


    /**
     * Gives a judgement-value correspondence given
     * when they are represented in a billatice,
     * the judgements occuppying the edges of the
     * underlying graph.
     *
     * For example:
     *
     * {
     *    {
     *     0, {{1, N}, {2, Y}},
     *     1, {{0, N}, {3, nY}},
     *     2, {{0, Y}, {3, nN}},
     *     3, {{1, nY}, {2, nN}}
     *    }
     * }
     *
     * */
    class BillaticeJudgementValueCorrespondence : public JudgementValueCorrespondence {

        private:

            std::map<int, std::vector<std::pair<int, CognitiveAttitude>>> _adjs;

            std::set<CognitiveAttitude> _compute_judgs_from_induced_subgraph(const std::set<int>& vals) {
                std::set<CognitiveAttitude> result;
                for (const auto& v : vals)
                    for (const auto& [u, j] : _adjs[v])
                        if (vals.find(u) != vals.end())
                            result.insert(j);
                return result;
            }

        public:

            BillaticeJudgementValueCorrespondence(const decltype(_adjs)& adjs) : _adjs {adjs} {}

            std::set<CognitiveAttitude> judgements_from_value(int v) override {
                std::set<CognitiveAttitude> atts;
                for (auto& ar : _adjs[v])
                    atts.insert(ar.second);
                return atts;
            }

            int value_from_judgements(const std::set<CognitiveAttitude>& judgs) override {
                for (const auto& [v, adjlist] : _adjs) {
                    if (judgements_from_value(v) == judgs)
                       return v; 
                } 
                throw std::logic_error("no value corresponds to those judgements");
            }

            std::vector<std::set<CognitiveAttitude>> judgements_from_set_values(const std::set<int>& vals) override {
                std::vector<std::set<CognitiveAttitude>> atts;
                auto vals_size = vals.size(); 
                // no values
                if (vals_size == 0)
                    return atts;
                // induced subgraph
                std::set<CognitiveAttitude> inducedSubgraph = this->_compute_judgs_from_induced_subgraph(vals);;
                if (inducedSubgraph.size() == 0) {
                    for (const auto& v : vals) {
                        auto js = judgements_from_value(v);
                        atts.push_back(js);
                    }
                } else {
                    for (const auto& a : inducedSubgraph)
                        atts.push_back({a});
                }
                return atts;
            }
    };


    /**
     * Produce a set of sequents that determinize
     * a truth table, according to notions
     * of cognitive attitudes.
     *
     * @author Vitor Greati
     * */
    class NdSequentsFromTruthTable {

        private:

            size_t _sequent_dim = 4;
            std::set<int> _all_values;
            Connective _connective;
            std::vector<std::shared_ptr<Prop>> _props;
            std::shared_ptr<Compound> _compound;
            std::map<CognitiveAttitude, int> _judg_position;
            std::vector<CognitiveAttitude> _judgements;
            std::shared_ptr<JudgementValueCorrespondence> _judg_value_correspondence;

        public:

            NdSequentsFromTruthTable(const decltype(_all_values)& all_values,
                    const decltype(_connective)& connective,
                    const decltype(_props)& props,
                    const decltype(_compound)& compound,
                    const decltype(_judgements)& judgements,
                    const decltype(_judg_value_correspondence)& judg_value_correspondence)
                : _all_values {all_values}, _props {props}, _compound {compound},
                _judgements {judgements}, _judg_value_correspondence {judg_value_correspondence} 
            {
                for (size_t i {0}; i < _judgements.size(); ++i)
                    _judg_position[_judgements[i]] = i;
            }


            std::vector<NdSequent<std::set>> axiomatize(const NDTruthTable& table) {
                std::vector<NdSequent<std::set>> sequents;
                for (const auto& d : table.get_determinants()) {
                   sequents_from_determinants(d, sequents); 
                } 
                return sequents;
            } 

        private:

            void sequents_from_determinants(const Determinant<std::set<int>>& det,
                    std::vector<NdSequent<std::set>>& sequents) {
                auto response_set = det.get_last(); 
                auto avoid_set = utils::set_difference(this->_all_values, response_set);
                auto avoid_set_size = avoid_set.size();
                auto all_values_size = _all_values.size();
                // fully non-deterministic cell, no sequents is necessary
                if (avoid_set_size == 0)
                    return;
                // partiality
                if (avoid_set_size == all_values_size) {
                    NdSequent<std::set> seq {_sequent_dim, {}};
                    place_variables(seq, det);
                    sequents.push_back(seq);
                }
                // other cases
                else {
                    auto judgs_from_set = _judg_value_correspondence->judgements_from_set_values(avoid_set); 
                    for (const auto& judgs : judgs_from_set) {
                        NdSequent<std::set> seq {_sequent_dim, {}};
                        place_in_sequent(seq, _compound, judgs); 
                        place_variables(seq, det);
                        sequents.push_back(seq);
                    }
                }
            };

            void place_variables(NdSequent<std::set>& seq, const Determinant<std::set<int>>& det) {
                const auto& det_args = det.get_args();
                for (size_t i {0}; i < det_args.size(); ++i) {
                    auto dv = det_args[i];
                    auto judgs = _judg_value_correspondence->judgements_from_value(dv);
                    this->place_in_sequent(seq, _props[i], judgs);
                }
            }

            /**
             * Place a given formula in positions corresponding to the
             * given judgements.
             * */
            void place_in_sequent(NdSequent<std::set>& sequent,
                    std::shared_ptr<Formula> fmla, const std::set<CognitiveAttitude>& judgs) {
                for (const auto& j : judgs) {
                    auto pos = _judg_position[j];
                    sequent[pos].insert(fmla);
                } 
            }
    };
};

#endif
