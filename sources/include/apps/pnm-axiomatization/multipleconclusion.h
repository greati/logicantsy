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
            };

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
                            new_res[distset].insert(fmla);
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
            : _discriminator {discriminator}, _gen_matrix {gen_matrix} {}

            std::set<MultipleConclusionRule> make_exists_rules() {
                auto values = _gen_matrix->values();
                DiscretureCombinationGenerator combination_gen {values.size()};
                while (combination_gen.has_next()) {
                    auto comb = *(combination_gen.next());
                    auto X = std::set<int>{comb.begin(), comb.end()};
                    auto NX = utils::set_difference(values, X);
                    auto ss = make_separator_choice(X, {0,1});
                    for (auto x : X)
                        std::cout << std::to_string(x) << ",";
                    std::cout << std::endl;
                    for (auto s : ss) {
                        std::cout << "(";
                        for (auto fs : s) {
                            std::cout << "{";
                            for (auto f : fs) {
                                std::cout << *f << ",";
                            }
                            std::cout << "},";
                        }
                        std::cout << ")" << std::endl;
                    }
                }
                return {};
            }
            std::set<MultipleConclusionRule> make_sigma_rules() {
            }
            std::set<MultipleConclusionRule> make_d_rules() {
            }
            std::set<MultipleConclusionRule> make_nontotal_rules() {
            }

    };


};

#endif
