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
    class MultipleConclusionRule {
    
        private:
            std::string _name;
            NdSequent<std::set> _sequent;

        public:
            MultipleConclusionRule(const std::string& name,
                    const decltype(_sequent)& sequent)
            : _name {name}, _sequent {sequent} {}

            decltype(_sequent) sequent() const { return _sequent; }
            decltype(_name) name() const { return _name; }
    
    };

    /* Represents a multiple conclusion calculus.
     *
     * Holds a set of rules and methods for
     * starting a proof search, considering
     * that the calculus is \phi-analytical.
     *
     * We also intend here to implement a
     * generalized version of a multiple
     * conclusion calculus, allowing tuples of
     * sets of formulas in each node of a derivation.
     *
     * @author Vitor Greati
     * */
    class MultipleConclusionCalculus {

        public:

            /* Represents a derivation tree 
             * in a (generalized) multiple conclusion calculus.
             * */
            struct DerivationTree {
                std::vector<FmlaSet> node;
                std::vector<DerivationTree> children;
            };
    
        private:

            std::vector<MultipleConclusionRule> _rules;
            std::vector<std::pair<int,int>> _prem_conc_pos_corresp;

            bool expand_node(std::vector<FmlaSet> node_fmlas, 
                    const std::vector<FmlaSet>& conclusions,
                    const FmlaSet& generalized_subfmlas,
                    DerivationTree& derivation) {
                // test if satisfied
                bool satisfied = true;
                for (auto i {0}; i < conclusions.size() and satisfied; ++i) {
                    FmlaSet inters;
                    std::set_intersection(node_fmlas[i].begin(), node_fmlas[i].end(),
                            conclusions[i].begin(), conclusions[i].end(), std::inserter(inters, inters.begin()));
                    satisfied &= not inters.empty();
                }
                // if satisfied, close this node (stop recursion)
                if (satisfied) return true;
                // if not satisfied, search by applying the system's rules
                else {
                    //TODO
                    for (const auto& rule : _rules) {
                        // collect rule props
                        // instantiate assignment generator
                        // for each assignment
                        // apply it to the rule
                        // check if the rule instance is applicable
                        // if yes, expand a new node for each conclusion
                        // (check Batista's work to see if there are more conditions here)
                    }
                    return false;    
                }
            }

        public:


            /* Basic constructor.
             * */
            MultipleConclusionCalculus(const decltype(_rules)& rules,
                    const decltype(_prem_conc_pos_corresp)& prem_conc_pos_corresp)
                : _rules {rules}, _prem_conc_pos_corresp {prem_conc_pos_corresp} {}

            /* Try to produce a derivation tree based
             * on the system's rules.
             *
             * @param statement the input statement to be proven,
             * which has the same form of a rule.
             * */
            bool derive(const MultipleConclusionRule& statement,
                    const FmlaSet& phi) {
                // collect all formulas in the input statement
                auto statement_fmlas = statement.sequent().collect_fmlas();
                // collect all subformulas of the statement formulas, in Sb
                FmlaSet statement_subfmlas;
                for (auto f : statement_fmlas) {
                    SubFormulaCollector collector;
                    f->accept(collector);
                    auto s = collector.subfmlas(); 
                    statement_subfmlas.insert(s.begin(), s.end());
                }
                // collect all propositional variables from the formulas in \phi
                PropSet props_phi;
                for (auto f : statement_subfmlas) {
                    VariableCollector collector;
                    f->accept(collector);
                    auto p = collector.get_collected_variables();
                    props_phi.insert(p.begin(), p.end());
                }
                // compute \Sb_\phi by all substitutions over \phi using formulas in \Sb
                FmlaSet sb_phi;
                FormulaVarAssignmentGenerator gen_sb_phi_subs {props_phi, statement_subfmlas};
                for (auto fm : phi) {
                    while(gen_sb_phi_subs.has_next()) {
                        auto s = gen_sb_phi_subs.next();
                        SubstitutionEvaluator eval {*s};
                        auto fmsubs = fm->accept(eval);
                        sb_phi.insert(fmsubs);
                    }
                    gen_sb_phi_subs.reset();
                }
                // split premisses and conclusions
                std::vector<FmlaSet> premisses;
                std::vector<FmlaSet> conclusions;
                for (const auto& [p,c] : _prem_conc_pos_corresp) {
                    premisses.push_back(statement.sequent()[p]);   
                    conclusions.push_back(statement.sequent()[c]);   
                }
                DerivationTree derivation;
                return expand_node(premisses, conclusions, sb_phi, derivation);
            };
    
    };

};
#endif
