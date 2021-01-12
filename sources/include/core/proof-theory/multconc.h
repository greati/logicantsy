#ifndef __MULT_CONC__
#define __MULT_CONC__

#include "ndsequents.h"
#include <numeric>
#include <memory>
#include "core/combinatorics/combinations.h"

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
            std::string _group = "unnamed";
            NdSequent<std::set> _sequent;
            std::shared_ptr<FormulaVarAssignment> _substitution;
            std::vector<std::pair<int,int>> _prem_conc_pos_corresp;
            std::vector<FmlaSet> _premisses;
            std::vector<FmlaSet> _conclusions;
            bool _all_conclusions_empty = true;

        public:

            MultipleConclusionRule(const std::string& name,
                    const decltype(_sequent)& sequent,
                    const decltype(_prem_conc_pos_corresp)& prem_conc_pos_corresp,
                    decltype(_substitution) substitution)
            : _name {name}, _sequent {sequent}, _prem_conc_pos_corresp {prem_conc_pos_corresp},
                _substitution {substitution} {
            
                for (const auto& [p,c] : prem_conc_pos_corresp) {
                    _premisses.push_back(sequent[p]);   
                    _conclusions.push_back(sequent[c]);   
                    _all_conclusions_empty &= sequent[c].empty();
                }
            
            }

            MultipleConclusionRule(const std::string& name,
                    const decltype(_sequent)& sequent,
                    const decltype(_prem_conc_pos_corresp)& prem_conc_pos_corresp)
            : MultipleConclusionRule {name, sequent, prem_conc_pos_corresp, nullptr} {}

            bool operator<(const MultipleConclusionRule& other) const {
                auto comp = [&](const FmlaSet& f1, const FmlaSet& f2) {
                    return std::lexicographical_compare(
                            f1.begin(), f1.end(), f2.begin(), f2.end(),
                            utils::DeepSharedPointerComp<Formula>());
                };
                auto seqfmlasleft = _sequent.sequent_fmlas();
                auto seqfmlasright = other._sequent.sequent_fmlas();
                return std::lexicographical_compare(seqfmlasleft.begin(), seqfmlasleft.end(),
                        seqfmlasright.begin(), seqfmlasright.end(),
                        comp);
            }
            bool operator==(const MultipleConclusionRule& other) const {
                for (int i = 0; i < _sequent.dimension(); ++i) {
                    if (_sequent.sequent_fmlas()[i] != other._sequent.sequent_fmlas()[i])    
                        return false;
                }
                return true;
            }

            inline void set_group(const decltype(_group)& group) { _group = group; }
            inline decltype(_group) group() const { return _group; }

            decltype(_sequent) sequent() const { return _sequent; }

            void set(int i, FmlaSet fmlas) {
                _sequent[i] = fmlas;
            }

            inline decltype(_name) name() const { return _name; }
            inline void set_name(const decltype(_name)& name) { _name = name; }

            decltype(_all_conclusions_empty) all_conclusions_empty() const { return _all_conclusions_empty; }

            decltype(_prem_conc_pos_corresp) prem_conc_pos_corresp() const { return _prem_conc_pos_corresp; }

            std::vector<FmlaSet> premisses() const { return _premisses; }
            std::vector<FmlaSet> conclusions() const { return _conclusions; }

            MultipleConclusionRule apply_substitution(const FormulaVarAssignment& ass) const {
                auto new_seq = _sequent.apply_substitution(ass);
                auto assptr = std::make_shared<FormulaVarAssignment>(ass);
                MultipleConclusionRule r {_name, new_seq, _prem_conc_pos_corresp, assptr}; 
                return r;
            }

            bool has_only(const FmlaSet& fmlas) const {
                auto rule_fmlas = _sequent.collect_fmlas();
                return is_subset(rule_fmlas, fmlas); 
            }
    };


    /* Generate all subrules of a given rule.
     * */
    class MultipleConclusionSubrulesGenerator {

        private:

            MultipleConclusionRule _base_rule;
            std::vector<std::vector<std::shared_ptr<Formula>>> _fmlas;
            std::vector<DiscretureCombinationGenerator> _comb_gens;
            NdSequent<std::set> _current;
            int _dim;
            bool finished = false;
            bool first = true;
    
        public:

            MultipleConclusionSubrulesGenerator(const MultipleConclusionRule& base_rule) : _base_rule {base_rule},
               _current {NdSequent<std::set> {base_rule.sequent().dimension()}} {
                   for (int i = 0; i < _base_rule.sequent().dimension(); ++i) {
                        auto fmlas_pos = _base_rule.sequent().sequent_fmlas()[i];
                        _fmlas.push_back(std::vector<std::shared_ptr<Formula>>(fmlas_pos.begin(), fmlas_pos.end()));
                   }
                   reset();
            }

            void reset() {
                finished = false;
                first = true;
                _dim = _base_rule.sequent().dimension();
                _current = NdSequent<std::set> {_dim};
                // initialize combination generators
                for (auto i {0}; i < _dim; ++i) {
                    _comb_gens.push_back(DiscretureCombinationGenerator {_fmlas[i].size()});
                    _comb_gens[i].next();
                }
            }

            NdSequent<std::set> next_sequent() {
                if (not has_next())
                    throw std::logic_error("no next subformula to produce");
                if (first) {first=false; return _current;}
                int i = _dim - 1;
                while (i >= 0 and not _comb_gens[i].has_next()) {
                    _comb_gens[i].reset();
                    auto comb_idxs = *(_comb_gens[i].next());
                    _current.set(i, {});
                    i--;
                }
                if (i >= 0) {
                    auto comb_idxs = *(_comb_gens[i].next());
                    FmlaSet subfmlas;
                    for (auto j : comb_idxs) {
                        subfmlas.insert(_fmlas[i][j]);
                    }
                    _current.set(i, subfmlas);
                }
                i = _dim - 1;
                while (i >= 0 and not _comb_gens[i].has_next()) i--;
                if (i < 0) finished=true;
                return _current;
            }

            MultipleConclusionRule next() {
                if (not has_next())
                    throw std::logic_error("no next subformula to produce");
                auto sequent = next_sequent();
                auto res = MultipleConclusionRule {_base_rule.name()+" (sub)", sequent, _base_rule.prem_conc_pos_corresp()};
                res.set_group(_base_rule.group());
                return res;
            }

            bool has_next() {
                return not finished;
            }
    
    };

    /* An interface for proof search heuristics.
     * */
    class MCProofSearchHeuristics {
        protected:
            std::vector<MultipleConclusionRule> _rules;
            FmlaSet _fmlas_to_make_instances;
        public:
            MCProofSearchHeuristics(const decltype(_rules)& rules,
                    const decltype(_fmlas_to_make_instances)& fmlas_to_make_instances)
                : _rules {rules}, _fmlas_to_make_instances {fmlas_to_make_instances} {}
            virtual ~MCProofSearchHeuristics() {};
            virtual MultipleConclusionRule select_instance() = 0;
            virtual bool has_next() = 0;
    };

    /* Sequential heuristics.
     *
     * The simplest heuristics: generate the instances
     * in a fixed order.
     *
     * @author Vitor Greati
     * */
    class MCProofSearchSequentialHeuristics : public MCProofSearchHeuristics {
        private:
            decltype(_rules)::iterator _rules_it;
            FormulaVarAssignmentGenerator _current_subst_generator;
            bool finished = false;
        public:
            MCProofSearchSequentialHeuristics(const decltype(_rules)& rules,
                    const decltype(_fmlas_to_make_instances)& fmlas_to_make_instances)
                : MCProofSearchHeuristics {rules, fmlas_to_make_instances} {
                    init();
            }

            void init() {
                if (_rules.empty())
                    finished = true;
                else {
                    finished = false;
                    _rules_it = _rules.begin();
                    auto rule = *(_rules_it);
                    auto rule_props = rule.sequent().collect_props();
                    _current_subst_generator = FormulaVarAssignmentGenerator 
                            {rule_props, _fmlas_to_make_instances};
                }
            }

            MultipleConclusionRule select_instance() {
                if (has_next()) {
                    if (std::next(_rules_it) != _rules.end() and not _current_subst_generator.has_next()) {
                        auto rule = *(++_rules_it);
                        auto rule_props = rule.sequent().collect_props();
                        _current_subst_generator = FormulaVarAssignmentGenerator 
                                                        {rule_props, _fmlas_to_make_instances};
                    }
                    auto rule = *(_rules_it);
                    if (_current_subst_generator.has_next()) {
                        auto ass = _current_subst_generator.next();
                        // apply it to the rule
                        auto rule_instance = rule.apply_substitution(*ass);
                        if (std::next(_rules_it) == _rules.end() and not _current_subst_generator.has_next())
                            finished = true;
                        return rule_instance;
                    } else {
                        if (std::next(_rules_it) == _rules.end() and not _current_subst_generator.has_next())
                            finished = true;
                        return rule;
                    }
                }
                throw std::logic_error("there is no next rule instance");
            }
            bool has_next() {
                return not finished;
                //return (std::next(_rules_it) != _rules.end()) or 
                //    (std::next(_rules_it) == _rules.end() and _current_subst_generator.has_next());
            }
    };

    /* A modification of the sequential heuristics
     * to shuffle the vector of rules at the beginning.
     *
     * @author Vitor Greati
     * */
    class MCProofSearchRandSequentialHeuristics : public MCProofSearchSequentialHeuristics {
        public:
            MCProofSearchRandSequentialHeuristics(const decltype(_rules)& rules,
                    const decltype(_fmlas_to_make_instances)& fmlas_to_make_instances)
                : MCProofSearchSequentialHeuristics {rules, fmlas_to_make_instances} {
                // shuffle
                std::random_shuffle(_rules.begin(), _rules.end());
                init();
            }
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
            struct DerivationTreeNode {
                std::vector<FmlaSet> node;
                std::optional<MultipleConclusionRule> rule_instance;
                std::vector<std::shared_ptr<DerivationTreeNode>> children;
                bool closed = false;
                bool star = false;
                bool end_branch = false;

                DerivationTreeNode(const decltype(rule_instance)& _rule,
                        bool _star = true) :
                    rule_instance {_rule}, star {_star} {}

                DerivationTreeNode(const decltype(node)& _node,
                        const decltype(rule_instance)& _rule,
                        const decltype(children)& _children) 
                : node {_node}, rule_instance {_rule}, children {_children}{/**/}

                DerivationTreeNode(const decltype(node)& _node,
                        const decltype(children)& _children) 
                : DerivationTreeNode {_node, std::nullopt, _children} {/**/}

                void add_child(std::shared_ptr<DerivationTreeNode> new_node) {
                    children.push_back(new_node);
                }

                std::stringstream print(const DerivationTreeNode* parent=nullptr, int level = 0) const {
                    const int SPACES = 2;
                    std::stringstream ss;
                    std::string spaces = "";
                    if (level > 0) {
                        spaces = std::string (level*SPACES, ' ');
                        for (int i = 0; i < level*SPACES; i += SPACES)
                            spaces[i] = '|';
                        for (int i = (level - 1)*SPACES+1; i < level*SPACES; ++i)
                            spaces[i] = '-';

                    }
                    ss << spaces << "> ";
                    if (rule_instance) {
                        ss << "(" << rule_instance->name() << ") ";
                    }
                    if (star) 
                        ss << "(*)";
                    else {
                        for (auto i {0}; i < node.size(); ++i) {
                            FmlaSet diff;
                            if (parent != nullptr) { 
                                std::set_difference(
                                        node[i].begin(), node[i].end(),
                                        parent->node[i].begin(),
                                        parent->node[i].end(),
                                        std::inserter(diff, diff.begin()),
                                        utils::DeepSharedPointerComp<Formula>());
                            } else diff = node[i];
                            for (auto f = diff.begin(); f != diff.end(); ++f) {
                                ss << *(*f);
                                if (std::next(f) != diff.end())
                                    ss << ",";
                            }
                            if (i < node.size() - 1)
                                ss << " | ";
                        }
                    }
                    if (end_branch)
                        ss << "(#)";    
                    ss << std::endl;
                    for (auto j {0}; j < children.size(); ++j) {
                        ss << children[j]->print(this, level + 1).str();
                    }
                    return ss;
                }
            };
    
        private:

            std::vector<MultipleConclusionRule> _rules;
            unsigned int _analiticity_level = 1;

            void print_set(const FmlaSet& f) {
                for (auto ff : f)
                    std::cout << (*ff) << std::endl;
            }

            bool expand_node(std::vector<FmlaSet> node_fmlas, 
                    const std::vector<FmlaSet>& conclusions,
                    const FmlaSet& fmlas_to_make_instances,
                    const FmlaSet& fmlas_allowed_in_derivations,
                    std::shared_ptr<DerivationTreeNode> derivation,
                    int level, const std::optional<int>& max_depth) {
                // check satisfaction
                bool satisfied = false;
                for (auto i {0}; i < conclusions.size() and not satisfied; ++i) {
                    FmlaSet inters = intersection(node_fmlas[i], conclusions[i]);
                    satisfied = satisfied or not inters.empty();
                }
                // if satisfied, close this node
                if (satisfied) {
                    derivation->end_branch = true;
                    derivation->closed = true;
                    return true;
                }
                // if not satisfied, search by applying the system's rules
                else {
                    // check max_depth
                    if (max_depth and level > *max_depth)
                        return false;
                    satisfied = true;
                    bool some_premiss_satisfied = false;
                    auto rules = _rules;
                    // create the heuristics
                    auto heuristics = std::make_shared<MCProofSearchRandSequentialHeuristics>(rules,
                            fmlas_to_make_instances);
                    while (heuristics->has_next()) {
                       // obtain an instance
                       auto rule_instance = heuristics->select_instance();
                       // validate for analiticity
                       if (not rule_instance.has_only(fmlas_allowed_in_derivations))
                           continue;
                       // check if premisses subseteq node_fmlas
                       const auto& premisses = rule_instance.premisses();
                       bool premisses_satisfied = true;
                       for (auto i {0}; i < node_fmlas.size(); ++i)
                           premisses_satisfied &= is_subset(premisses[i], node_fmlas[i]);
                       if (premisses_satisfied) {
                           if (rule_instance.all_conclusions_empty()) {
                               auto star_node = 
                                   std::make_shared<DerivationTreeNode>(
                                       std::make_optional<MultipleConclusionRule>(rule_instance)
                                   );
                               derivation->add_child(star_node);
                               return true;
                           } else {
                               auto rule_conclusions = rule_instance.conclusions();
                               for (auto i {0}; i < rule_conclusions.size(); ++i) {
                                   FmlaSet inters = intersection(node_fmlas[i], rule_conclusions[i]);
                                   // check if the node formulas have some of the conclusion formulas
                                   if (not inters.empty())
                                       continue;
                                   // if not, expand
                                   some_premiss_satisfied = true;
                                   for (auto rule_conc_fmla : rule_conclusions[i]) {
                                       // expand a new node by adding A in position i
                                       decltype(node_fmlas) new_node_fmlas {node_fmlas.begin(), 
                                           node_fmlas.end()};
                                       new_node_fmlas[i].insert(rule_conc_fmla);
                                       auto new_node = 
                                           std::make_shared<DerivationTreeNode>(
                                               new_node_fmlas, 
                                               std::make_optional<MultipleConclusionRule>(rule_instance), 
                                               std::vector<std::shared_ptr<DerivationTreeNode>>{}
                                           );
                                       // expand
                                       auto expanded_satisfied = expand_node(new_node_fmlas, 
                                               conclusions, fmlas_to_make_instances, 
                                               fmlas_allowed_in_derivations, new_node, level+1,
                                               max_depth);
                                       satisfied &= expanded_satisfied;
                                       // if the expanded node do not lead to a closed branch
                                       if (not expanded_satisfied)
                                           break;
                                       // else
                                       derivation->add_child(new_node);
                                   }
                               }
                           }
                       }
                       if (some_premiss_satisfied and satisfied) {
                           derivation->closed = true;
                           return true; 
                       }
                    }
                    return false;
                }
            }

            std::pair<FmlaSet, FmlaSet> 
            gen_subformulas(const MultipleConclusionRule& statement, const FmlaSet& phi, 
                    const PropSet& props_phi, const unsigned int& k) {
                if (k == 0) {
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
                    return {{}, statement_subfmlas};
                } 
                auto [S1, S2] = gen_subformulas(statement, phi, props_phi, k-1);
                FmlaSet gen_sb_phi = S2; // S_{k-1} cup...
                // compute \Sb_\phi by all substitutions over \phi using formulas in \Sb
                FormulaVarAssignmentGenerator gen_sb_phi_subs {props_phi, S2}; // props_phi -> S2
                for (auto fm : phi) {
                    while(gen_sb_phi_subs.has_next()) {
                        auto s = gen_sb_phi_subs.next();
                        SubstitutionEvaluator eval {*s};
                        auto fmsubs = fm->accept(eval);
                        gen_sb_phi.insert(fmsubs);
                    }
                    gen_sb_phi_subs.reset();
                }
                return {S2, gen_sb_phi};
            }

        public:

            MultipleConclusionCalculus() {}

            /* Basic constructor.
             * */
            MultipleConclusionCalculus(const decltype(_rules)& rules)
                : _rules {rules} {}

            decltype(_rules) rules() const { return _rules; }
            void add_rule(const MultipleConclusionRule& rule) { _rules.push_back(rule); }

            inline unsigned int size() const { return _rules.size(); }

            std::map<std::string, MultipleConclusionCalculus> group() const {
                std::map<std::string, MultipleConclusionCalculus> groups;
                for (const auto& rule : _rules) {
                    auto group_name = rule.group();
                    if (groups.find(group_name) == groups.end())
                        groups[group_name] = MultipleConclusionCalculus {{rule}};
                    else
                        groups[group_name].add_rule(rule);
                }
                return groups;
            }

            /* Try to produce a derivation tree based
             * on the system's rules.
             *
             * @param statement the input statement to be proven,
             * which has the same form of a rule.
             * */
            std::shared_ptr<DerivationTreeNode> derive(const MultipleConclusionRule& statement,
                    const FmlaSet& phi, std::optional<int> max_depth = std::nullopt) {
                // get the props in phi
                PropSet props_phi;
                for (auto f : phi) {
                    VariableCollector collector;
                    f->accept(collector);
                    auto p = collector.get_collected_variables();
                    props_phi.insert(p.begin(), p.end());
                }
                // compute the generalized subformulas
                const auto& [thetak_1, thetak] = gen_subformulas(statement, phi, props_phi, _analiticity_level);
                // identify premisses and conclusion
                std::vector<FmlaSet> premisses;
                std::vector<FmlaSet> conclusions;
                for (const auto& [p,c] : statement.prem_conc_pos_corresp()) {
                    premisses.push_back(statement.sequent()[p]);   
                    conclusions.push_back(statement.sequent()[c]);   
                }
                // search for the derivation
                auto derivation = std::make_shared<DerivationTreeNode>(premisses, 
                        std::vector<std::shared_ptr<DerivationTreeNode>>{});
                bool derivation_result = expand_node(premisses, conclusions, thetak_1, thetak, derivation, 0,
                        max_depth);
                return derivation;
            };
    
    };

};
#endif
