#ifndef __YAML_CPP_PARSER__
#define __YAML_CPP_PARSER__

#include "yaml-cpp/yaml.h"
#include "core/exception/exceptions.h"
#include "core/semantics/truth_tables.h"
#include "spdlog/spdlog.h"

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
        }

    };

};

#endif
