#ifndef __PNM_AXIOM_MC__
#define __PNM_AXIOM_MC__

#include "core/syntax.h"
#include "core/utils.h"
#include "core/semantics/genmatrix.h"
#include "core/proof-theory/multconc.h"
#include "spdlog/spdlog.h"

namespace ltsy {


    /* Produce an axiomatization of a generalized
     * matrix. For now, the focus is on at most
     * two distinguished sets.
     *
     * D1 D1* D2 D2* ... DN DN* (dimension = N*2)
     *
     * [[D1],[D1*],[D2],[D2*],...,[DN],    [DN*]]
     *
     *   [0    1]   [2   3]        [2N-2   2N-1] 	(prem_conc_pos_corresp)
     *   [0 1 2 3 ... 2N-2 2N-1] 			(dsets_rule_positions)
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
                s += std::to_string(idx);
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
                int possible = true;
                for (const auto x : X) {
                     auto i {0};
                     while (i < distsets.size() and _discriminator.separators[x][distsets[i]].size() == 0) i++;
                     if (i == distsets.size()) {
                        possible = false;
                        break;
                     }
                }
                if (not possible) return {};
                const auto x = *(X.begin());
                // recursive step
                auto Xm = X;
                Xm.erase(x);
                auto rec_result = make_separator_choice(Xm, distsets);
                std::set<std::vector<FmlaSet>> result = {};
                for (auto res : rec_result) {
                    std::set<std::vector<FmlaSet>> rec_choice_expansions = {res};
                    for (auto distset : distsets) {
                        std::set<std::vector<FmlaSet>> rec_choice_expansions_cpy = rec_choice_expansions;
                        std::set<std::vector<FmlaSet>> new_choice_expansions = {};
                        for (auto rec_choice_expansion : rec_choice_expansions_cpy) {
                            if (_discriminator.separators[x][distset].size() > 0) {
                                for (auto fmla : _discriminator.separators[x][distset]) {
                                    auto new_res = rec_choice_expansion;
                                    new_res[_dsets_positions[distset]].insert(fmla);
                                    new_choice_expansions.insert(new_res); 
                                }
                            } else {
                                new_choice_expansions.insert(rec_choice_expansion);
                            }
                        } 
                        rec_choice_expansions = new_choice_expansions;
                    }
                    result.insert(rec_choice_expansions.begin(), rec_choice_expansions.end());
                } 
                return result;
            }

        public:

            PNMMultipleConclusionAxiomatizer(const decltype(_discriminator)& discriminator,
                    decltype(_gen_matrix) gen_matrix, 
		    std::optional<std::vector<int>> dsets_rule_positions,
                    std::optional<decltype(_prem_conc_pos_corresp)> prem_conc_pos_corresp)
            : _discriminator {discriminator}, _gen_matrix {gen_matrix} {

		if (not dsets_rule_positions) {
			_dsets_rule_positions = std::map<int, int>{};
			for (int i = 0; i < gen_matrix->distinguished_sets().size(); i += 1)
				_dsets_rule_positions[i] = i;
		} else {
			for (int i = 0; i < (*dsets_rule_positions).size(); i += 1) 
			    _dsets_rule_positions[i] = (*(dsets_rule_positions))[i]; 
		}

		if (not prem_conc_pos_corresp) {
			_prem_conc_pos_corresp = decltype(_prem_conc_pos_corresp){};
			for (int i = 0; i < gen_matrix->distinguished_sets().size(); i += 2)
				_prem_conc_pos_corresp.push_back(std::pair<int, int>{i, i+1});
		} else _prem_conc_pos_corresp = *prem_conc_pos_corresp;
            
                for (int i = 0, j=0; i < _gen_matrix->distinguished_sets().size(); i += 2) {
                    _opposition_dsets[i] = i+1;
                    _dsets_positions[i] = j;
                    _dsets_positions[i+1] = j++;
                }
            }

            PNMMultipleConclusionAxiomatizer(const std::vector<int>& dsets_rule_positions,
                    decltype(_prem_conc_pos_corresp) prem_conc_pos_corresp)
            : _prem_conc_pos_corresp {prem_conc_pos_corresp} {
            
                for (int i = 0, j=0; i < dsets_rule_positions.size(); i += 2) {
                    _opposition_dsets[i] = i+1;
                    _dsets_positions[i] = j;
                    _dsets_positions[i+1] = j++; 
                }
                for (int i = 0; i < dsets_rule_positions.size(); i += 1) {
                    _dsets_rule_positions[i]=dsets_rule_positions[i]; 
                }
            }

	    decltype(_prem_conc_pos_corresp) prem_conc_pos_corresp() const { 
	   	return _prem_conc_pos_corresp; 
	    }

	    std::vector<int> seq_dset_corr() const { 
		    std::vector<int> seq_dset_corr;
		    for (auto [k,v]: this->_dsets_rule_positions)
			    seq_dset_corr.push_back(v);
		    return seq_dset_corr;
	    }

            MultipleConclusionCalculus simplify_calculus(
                const MultipleConclusionCalculus& calculus,
                bool simplify_overlap=true, 
                bool simplify_dilution=true, 
                bool simplify_by_cuts=true,
                std::optional<unsigned int> simplify_by_subrule_deriv=std::nullopt
                ) 
            {
                MultipleConclusionCalculus result = calculus;
                auto rules_vector = result.rules();
                auto rules_set = std::set<MultipleConclusionRule> {rules_vector.begin(), rules_vector.end()};
                spdlog::info("Overlap...");
                if (simplify_overlap)
                    rules_set = remove_overlaps(rules_set);
                spdlog::info("Dilution...");
                if (simplify_dilution)
                    remove_dilutions(rules_set);
                spdlog::info("Cuts...");
                if (simplify_by_cuts)
                    rules_set = simplify_by_cut2(rules_set);
                spdlog::info("Dilution...");
                if (simplify_dilution)
                    remove_dilutions(rules_set);
                if (simplify_by_cuts)
                    rules_set = simplify_by_cut2(rules_set);
                spdlog::info("Dilution...");
                if (simplify_dilution)
                    remove_dilutions(rules_set);
                result = MultipleConclusionCalculus {std::vector<MultipleConclusionRule> {rules_set.begin(), rules_set.end()}};
                if (simplify_by_subrule_deriv)
                    rules_set = this->simplify_by_subrule_deriv(result, *simplify_by_subrule_deriv);
                spdlog::info("Done.");
                return MultipleConclusionCalculus {std::vector<MultipleConclusionRule> {rules_set.begin(), rules_set.end()}};
            }

            MultipleConclusionCalculus make_single_calculus(
                    bool simplify_overlap=true, 
                    bool simplify_dilution=true, 
                    std::optional<unsigned int> simplify_subrules_deriv=std::nullopt,
                    std::optional<unsigned int> simplify_with_derivation=std::nullopt,
                    bool simplify_by_cuts=true) {
                auto axiomatization = make_calculus(simplify_overlap, simplify_dilution, false, simplify_by_cuts); 
                std::set<MultipleConclusionRule> full_calculus_rules; 
                for (auto [k, calculus] : axiomatization) {
                    auto calculus_rules = calculus.rules();
                    full_calculus_rules.insert(calculus_rules.begin(), calculus_rules.end());
                }
                MultipleConclusionCalculus full_calculus {
                            std::vector<MultipleConclusionRule>{full_calculus_rules.begin(), 
                                                                full_calculus_rules.end()}
                };

                if (simplify_subrules_deriv and *simplify_subrules_deriv > 0) {
                    spdlog::debug("Simplify by deriving subrules");
                    auto simp_rules = simplify_by_subrule_deriv(full_calculus, *simplify_subrules_deriv);
                    full_calculus = MultipleConclusionCalculus {std::vector<MultipleConclusionRule>{
                                                                        simp_rules.begin(), 
                                                                        simp_rules.end()}};
                }

                if (simplify_with_derivation and *simplify_with_derivation > 0) {
                    spdlog::info("Simplifying using derivations...");
                    const auto& [simplified_calc, removed_rules, depth] = 
                        simplify_by_derivation(full_calculus, 0, simplify_with_derivation);
                    for (const auto& rr : removed_rules)
                        spdlog::info("Removed rule " + rr.sequent().to_string());
                    full_calculus = simplified_calc;
                }
                spdlog::info("Ended with " + std::to_string(full_calculus.rules().size()) + " rules.");
                full_calculus.rename_rules();
                return full_calculus;
            }

            std::map<std::string, MultipleConclusionCalculus> make_calculus(
                    bool simplify_overlap=true, 
                    bool simplify_dilution=true, 
                    bool simplify_subrule_deriv=true,
                    bool simplify_by_cuts=true) {
                auto make_calc_item = [&](const std::set<MultipleConclusionRule>& rulesset) {
                    std::vector<MultipleConclusionRule> rules;
                    for (auto r : rulesset)
                        rules.push_back(r);
                    return MultipleConclusionCalculus {rules};
                };
                std::map<std::string, MultipleConclusionCalculus> result;
                // exists
                spdlog::info("Producing exists rules...");
                auto exists = make_exists_rules(); 
                if (simplify_overlap)
                    exists = remove_overlaps(exists);
                if (simplify_dilution)
                    remove_dilutions(exists);
                result["\\exists"] = make_calc_item(exists);
                spdlog::info("Done.");
                // d
                spdlog::info("Producing D rules...");
                auto d = make_d_rules(); 
                if (simplify_overlap)
                    d = remove_overlaps(d);
                if (simplify_dilution)
                    remove_dilutions(d);
                spdlog::info(std::to_string(d.size()) + " D rules.");
                result["\\mathcal{D}"] = make_calc_item(d);
                spdlog::info("Done.");
                // sigma
                auto sigma_groups = make_sigma_rules();
                for (auto [symb, sigma_conn] : sigma_groups) {
                    spdlog::info("Producing sigma(" + symb + ") rules...");
                    if (simplify_overlap)
                        sigma_conn = remove_overlaps(sigma_conn);
                    if (simplify_dilution)
                        remove_dilutions(sigma_conn);
                    if (simplify_by_cuts)
                        sigma_conn = simplify_by_cut2(sigma_conn);
                    if (simplify_dilution)
                        remove_dilutions(sigma_conn);
                    if (simplify_by_cuts)
                        sigma_conn = simplify_by_cut2(sigma_conn);
                    if (simplify_dilution)
                        remove_dilutions(sigma_conn);
                    result["\\Sigma_{" + symb + "}"] = make_calc_item(sigma_conn);
                    spdlog::info("Done.");
                }
                //non-total rules
                spdlog::info("Producing T rules...");
                auto non_total_rules = make_nontotal_rules();
                if (simplify_overlap)
                    non_total_rules = remove_overlaps(non_total_rules);
                if (simplify_by_cuts)
                    non_total_rules = simplify_by_cut2(non_total_rules);
                if (simplify_dilution)
                    remove_dilutions(non_total_rules);
                result["\\mathcal{T}"] = make_calc_item(non_total_rules);
                spdlog::info("Done.");
                return result;
            }

            void remove_dilutions(std::set<MultipleConclusionRule>& rules) {
                spdlog::debug("Removing dilutions (size " + std::to_string(rules.size()) + ")...");
                auto it = rules.begin();
                int round = 1;
                while (it != rules.end()) {
                    bool is_dilution = false;
                    auto rule = *it;
                    auto rules_cpy = rules;
                    for (const auto& another_rule : rules_cpy) {
                        if (rule == another_rule)
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
                        new_rule.set_name("(cut:" + r1.name() + "," + r2.name() + ")");
                        // perform unions in other dimensions
                        for (auto [d1o,d2o] : _opposition_dsets) {
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
                                    or not r2.sequent().is_dilution_of(new_rule.sequent())) {
                                return std::nullopt;
                            }
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
                spdlog::debug("Cutting sets of rules: " + std::to_string(rules1.size()) + " x " 
                        + std::to_string(rules2.size()));
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
                spdlog::debug("Simplifying by cuts...");
                std::set<MultipleConclusionRule> previous = {};
                std::set<MultipleConclusionRule> current = rules;
                std::set<MultipleConclusionRule> prev_newrules = {};
                std::set<MultipleConclusionRule> newrules = cuts_between_sets(rules, rules);
                int round = 0;
                while (current != previous) {
                    // update progress
                    round += 1;
                    spdlog::debug("Round " + std::to_string(round) + " of cuts");
                    previous = current;
                    prev_newrules = newrules;
                    current.insert(newrules.begin(), newrules.end());
                    newrules = cuts_between_sets(current, prev_newrules); 
                }
                return current;
            }

            std::set<MultipleConclusionRule> remove_overlaps(const std::set<MultipleConclusionRule>& rules) {
                spdlog::debug("Removing overlaps...");
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

                    auto sep_choices1 = make_separator_choice(X, dset2);
                    auto sep_choices2 = make_separator_choice(NX, dset1);

                    int rule_idx = 1;
                    for (auto seps1 : sep_choices1) {
                        for (auto seps2 : sep_choices2) {
                            std::vector<FmlaSet> sequent_fmlas {_dsets_rule_positions.size()};
                            for (auto d1 : dset1)
                                sequent_fmlas[_dsets_rule_positions[d1]] = seps2[_dsets_positions[d1]];
                            for (auto d2 : dset2)
                                sequent_fmlas[_dsets_rule_positions[d2]] = seps1[_dsets_positions[d2]];
                            NdSequent<std::set> sequent {sequent_fmlas};
                            auto rule_name = make_exists_rule_name(X, rule_idx++);
                            MultipleConclusionRule mcrule {rule_name, sequent, _prem_conc_pos_corresp};
                            mcrule.set_group("\\exists");
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
                int rule_idx = 1;
                for (auto v : values) {
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
                        mcrule.set_group("\\mathcal{D}");
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
                         mcrule.set_group("\\Sigma_{" + connective->symbol() + "}");
                         result.insert(mcrule);
                    }
                } 
                return result;
            }

            std::set<MultipleConclusionRule> make_sigma_deterministic_rules(
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
                    //auto response_complement = utils::set_difference(values, response);
                    for (auto y : response) {
                         std::vector<FmlaSet> sequent {dsets.size()};
                         for (int i = 0; i < args.size(); ++i) {
                            auto xi = args[i];
                            for (auto [dset, dpos] : _dsets_rule_positions) {
                                auto seps = _discriminator.apply_subs(xi, props[i]);
                                sequent[dpos].insert(seps[dset].begin(), seps[dset].end());
                            }
                         }
                         for (auto [dset, dpos] : _dsets_rule_positions) {
                            auto seps = _discriminator.apply_subs(y, compound);
                            for (auto sep : seps[dset]) {
                                auto sequent_copy = sequent;
                                auto opposite_pos = _opposition_dsets.find(dpos)->second;
                                sequent_copy[opposite_pos].insert(sep);
                                NdSequent<std::set> sequent_rule {sequent_copy};
                                // make rule name
                                std::string rule_name = connective->symbol() + "-";
                                for (auto a : args)
                                    rule_name += std::to_string(a) + ",";
                                rule_name += std::to_string(y);
                                MultipleConclusionRule mcrule {rule_name, sequent_rule, _prem_conc_pos_corresp};
                                mcrule.set_group("\\Sigma_{" + connective->symbol() + "}");
                                result.insert(mcrule);
                            }
                         }
                    }
                } 
                return result;
            }

            std::map<std::string, std::set<MultipleConclusionRule>> make_sigma_rules() {
                std::map<std::string, std::set<MultipleConclusionRule>> result;
                auto interpretation = _gen_matrix->interpretation();
                // loop over interpretations
                for (auto [symb, interp] : *interpretation) {
                    //auto rules = make_sigma_deterministic_rules(interp);
                    auto rules = make_sigma_rules(interp);
                    result[symb] = rules;
                }
                return result;
            }

            std::set<MultipleConclusionRule> make_nontotal_rules() {
                auto non_total_subsets = _gen_matrix->get_non_total_subsets();
                auto dist_sets_qtd = _gen_matrix->distinguished_sets().size();
                std::set<MultipleConclusionRule> result;
                auto props = make_props(_gen_matrix->values().size()); 
                for (const auto& X : non_total_subsets) {
		    if (X.empty()) continue;
                    std::vector<FmlaSet> sequent_fmlas {dist_sets_qtd};
                    for (const auto& x : X) {
                        auto prop = props[x];
                        for (int i = 0; i < dist_sets_qtd; ++i) {
                            auto seps = _discriminator.apply_subs(x, prop);
                            sequent_fmlas[_dsets_rule_positions[i]].insert(seps[i].begin(), seps[i].end());
                        }
                    }
                    NdSequent<std::set> sequent {sequent_fmlas};
                    MultipleConclusionRule mcrule {"", sequent, _prem_conc_pos_corresp};
                    mcrule.set_group("\\mathcal{T}");
                    result.insert(mcrule);
                }
                return result;
            }

            std::set<MultipleConclusionRule> simplify_by_subrule_deriv(
                    MultipleConclusionCalculus calculus, 
                    std::optional<unsigned int> max_depth = std::nullopt) const {
                auto rules = calculus.rules();
                spdlog::debug("Simplifying by subrule derivations (size "+ std::to_string(rules.size()) +")...");
                std::vector<int> dset_positions;
                for (const auto& [a, b] : _dsets_rule_positions)
                    dset_positions.push_back(b);
                std::set<MultipleConclusionRule> result;
                for (const auto& rule : rules) {
                    spdlog::debug("Processing rule " + rule.sequent().to_string());
                    std::set<MultipleConclusionRule> sound_subrules;
                    ltsy::MultipleConclusionSubrulesGenerator gen {rule}; 
                    gen.next(); // skip empty
                    while(gen.has_next()) {
                        auto subr = gen.next();
                        // if it was the last, ignore
                        if (not gen.has_next()) break;
                        spdlog::debug("Subrule " + subr.sequent().to_string());
                        // DERIVE HERE
                        auto derivation = calculus.derive(subr, _discriminator.get_formulas());
                        if (derivation->closed) {
                            sound_subrules.insert(subr);
                            spdlog::debug("Derived subrule " + subr.sequent().to_string());
                            break; //! TODO keep all sound subrules and then select one amonst them? 
                        }
                    }
                    if (sound_subrules.empty())
                        result.insert(rule);
                    else
                        result.insert(sound_subrules.begin(), sound_subrules.end());
                }
                return result;
            }

            std::tuple<MultipleConclusionCalculus, std::set<MultipleConclusionRule>, unsigned int>
                simplify_by_derivation(const MultipleConclusionCalculus& calculus, 
                        unsigned int depth, std::optional<unsigned int> max_depth = std::nullopt) const {
                if (calculus.size() == 0)
                    return {calculus, {}, depth};
                if (max_depth and depth >= *max_depth)
                    return std::tuple<MultipleConclusionCalculus, 
                           std::set<MultipleConclusionRule>, unsigned int>(calculus, {}, *max_depth);
                MultipleConclusionCalculus simplified_calc = calculus;
                std::set<MultipleConclusionRule> rules_removed = {};
                auto rules = calculus.rules();
                int  max_depth_so_far = depth;
                for (auto i {0}; i < rules.size(); ++i) {
                    auto rules_simp = rules;
                    rules_simp.erase(rules_simp.begin() + i);
                    MultipleConclusionCalculus simp_calc {rules_simp};
                    auto derivation = simp_calc.derive(rules[i], _discriminator.get_formulas());
                    if (derivation->closed) {
                        spdlog::debug("Derived " + rules[i].sequent().to_string() + 
                                " in depth " + std::to_string(depth) + " using " + 
                                std::to_string(simp_calc.size()) + " rules ");
                        const auto [rec_calculus, rec_rules_rem, rec_depth] = 
                            simplify_by_derivation(simp_calc, depth + 1, max_depth);
                        if (rec_depth > max_depth_so_far) {
                            max_depth_so_far = rec_depth;
                            simplified_calc = rec_calculus; 
                            rules_removed = rec_rules_rem;
                            rules_removed.insert(rules[i]);
                            if (max_depth and rec_depth >= *max_depth)
                                break;
                        }
                    }
                }
                return std::make_tuple(simplified_calc, rules_removed, max_depth_so_far);
            }
    };


};

#endif
