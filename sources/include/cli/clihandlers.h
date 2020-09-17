#ifndef __CLI_HANDLERS__
#define __CLI_HANDLERS__

#include <iostream>
#include "yaml/yamlcpp_parser.h"
#include "spdlog/spdlog.h"
#include "tt_determination/ndsequents.h"
#include "core/parser/fmla/fmla_parser.h"
#include "apps/apps_facade.h"
#include "core/semantics/attitude_semantics.h"
#include "core/common.h"

namespace ltsy {

    class TTAxiomatizerCLIHandler {
        private:
            const std::string SEMANTICS_TITLE = "semantics";
            const std::string SEMANTICS_VALUES_TITLE = "values";
            const std::string SEMANTICS_JUDGMENTS_TITLE = "judgements";
            const std::string TRUTH_TABLE_TITLE = "truth-tables";
            std::map<int, std::string> _val_to_str;
            std::map<std::string, int> _str_to_val;
            std::map<std::string, std::shared_ptr<Connective>> _connectives;
            std::map<std::string, std::vector<std::shared_ptr<Prop>>> _props;
        public:
            void handle(const std::string& yaml_path) {
                YAMLCppParser parser;
                auto root = parser.load_from_file(yaml_path);
                try {
                    auto semantics_node = parser.hard_require(root, SEMANTICS_TITLE);
                    // parse values
                    auto values_node = parser.hard_require(semantics_node, SEMANTICS_VALUES_TITLE);
                    auto values = values_node.as<std::vector<std::string>>();
                    std::set<int> real_values;
                    auto nvalues = values.size();
                    for (int i {0}; i < nvalues; ++i) {
                        _str_to_val[values[i]] = i;
                        _val_to_str[i] = values[i];
                        real_values.insert(i);
                    }
                    // parse judgement names
                    auto judgements_node = parser.hard_require(semantics_node, SEMANTICS_JUDGMENTS_TITLE);
                    auto judgements_names = judgements_node.as<std::vector<std::string>>();
                    // semantics
                    std::shared_ptr<JudgementValueCorrespondence> jvc =
                        std::make_shared<FourBillatice>();
                    // semantics judgements
                    std::vector<CognitiveAttitude> judgements_in_semantics;
                    for (const auto& s : judgements_names)
                        judgements_in_semantics.push_back(jvc->get_judgement(s));
                    //parse tts and run app
                    auto tt_root = parser.hard_require(root, TRUTH_TABLE_TITLE);
                    for (const auto& tt_node : tt_root) {
                        // capture connective and variables
                        BisonFmlaParser fmla_parser;
                        auto conn_compound = tt_node.first.as<std::string>();
                        auto compound = std::dynamic_pointer_cast<Compound>(fmla_parser.parse(conn_compound));
                        auto conn_name = compound->connective()->symbol(); 
                        VariableCollector var_collector;
                        compound->accept(var_collector);
                        auto collected_vars = var_collector.get_collected_variables();
                        std::vector<std::shared_ptr<Prop>> vars;
                        for (const auto v : collected_vars)
                            vars.push_back(std::make_shared<Prop>(*v));
                        _connectives[conn_name] = compound->connective();
                        _props[conn_name] = vars;
                        // parse tt
                        NDTruthTable tt = parser.parse_nd_truth_table(tt_node.second, real_values,
                                compound->connective()->arity(), _str_to_val);
                        auto table_print = tt.print(_val_to_str).str();
                        spdlog::info("Input table for connective " + conn_name + " is\n" + table_print);
                        AppsFacade apps_facade;
                        auto sequents = apps_facade.axiomatize_truth_table(real_values,
                                *compound->connective(),
                                vars,
                                compound,
                                judgements_in_semantics,
                                jvc,
                                tt);
                        std::string out = "";
                        for (const auto& seq : sequents)
                            out += seq.to_string() + "\n"; 
                        spdlog::info("Here is your axiomatization for " + conn_name + ":\n" + out);
                    }
                    spdlog::info("Done.");
                } catch (ParseException& pe) {
                    spdlog::critical(pe.message());
                }
            }
    };

    class TTDeterminizerCLIHandler {

        private:

            const std::string SEMANTICS_TITLE = "semantics";
            const std::string SEMANTICS_VALUES_TITLE = "values";
            const std::string SEMANTICS_JUDGEMENTS_TITLE = "judgements";
            const std::string CONDITIONS_TITLE = "conditions";

            std::map<int, std::string> _val_to_str;
            std::map<std::string, int> _str_to_val;
            std::vector<CognitiveAttitude> _judgements;
            std::map<std::string, std::shared_ptr<Connective>> _connectives;
            std::map<std::string, std::vector<Prop>> _props;
            //std::vector<std::shared_ptr<Prop>> _props;
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
                        _str_to_val[values[i]] = i;
                        _val_to_str[i] = values[i];
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
                    // parse conditions
                    auto conditions_node = parser.hard_require(root, CONDITIONS_TITLE);
                    for (auto it = conditions_node.begin(); it != conditions_node.end(); ++it) {
                        // capture connective and variables
                        BisonFmlaParser fmla_parser;
                        auto conn_compound = it->first.as<std::string>();
                        auto compound = std::dynamic_pointer_cast<Compound>(fmla_parser.parse(conn_compound));
                        auto conn_name = compound->connective()->symbol(); 
                        VariableCollector var_collector;
                        compound->accept(var_collector);
                        auto collected_vars = var_collector.get_collected_variables();
                        std::vector<Prop> vars;
                        for (const auto& v : collected_vars)
                            vars.push_back(*v);
                        _connectives[conn_name] = compound->connective();
                        _props[conn_name] = vars;

                        // parse sequents, for each connective
                        auto conn_seqs = it->second;//parser.hard_require(root, SEQUENTS_TITLE);
                        std::vector<NdSequent<std::set>> conn_parsed_sequents;
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
                                }
                                seq_places_sets.push_back(fmlas_in_place);
                            }
                            conn_parsed_sequents.push_back(NdSequent<std::set>(seq_places_sets));
                        }
                        _sequents_per_connective[conn_name] = conn_parsed_sequents;
                        
                        spdlog::info("Determinizing " + conn_name + "...");

                        AppsFacade apps_facade;
                        auto table = apps_facade.determinize_truth_table(nvalues, _props[conn_name], 
                                *_connectives[conn_name],
                                _judgements, _sequents_per_connective[conn_name]);
                        auto table_print = table.print(_val_to_str).str();
                        spdlog::info("The table for " + conn_name + " is" + "\n" + table_print);
                    }
                } catch (ParseException& pe) {
                    spdlog::critical(pe.message());
                }
            };
    };

};

#endif
