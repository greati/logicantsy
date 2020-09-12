#ifndef __CLI_HANDLERS__
#define __CLI_HANDLERS__

#include <iostream>
#include "yaml/yamlcpp_parser.h"
#include "core/common.h"
#include "spdlog/spdlog.h"
#include "tt_determination/ndsequents.h"
#include "core/parser/fmla/fmla_parser.h"
#include "apps/apps_facade.h"

namespace ltsy {

    class TTDeterminizerCLIHandler {

        private:

            const std::string SEMANTICS_TITLE = "semantics";
            const std::string SEMANTICS_VALUES_TITLE = "values";
            const std::string SEMANTICS_JUDGEMENTS_TITLE = "judgements";
            const std::string CONNECTIVES_TITLE = "connectives";
            const std::string SEQUENTS_TITLE = "sequents";

            std::map<int, std::string> _val_to_str;
            std::map<std::string, int> _str_to_val;
            std::vector<CognitiveAttitude> _judgements;
            std::map<std::string, std::shared_ptr<Connective>> _connectives;
            std::vector<std::shared_ptr<Prop>> _props;
            std::map<std::string, std::vector<NdSequent<std::set>>> _sequents_per_connective;

        public:
            void handle(const std::string& yaml_path) {
                YAMLCppParser parser;
                auto root = parser.load_from_file(yaml_path);
                try {
                    auto semantics_node = parser.hard_require(root, SEMANTICS_TITLE);

                    // parse values
                    auto values_node = parser.hard_require(semantics_node, SEMANTICS_VALUES_TITLE);
                    auto values = values_node.as<std::vector<std::string>>();
                    auto nvalues = values.size();
                    for (int i {0}; i < nvalues; ++i) {
                        _str_to_val[values[0]] = 0;
                        _val_to_str[0] = values[0];
                    }
                    // parse judgements
                    auto judgements_node = parser.hard_require(semantics_node, SEMANTICS_JUDGEMENTS_TITLE);
                    for (auto it = judgements_node.begin(); it != judgements_node.end(); ++it) {
                        auto judge_name = it->first.as<std::string>(); 
                        auto judge_strs = it->second.as<std::vector<std::string>>(); 
                        std::set<int> judge_values;
                        for (const auto& j : judge_strs) {
                            judge_values.insert(_str_to_val[j]); 
                        }
                        _judgements.push_back({judge_name, judge_values});
                    }
                    // parse connectives
                    auto connectives_node = parser.hard_require(root, CONNECTIVES_TITLE);
                    for (auto it = connectives_node.begin(); it != connectives_node.end(); ++it) {
                        auto conn_name = it->first.as<std::string>();
                        auto conn_arity = it->second.as<int>();
                        _connectives[conn_name] = std::make_shared<Connective>(conn_name, conn_arity);
                    }

                    // parse sequents, for each connective
                    auto sequents_node = parser.hard_require(root, SEQUENTS_TITLE);
                    for (const auto& conn_node : sequents_node) {
                        auto conn_name = conn_node.first.as<std::string>(); 
                        auto conn_seqs = conn_node.second;
                        std::vector<NdSequent<std::set>> conn_parsed_sequents;
                        std::set<Prop*, utils::DeepPointerComp<Prop>> variables;
                        for (const auto& seq_node : conn_seqs) {
                            // each seq_node is a list of lists
                            std::vector<std::set<std::shared_ptr<Formula>>> seq_places_sets;
                            for (const auto& seq_place_node : seq_node) {
                                std::set<std::shared_ptr<Formula>> fmlas_in_place;
                                auto fmlas_str = seq_place_node.as<std::vector<std::string>>();
                                for (const auto& fmla_str : fmlas_str) {
                                    BisonFmlaParser fmla_parser;
                                    auto parsed_fmla = fmla_parser.parse(fmla_str);
                                    fmlas_in_place.insert(parsed_fmla);
                                    VariableCollector var_collector;
                                    parsed_fmla->accept(var_collector);
                                    auto collected_vars = var_collector.get_collected_variables();
                                    variables.insert(collected_vars.begin(), collected_vars.end());
                                }
                                seq_places_sets.push_back(fmlas_in_place);
                            }
                            conn_parsed_sequents.push_back(NdSequent<std::set>(seq_places_sets));
                        }
                        _sequents_per_connective[conn_name] = conn_parsed_sequents;
                        std::vector<Prop> vars;
                        for (const auto& v : variables)
                            vars.push_back(*v);
                        AppsFacade apps_facade;
                        auto table = apps_facade.determinize_truth_table(nvalues, vars, *_connectives[conn_name],
                                _judgements, _sequents_per_connective[conn_name]);
                    }
                    std::cout << "Successful parsing." << std::endl;    
                } catch (ParseException& pe) {
                    spdlog::critical(pe.message());
                }
            };
    };

};

#endif
