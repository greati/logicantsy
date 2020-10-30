#ifndef __MULT_CONC__
#define __MULT_CONC__

#include "ndsequents.h"
#include <numeric>

namespace ltsy {

    /* A multiple-conclusion rule is simply an
     * NdSequent. For the typical case (one-dimensional),
     * they are sequents with two positions:
     * \Gamma |-- \Delta. For the two-dimensional case,
     * they are sequents with four positions.
     * */
    using MultipleConclusionRule = NdSequent<std::set>;

    /* Represents a multiple conclusion calculus.
     *
     * Holds a set of rules and methods for
     * starting a proof search, considering
     * that the calculus is \phi-analytical.
     *
     * @author Vitor Greati
     * */
    class MultipleConclusionCalculus {
    
        private:

            std::vector<MultipleConclusionRule> _rules;
            std::vector<std::pair<int,int>> _prem_conc_pos_corresp;

        public:

            MultipleConclusionCalculus(const decltype(_rules)& rules)
                : _rules {rules} {}

            /* Try to produce a derivation tree based
             * on the system's rules.
             *
             * @param statement the input statement to be proven,
             * which has the same form of a rule.
             * */
            bool derive(const MultipleConclusionRule& statement,
                    const FmlaSet& phi) {

                // collect all formulas in the input statement
                auto statement_fmlas = statement.collect_fmlas();
                // collect all subformulas of the statement formulas, in Sb
                FmlaSet statement_subfmlas;
                for (const auto& f : statement_fmlas) {
                    SubFormulaCollector collector;
                    f->accept(collector);
                    auto s = collector.subfmlas(); 
                    statement_subfmlas.insert(s.begin(), s.end());
                }
                // collect all propositional variables from the formulas in \phi
                std::set<std::shared_ptr<Prop>> props_phi;
                for (const auto& f : statement_subfmlas) {
                    VariableCollector collector;
                    f->accept(collector);
                    auto p = collector.get_collected_variables();
                    props_phi.insert(p.begin(), p.end());
                }
                // compute \Sb_\phi by all substitutions over \phi using formulas in \Sb
                // generate all substitution instances of the rules _rules using the subformulas in \Sb_\phi
                return false;
            };
    
    };

};
#endif
