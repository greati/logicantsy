#ifndef __PNM_AXIOM_MC__
#define __PNM_AXIOM_MC__

#include "core/syntax.h"
#include "core/utils.h"
#include "core/semantics/genmatrix.h"
#include "core/proof-theory/multconc.h"

namespace ltsy {


    /* Produce an axiomatization of a generalized
     * matrix. For now, the focus is on at most
     * two distinguished sets.
     *
     * @author Vitor Greati
     * */
    class PNMMultipleConclusionAxiomatizer {
    
        public:
            /* Represents a discriminator
             * as a map of lists of sets of formulas.
             * */
            struct Discriminator {
                public:
                    std::map<int, std::vector<FmlaSet>> separators;
                    Discriminator(const decltype(separators) _separators) : separators {_separators} {}
                    std::vector<FmlaSet> apply_subs(int v, std::shared_ptr<Formula>& sfmla) {
                        std::vector<FmlaSet> result {separators[v].size()};
                        for (int i = 0; i < separators[v].size(); ++i) {
                            auto set = separators[v][i];
                            for (auto fmla : set) {
                                VariableCollector vcollector; fmla->accept(vcollector);
                                auto props = vcollector.get_collected_variables(); // this should have a single var
                                if (props.size() != 1) throw std::logic_error("discriminator is not monadic");
                                auto prop = *props.begin();
                                FormulaVarAssignment subst { {{*prop, sfmla}} };
                                SubstitutionEvaluator substeval {subst};
                                auto resfmla = fmla->accept(substeval);
                                result[i].insert(resfmla); 
                            }
                        }
                        return result;
                    }
            };

        private:

            std::map<int,int> _opposition_dsets;
            std::map<int, int> _dsets_positions;
            std::map<int, int> _dsets_rule_positions;
            std::vector<std::pair<int,int>> _prem_conc_pos_corresp;

            std::string make_exists_rule_name(const std::set<int>& X, const int& idx) const {
                std::string s = "E-";
                for (auto x : X) {
                    s += std::to_string(x);
                }
                s += "|";
                s += std::to_string(idx);
                return s;
            }
            std::string make_d_rule_name(const int v, const int& idx) const {
                std::string s = "D-";
                s += std::to_string(v);
                return s;
            }

        protected:
            Discriminator _discriminator;
            std::shared_ptr<GenMatrix> _gen_matrix;

            /* For each x \in X, choose formulas from Dx1...Dxk,
             * resulting in tuples of formulas.
             * No choice is available when for some x \in X,
             * Ui Dxi = \empty.
             * When X = \empty, a single choice is possible: (\empty,...,\empty)
             * */
            std::set<std::vector<FmlaSet>> 
                make_separator_choice(const std::set<int> X, const std::vector<int>& distsets) {
                if (X.empty()) {
                    return { std::vector<FmlaSet>(distsets.size(), FmlaSet{}) };
                } 
                auto x = *(X.begin());

                int possible = false;
                for (auto dset : distsets) {
                    if (_discriminator.separators[x][dset].size() > 0) {
                        possible=true;
                        break;
                    }
                }

                if (not possible) return {};

                auto Xm = X;
                Xm.erase(Xm.begin());

                auto rec_result = make_separator_choice(Xm, distsets);
                std::set<std::vector<FmlaSet>> result = rec_result;
                for (auto distset : distsets) {
                    if (_discriminator.separators[x][distset].empty())
                         continue;
                    std::set<std::vector<FmlaSet>> new_result;
                    for (auto res : result) {
                        for (auto fmla : _discriminator.separators[x][distset]) {
                            auto new_res = res;
                            new_res[_dsets_positions[distset]].insert(fmla);
                            new_result.insert(new_res); 
                        } 
                    }
                    result = new_result;
                } 
                return result;
            }

        public:
            PNMMultipleConclusionAxiomatizer(const decltype(_discriminator)& discriminator,
                    decltype(_gen_matrix) gen_matrix)
            : _discriminator {discriminator}, _gen_matrix {gen_matrix} {
            
                for (int i = 0, j=0; i < _gen_matrix->distinguished_sets().size(); i += 2) {
                    _opposition_dsets[i] = i+1;
                    _dsets_positions[i] = j;
                    _dsets_positions[i+1] = j++; 
                }
                for (int i = 0; i < _gen_matrix->distinguished_sets().size(); i += 1) {
                    _dsets_rule_positions[i]=i; 
                }
                for (auto [a,b] : _opposition_dsets) {
                    _prem_conc_pos_corresp.push_back({a,b});
                }
            }

            std::map<std::string, MultipleConclusionCalculus> make_calculus() {
                auto exists = make_exists_rules(); 
                exists = remove_overlaps(exists);
                auto d = make_d_rules(); 
                d = remove_overlaps(d);
                auto sigma = make_sigma_rules(); 
                sigma = remove_overlaps(sigma);

                auto allrules = exists;
                allrules.insert(d.begin(), d.end());
                allrules.insert(sigma.begin(), sigma.end());

                auto make_calc_item = [&](const decltype(exists)& rulesset) {
                    std::vector<MultipleConclusionRule> rules;
                    for (auto r : rulesset)
                        rules.push_back(r);
                    return MultipleConclusionCalculus {rules};
                };
                std::map<std::string, MultipleConclusionCalculus> result;
                result["all"] = make_calc_item(allrules);
                result["exists"] = make_calc_item(exists);
                result["sigma"] = make_calc_item(sigma);
                result["d"] = make_calc_item(d);
                return result;
            }

            std::set<MultipleConclusionRule> remove_overlaps(const std::set<MultipleConclusionRule>& rules) {
                std::set<MultipleConclusionRule> result;
                for (auto r : rules) {
                    for (auto [d1,d2] : _opposition_dsets) {
                        auto s1 = r.sequent().sequent_fmlas()[d1];
                        auto s2 = r.sequent().sequent_fmlas()[d2];
                        decltype(s1) inters;
                        std::set_intersection(s1.begin(), s1.end(),
                                s2.begin(), s2.end(), 
                                std::inserter(inters, inters.begin()),
                                utils::DeepSharedPointerComp<Formula>());
                        if (not inters.empty())
                            continue;
                        result.insert(r);
                    }
                }
                return result;    
            }

            std::set<MultipleConclusionRule> make_exists_rules() {
                auto values = _gen_matrix->values();
                std::set<MultipleConclusionRule> result;
                DiscretureCombinationGenerator combination_gen {values.size()};
                while (combination_gen.has_next()) {
                    auto comb = *(combination_gen.next());
                    auto X = std::set<int>{comb.begin(), comb.end()};
                    auto NX = utils::set_difference(values, X);

                    std::vector<int> dset1, dset2;
                    for (auto [d1,d2] : _opposition_dsets) {
                        dset1.push_back(d1); dset2.push_back(d2);
                    }

                    auto sep_choices1 = make_separator_choice(X, dset1);
                    auto sep_choices2 = make_separator_choice(NX, dset2);
                    int rule_idx = 1;
                    for (auto seps1 : sep_choices1) {
                        for (auto seps2 : sep_choices2) {
                            std::vector<FmlaSet> sequent_fmlas {_dsets_rule_positions.size()};
                            for (auto d1 : dset1)
                                sequent_fmlas[_dsets_rule_positions[d1]] = seps1[_dsets_positions[d1]];
                            for (auto d2 : dset2)
                                sequent_fmlas[_dsets_rule_positions[d2]] = seps2[_dsets_positions[d2]];
                            NdSequent<std::set> sequent {sequent_fmlas};
                            auto rule_name = make_exists_rule_name(X, rule_idx++);
                            MultipleConclusionRule mcrule {rule_name, sequent, _prem_conc_pos_corresp};
                            result.insert(mcrule);
                        }
                    }
                }
                return result;
            }
            FmlaSet make_p(const std::set<int>& S, int x, std::shared_ptr<Formula> p){
                if (S.find(x) != S.end())
                    return {p};
                return {};
            }
            
            std::set<MultipleConclusionRule> make_d_rules() {
                auto values = _gen_matrix->values();
                auto dsets = _gen_matrix->distinguished_sets();
                std::set<MultipleConclusionRule> result;
                std::shared_ptr<Formula> p = std::make_shared<Prop>("p");
                for (auto v : values) {
                    int rule_idx = 1;
                    std::vector<FmlaSet> base_seq_fmlas {dsets.size()};
                    for (int i = 0; i < dsets.size(); ++i)
                        base_seq_fmlas[_dsets_rule_positions[i]] = 
                            _discriminator.separators[v][i]; 
                    for (auto [d1,d2] : _opposition_dsets) {
                        auto seq_fmlas = base_seq_fmlas;
                        auto p_set1 = make_p(dsets[d1], v, p);
                        auto p_set2 = make_p(dsets[d2], v, p);
                        seq_fmlas[_dsets_rule_positions[d1]].insert(p_set2.begin(), p_set2.end()); 
                        seq_fmlas[_dsets_rule_positions[d2]].insert(p_set1.begin(), p_set1.end()); 
                        NdSequent<std::set> sequent {seq_fmlas};
                        auto rule_name = make_d_rule_name(v, rule_idx++);
                        MultipleConclusionRule mcrule {rule_name, sequent, _prem_conc_pos_corresp};
                        result.insert(mcrule);
                    }
                }
                return result;
            }
            std::vector<std::shared_ptr<Formula>> make_props(int k) const {
                std::vector<std::shared_ptr<Formula>> result;
                for(int i = 1; i <= k; ++i) {
                    std::string s = "p" + std::to_string(i);
                    result.push_back(std::make_shared<Prop>(s));
                }
                return result;
            }
            std::shared_ptr<Formula> make_compound(std::shared_ptr<Connective> conn,
                    const std::vector<std::shared_ptr<Formula>>& props) {
                return std::make_shared<Compound>(conn, props); 
            }
            std::set<MultipleConclusionRule> make_sigma_rules() {
                auto values = _gen_matrix->values();
                auto dsets = _gen_matrix->distinguished_sets();
                std::set<MultipleConclusionRule> result;
                auto interpretation = _gen_matrix->interpretation();
                // loop over interpretations
                for (auto [symb, interp] : *interpretation) {
                    auto connective = interp->connective();
                    auto truth_table = interp->truth_table();
                    auto arity = connective->arity();
                    auto props = make_props(arity);
                    auto compound = make_compound(connective, props);
                    // loop over the determinants
                    for (auto determinant : truth_table->get_determinants()) {
                        auto args = determinant.get_args(); 
                        auto response = determinant.get_last();
                        auto response_complement = utils::set_difference(values, response);
                        for (auto y : response_complement) {
                             std::vector<FmlaSet> sequent {dsets.size()};
                             for (auto [dset, dpos] : _dsets_rule_positions) {
                                 auto seps = _discriminator.apply_subs(y, compound);
                                 sequent[dpos].insert(seps[dset].begin(), seps[dset].end());
                             }
                             for (int i = 0; i < args.size(); ++i) {
                                 auto xi = args[i];
                                 for (auto [dset, dpos] : _dsets_rule_positions) {
                                     auto seps = _discriminator.apply_subs(xi, props[i]);
                                     sequent[dpos].insert(seps[dset].begin(), seps[dset].end());
                                 }
                             }
                             NdSequent<std::set> sequent_rule {sequent};
                             // make rule name
                             std::string rule_name = connective->symbol() + "-";
                             for (auto a : args)
                                 rule_name += std::to_string(a) + ",";
                             rule_name += std::to_string(y);
                             MultipleConclusionRule mcrule {rule_name, sequent_rule, _prem_conc_pos_corresp};
                             result.insert(mcrule);
                        }
                    } 
                }
                return result;
            }
            std::set<MultipleConclusionRule> make_nontotal_rules() {
            }

    };


};

#endif
