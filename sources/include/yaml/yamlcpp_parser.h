#ifndef __YAML_CPP_PARSER__
#define __YAML_CPP_PARSER__

#include "yaml-cpp/yaml.h"
#include "core/exception/exceptions.h"
#include "core/semantics/truth_tables.h"
#include "core/parser/fmla/fmla_parser.h"
#include "spdlog/spdlog.h"
#include "core/semantics/genmatrix.h"
#include <tuple>

namespace ltsy {

    class YAMLCppParser {
        
        public:

        YAML::Node load_from_file(const std::string& path) const {
            return YAML::LoadFile(path);
        }

        /**
         * Check if a node exists in the current one. If not, throws
         * an error.
         *
         * @param curr_node the current node
         * @param node_name the name of the node
         * @return if the node exists in the current one
         * */
        YAML::Node hard_require(const YAML::Node & curr_node, const std::string& node_name) const {
            if (!curr_node[node_name]) {
                throw ParseException("missing node " + node_name, curr_node.Mark().line, curr_node.Mark().column);
            }
            return curr_node[node_name]; 
        }

        /**
         * Check if a node exists in the current one. If not, print a warning.
         *
         * @param curr_node the current node
         * @param node_name the name of the node
         * @return if the node exists in the current one
         * */
        bool soft_require(const YAML::Node & curr_node, const std::string& node_name) const {
            return curr_node[node_name];
        }

        const std::string TT_DEFAULT_VALUE_NAME = "default";
        const std::string TT_RESTRICTIONS_NAME = "restrictions";
        const std::string PNMATRIX_NAME = "pnmatrix";
        const std::string PNMATRIX_VALUES_NAME = "values";
        const std::string PNMATRIX_DSETS_NAME = "distinguished_sets";
        const std::string PNMATRIX_INTERP_NAME = "interpretation";
        const std::string PREMISSES_SEQRULE_NAME = "premisses";
        const std::string CONCLUSIONS_SEQRULE_NAME = "conclusions";

        std::shared_ptr<FmlaParser> make_fmla_parser(const YAML::Node& node) const {
            return std::make_shared<BisonFmlaParser>(BisonFmlaParser::Location {node.Mark().line, 
                                                                                node.Mark().column});
        }

        std::set<int> parse_set_str_to_values(const YAML::Node& node,
                const std::map<std::string, int>& str_to_val) const {
            auto list_of_str = node.as<std::vector<std::string>>();
            std::set<int> s;
            for (const auto& str: list_of_str) {
                if (str_to_val.find(str) == str_to_val.end())
                    throw ParseException("value " + str + " not defined");
                s.insert(str_to_val.find(str)->second);
            }
            return s;
        }

        std::vector<int> parse_vector_str_to_values(const YAML::Node& node,
                const std::map<std::string, int>& str_to_val) const {
            auto list_of_str = node.as<std::vector<std::string>>();
            std::vector<int> v;
            for (const auto& str: list_of_str) {
                if (str_to_val.find(str) == str_to_val.end())
                    throw ParseException("value " + str + " not defined");
                v.push_back(str_to_val.find(str)->second);
            }
            return v;
        }

        std::vector<std::vector<int>> process_tt_restriction(
                std::vector<std::string>& rests,
                const std::set<int>& values,
                const std::map<std::string, int>& str_to_val) const {
            if (rests.empty()) {
                auto new_args = std::vector<std::vector<int>>();
                new_args.push_back(std::vector<int>());
                return new_args;
            }
            auto last_element = rests.back();
            rests.pop_back();
            auto restriction_recursive = process_tt_restriction(rests, values, str_to_val); 
            std::vector<std::vector<int>> r;
            for (const auto& args_recursive : restriction_recursive) {
                if (last_element == "_") {
                    for (const auto& v : values) {
                        auto new_args = std::vector<int>(args_recursive.begin(), args_recursive.end());
                        new_args.push_back(v);
                        r.push_back(new_args);
                    } 
                } else {
                    if (str_to_val.find(last_element) == str_to_val.end())
                        throw ParseException("value " + last_element + " not defined");
                    auto new_args = std::vector<int>(args_recursive.begin(), args_recursive.end());
                    new_args.push_back(str_to_val.find(last_element)->second);
                    r.push_back(new_args);
                }
            }
            return r;
        }

        std::tuple<std::shared_ptr<Compound>, std::vector<std::shared_ptr<Prop>>> parse_connective_representant(const YAML::Node& node) const {
            auto fmla_parser = make_fmla_parser(node);
            auto conn_compound = node.as<std::string>();
            auto compound = std::dynamic_pointer_cast<Compound>(fmla_parser->parse(conn_compound));
            auto conn_name = compound->connective()->symbol(); 
            VariableCollector var_collector;
            compound->accept(var_collector);
            auto collected_vars = var_collector.get_collected_variables();
            std::vector<std::shared_ptr<Prop>> vars;
            for (const auto v : collected_vars)
                vars.push_back(std::make_shared<Prop>(*v));
            return std::tuple{compound, vars};
        }

        /* Parse a truth table pattern.
         * */
        std::set<Determinant<std::set<int>>> parse_pattern_str_to_determinant(
                std::vector<std::string>& args_pattern,
                const std::set<int>& det_result,
                const std::set<int>& values,
                const std::map<std::string, int>& str_to_val) const {
            std::set<Determinant<std::set<int>>> dets;
            auto processed_args = process_tt_restriction(args_pattern, values, str_to_val);
            int nvalues = values.size();
            for (const auto& arg : processed_args) {
                dets.insert(Determinant<std::set<int>>(nvalues, arg, det_result)); 
            }
            return dets;
        }

        NDTruthTable parse_nd_truth_table(const YAML::Node& ttnode, 
                const std::set<int>& values, int arity,
                const std::map<std::string, int>& str_to_val) 
            const {
            try {
                auto nvalues = values.size();
                // default values and table creation
                NDTruthTable tt;
                if (soft_require(ttnode, TT_DEFAULT_VALUE_NAME)) {
                    std::set<int> default_values = parse_set_str_to_values(ttnode[TT_DEFAULT_VALUE_NAME],
                            str_to_val);
                    tt = NDTruthTable {nvalues, arity, default_values};
                } else {
                    tt = generate_fully_nd_table(nvalues, arity);
                }
                // restrictions
                auto restrictions_node = hard_require(ttnode, TT_RESTRICTIONS_NAME);
                for (const auto& restriction_dummy : restrictions_node) {
                    for (const auto& restriction : restriction_dummy) {
                        auto det_result = parse_set_str_to_values(restriction.second, str_to_val); 
                        auto args_pattern = restriction.first.as<std::vector<std::string>>(); 
                        auto dets = parse_pattern_str_to_determinant(args_pattern, det_result, values, str_to_val);
                        tt.update(dets);
                    }
                }
                return tt;
            } catch (YAML::ParserException &ye) {
                throw;
            }
        }

        std::shared_ptr<GenMatrix> parse_gen_matrix(const YAML::Node& root) const {
            auto pnmatrix_node = hard_require(root, PNMATRIX_NAME);             
            std::map<int, std::string> _val_to_str;
            std::map<std::string, int> _str_to_val;
            // parse values
            auto values_node = hard_require(pnmatrix_node, PNMATRIX_VALUES_NAME);
            auto values = values_node.as<std::vector<std::string>>();
            std::set<int> real_values;
            auto nvalues = values.size();
            for (int i {0}; i < nvalues; ++i) {
                _str_to_val[values[i]] = i;
                _val_to_str[i] = values[i];
                real_values.insert(i);
            }
            // parse distinguished sets
            std::vector<std::set<int>> distinguished_sets;
            auto dsets_node = hard_require(pnmatrix_node, PNMATRIX_DSETS_NAME);
            for (const auto& dset_node : dsets_node) {
                std::set<int> dset;
                auto dset_strs = dset_node.as<std::vector<std::string>>();
                for (const auto& s : dset_strs) {
                    dset.insert(_str_to_val[s]);
                }
                distinguished_sets.push_back(dset);
            } 
            // parse interpretations
            auto interp_node = hard_require(pnmatrix_node, PNMATRIX_INTERP_NAME);
            auto signature = std::make_shared<Signature>();
            auto sig_truth_interp = std::make_shared<SignatureTruthInterp<std::set<int>>>(signature);
            for (auto it_tt = interp_node.begin(); it_tt != interp_node.end(); ++it_tt) {
                 // capture connective and variables
                 auto [compound, collected_vars] = parse_connective_representant(it_tt->first);
                 signature->add(compound->connective());
                 // parse tt and interpret connectives
                 NDTruthTable tt = parse_nd_truth_table(it_tt->second, real_values,
                         compound->connective()->arity(), _str_to_val);
                 auto truth_interp = std::make_shared<TruthInterp<std::set<int>>>(compound->connective(), 
                         std::make_shared<NDTruthTable>(tt));
                 sig_truth_interp->try_interpret(truth_interp);
            }
            // create matrix
            auto gen_matrix = std::make_shared<GenMatrix>(real_values, distinguished_sets, 
                    signature, sig_truth_interp);
            return gen_matrix;
        }

        std::shared_ptr<NdSequent<std::set>> parse_nd_sequent(const YAML::Node& seq_node) {
            std::vector<std::set<std::shared_ptr<Formula>>> seq_places_sets;
            for (const auto& seq_place_node : seq_node) {
                std::set<std::shared_ptr<Formula>> fmlas_in_place;
                auto fmlas_str = seq_place_node.as<std::vector<std::string>>();
                for (const auto& fmla_str : fmlas_str) {
                    auto fmla_parser = make_fmla_parser(seq_place_node);
                    auto parsed_fmla = fmla_parser->parse(fmla_str);
                    fmlas_in_place.insert(parsed_fmla);
                }
                seq_places_sets.push_back(fmlas_in_place);
            }
            return std::make_shared<NdSequent<std::set>>(seq_places_sets); 
        }

        std::shared_ptr<NdSequentRule<std::set>> parse_nd_sequent_rule(const std::string& name, 
                const YAML::Node& rule_node) {
            auto premisses_node = hard_require(rule_node, PREMISSES_SEQRULE_NAME);
            std::vector<NdSequent<std::set>> premisses;
            for (const auto& p : premisses_node) {
                premisses.push_back(*parse_nd_sequent(p));
            }
            auto conclusions_node = hard_require(rule_node, CONCLUSIONS_SEQRULE_NAME);
            std::vector<NdSequent<std::set>> conclusions;
            for (const auto& c : conclusions_node) {
                conclusions.push_back(*parse_nd_sequent(c));
            }
            return std::make_shared<NdSequentRule<std::set>>(name, premisses, conclusions); 
        }

    };

};

#endif
