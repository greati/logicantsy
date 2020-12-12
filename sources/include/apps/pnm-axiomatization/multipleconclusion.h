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
                // base case: X is empty
                if (X.empty())
                    return { std::vector<FmlaSet>(distsets.size(), FmlaSet{}) };
                // check if possible
                auto x = *(X.begin());
                int possible = false;
                for (auto dset : distsets) {
                    if (_discriminator.separators[x][dset].size() > 0) {
                        possible=true;
                        break;
                    }
                }
                if (not possible) return {};
                // recursive step
                auto Xm = X;
                Xm.erase(Xm.begin());
                auto rec_result = make_separator_choice(Xm, distsets);
                std::set<std::vector<FmlaSet>> result = {};
                for (auto distset : distsets) {
                    for (auto res : rec_result) {
                        for (auto fmla : _discriminator.separators[x][distset]) {
                            auto new_res = res;
                            new_res[_dsets_positions[distset]].insert(fmla);
                            result.insert(new_res); 
                        } 
                    }
                } 
                return result;
            }

        public:
            PNMMultipleConclusionAxiomatizer(const decltype(_discriminator)& discriminator,
                    decltype(_gen_matrix) gen_matrix, const std::vector<int>& dsets_rule_positions)
            : _discriminator {discriminator}, _gen_matrix {gen_matrix} {
            
                for (int i = 0, j=0; i < _gen_matrix->distinguished_sets().size(); i += 2) {
                    _opposition_dsets[i] = i+1;
                    _dsets_positions[i] = j;
                    _dsets_positions[i+1] = j++; 
                }
                for (int i = 0; i < _gen_matrix->distinguished_sets().size(); i += 1) {
                    _dsets_rule_positions[i]=dsets_rule_positions[i]; 
                }
                for (auto [a,b] : _opposition_dsets) {
                    _prem_conc_pos_corresp.push_back({a,b});
                }
            }

            std::map<std::string, MultipleConclusionCalculus> make_calculus(bool simplify_overlap=true, bool simplify_dilution=true) {
                auto make_calc_item = [&](const std::set<MultipleConclusionRule>& rulesset) {
                    std::vector<MultipleConclusionRule> rules;
                    for (auto r : rulesset)
                        rules.push_back(r);
                    return MultipleConclusionCalculus {rules};
                };
                std::map<std::string, MultipleConclusionCalculus> result;
                // exists
                auto exists = make_exists_rules(); 
                if (simplify_overlap)
                    exists = remove_overlaps(exists);
                if (simplify_dilution)
                    remove_dilutions(exists);
                result["exists"] = make_calc_item(exists);
                // d
                auto d = make_d_rules(); 
                if (simplify_overlap)
                    d = remove_overlaps(d);
                if (simplify_dilution)
                    remove_dilutions(d);
                result["d"] = make_calc_item(d);
                // sigma
                auto sigma_groups = make_sigma_rules();
                for (auto [symb, sigma_conn] : sigma_groups) {
                    if (simplify_overlap)
                        sigma_conn = remove_overlaps(sigma_conn);
                    if (simplify_dilution)
                        remove_dilutions(sigma_conn);
                    sigma_conn = simplify_by_cut2(sigma_conn);
                    remove_dilutions(sigma_conn);
                    //simplify_by_cut(sigma_conn);
                    result["sigma-"+symb] = make_calc_item(sigma_conn);
                }
                //non-total rules
                auto non_total_rules = make_nontotal_rules();
                non_total_rules = simplify_by_cut2(non_total_rules);
                remove_dilutions(non_total_rules);
                result["nontotal"] = make_calc_item(non_total_rules);
                return result;
            }

            void remove_dilutions(std::set<MultipleConclusionRule>& rules) {
                auto it = rules.begin();
                while (it != rules.end()) {
                    bool is_dilution = false;
                    auto rule = *it;
                    for (const auto& another_rule : rules) {
                        if (rule.sequent() == another_rule.sequent())
                            continue;
                        if (rule.sequent().is_dilution_of(another_rule.sequent())) {
                            it = rules.erase(it);
                            is_dilution = true;
                            break;
                        }
                    } 
                    if (not is_dilution) {
                        it++;
                    }
                }
            }

            std::optional<MultipleConclusionRule> 
            simple_cut(const MultipleConclusionRule& r1, const MultipleConclusionRule& r2,
                    bool subrule_check=true) {
                for (auto [d1,d2] : _opposition_dsets) {
                    auto s1 = r1.sequent().sequent_fmlas()[_dsets_rule_positions[d1]];
                    auto s2 = r2.sequent().sequent_fmlas()[_dsets_rule_positions[d2]];
                    auto inters = intersection(s1, s2);
                    if (not inters.empty()) {
                        inters = {*inters.begin()};
                        // perform the cut
                        auto dif1 = difference(s1, inters); 
                        auto dif2 = difference(s2, inters); 
                        auto others1 = r1.sequent().sequent_fmlas()[_dsets_rule_positions[d2]];
                        auto others2 = r2.sequent().sequent_fmlas()[_dsets_rule_positions[d1]];
                        dif2.insert(others1.begin(), others1.end());
                        dif1.insert(others2.begin(), others2.end());
                        MultipleConclusionRule new_rule = r1;
                        new_rule.set(_dsets_rule_positions[d1], dif1);
                        new_rule.set(_dsets_rule_positions[d2], dif2);
                        // perform unions in other dimensions
                        for (const auto& [d1o,d2o] : _opposition_dsets) {
                            if (d1 != d1o and d2 != d2o) {
                                auto s11o = r1.sequent().sequent_fmlas()[_dsets_rule_positions[d1o]]; 
                                auto s21o = r2.sequent().sequent_fmlas()[_dsets_rule_positions[d1o]]; 
                                auto s12o = r1.sequent().sequent_fmlas()[_dsets_rule_positions[d2o]]; 
                                auto s22o = r2.sequent().sequent_fmlas()[_dsets_rule_positions[d2o]]; 
                                s11o.insert(s21o.begin(), s21o.end());
                                s12o.insert(s22o.begin(), s22o.end());
                                new_rule.set(_dsets_rule_positions[d1o], s11o);
                                new_rule.set(_dsets_rule_positions[d2o], s12o);
                            }
                        }
                        // check if result is subrule
                        if (subrule_check) {
                            if (not r1.sequent().is_dilution_of(new_rule.sequent())
                                    or not r2.sequent().is_dilution_of(new_rule.sequent()))
                                return std::nullopt;
                        }
                        return std::optional<MultipleConclusionRule>(new_rule);
                    }
                }
                return std::nullopt;
            }

            /* Given sets of rules R1 and R2, consider
             * each pair <r1,r2> in R1xR2 and try to cut.
             * The result is the set of all new rules generated from cuts.
             * */
            std::set<MultipleConclusionRule> cuts_between_sets(const std::set<MultipleConclusionRule>& rules1,
                    const std::set<MultipleConclusionRule>& rules2, bool subrule_check=true) {
                std::set<MultipleConclusionRule> result;
                for (auto it1 = rules1.begin(); it1 != rules1.end(); ++it1) {
                    auto r1 = *it1;
                    for (auto it2 = rules2.begin(); it2 != rules2.end(); ++it2) {
                        auto r2 = *it2;
                        // try cut in one direction
                        auto cut_result1 = simple_cut(r1, r2);
                        if (cut_result1)
                            result.insert(*cut_result1);        
                        // try cut in another direction
                        auto cut_result2 = simple_cut(r2, r1);
                        if (cut_result2)
                            result.insert(*cut_result2);        
                    }
                } 
                return result;
            }

            std::set<MultipleConclusionRule> simplify_by_cut2(const std::set<MultipleConclusionRule>& rules) {
                std::set<MultipleConclusionRule> previous = {};
                std::set<MultipleConclusionRule> current = rules;
                std::set<MultipleConclusionRule> prev_newrules = {};
                std::set<MultipleConclusionRule> newrules = cuts_between_sets(rules, rules);
                while (current != previous) {
                    previous = current;
                    prev_newrules = newrules;
                    current.insert(newrules.begin(), newrules.end());
                    newrules = cuts_between_sets(current, prev_newrules); 
                }
                return current;
            }

            std::set<MultipleConclusionRule> remove_overlaps(const std::set<MultipleConclusionRule>& rules) {
                std::set<MultipleConclusionRule> result;
                for (auto r : rules) {
                    bool overlap = false;
                    for (auto [d1,d2] : _opposition_dsets) {
                        auto s1 = r.sequent().sequent_fmlas()[_dsets_rule_positions[d1]];
                        auto s2 = r.sequent().sequent_fmlas()[_dsets_rule_positions[d2]];
                        decltype(s1) inters;
                        std::set_intersection(s1.begin(), s1.end(),
                                s2.begin(), s2.end(), 
                                std::inserter(inters, inters.begin()),
                                utils::DeepSharedPointerComp<Formula>());
                        if (not inters.empty())
                            overlap = true;
                    }
                    if (not overlap)
                        result.insert(r);
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

            std::set<MultipleConclusionRule> make_sigma_rules(
                    std::shared_ptr<TruthInterp<std::set<int>>> interp) {
                auto values = _gen_matrix->values();
                auto dsets = _gen_matrix->distinguished_sets();
                std::set<MultipleConclusionRule> result;
                auto interpretation = _gen_matrix->interpretation();
                // loop over interpretations
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
                return result;
            }

            std::map<std::string, std::set<MultipleConclusionRule>> make_sigma_rules() {
                std::map<std::string, std::set<MultipleConclusionRule>> result;
                auto interpretation = _gen_matrix->interpretation();
                // loop over interpretations
                for (auto [symb, interp] : *interpretation) {
                    auto rules = make_sigma_rules(interp);
                    result[symb] = rules;
                }
                return result;
            }

            std::set<MultipleConclusionRule> make_nontotal_rules() {
                auto non_total_subsets = _gen_matrix->get_non_total_subsets();
                //std::cout << "non total" << std::endl;
                //for (auto sts : non_total_subsets) {
                //    for (auto nt : sts) {
                //        std::cout << nt << ",";
                //    }
                //    std::cout << std::endl;
                //}
                auto dist_sets_qtd = _gen_matrix->distinguished_sets().size();
                std::set<MultipleConclusionRule> result;
                auto props = make_props(_gen_matrix->values().size()); 
                for (const auto& X : non_total_subsets) {
                    std::vector<FmlaSet> sequent_fmlas {dist_sets_qtd};
                    for (const auto& x : X) {
                        if (X.empty()) continue;
                        auto prop = props[x];
                        for (int i = 0; i < dist_sets_qtd; ++i) {
                            auto seps = _discriminator.apply_subs(x, prop);
                            sequent_fmlas[_dsets_rule_positions[i]].insert(seps[i].begin(), seps[i].end());
                        }
                    }
                    NdSequent<std::set> sequent {sequent_fmlas};
                    MultipleConclusionRule mcrule {"", sequent, _prem_conc_pos_corresp};
                    result.insert(mcrule);
                }
                return result;
            }

    };


};

#endif
