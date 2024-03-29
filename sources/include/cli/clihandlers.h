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
#include "core/semantics/genmatrix.h"
#include "core/printers/factory.h"
#include "apps/apps_reports.h"

namespace ltsy {

    class CLIHandler {
        protected:
            std::map<std::string, std::string> _tex_translation;
    };

    class TTAxiomatizerCLIHandler {
        private:
            const std::string SEMANTICS_TITLE = "semantics";
            const std::string LATEX_TITLE = "latex";
            const std::string SEMANTICS_VALUES_TITLE = "values";
            const std::string SEMANTICS_JUDGMENTS_TITLE = "judgements";
            const std::string TRUTH_TABLE_TITLE = "truth-tables";
            std::map<int, std::string> _val_to_str;
            std::map<std::string, int> _str_to_val;
            const std::string TRUTH_TABLES_NAME_TITLE = "name";
            const std::string TEMPLATE_TITLE = "template";
            std::map<std::string, std::shared_ptr<Connective>> _connectives;
            std::map<std::string, std::vector<std::shared_ptr<Prop>>> _props;
            std::map<std::string, std::string> _tex_translation;
        public:

            std::string get_default_template(Printer::PrinterType output_type, unsigned int dimension=4) {
                std::string temp;
                switch(output_type) {
                    case Printer::PrinterType::PLAIN:
                        temp += "Check the result below:\n";
                        temp += "{\% for connective, axioms in axiomatizations \%}";
                        temp += "Axiomatizations for (|{ connective }|):\n";
                        temp += "    {\% for axiom in axioms \%}\n";
                        temp += "- (|{ axiom }|)";
                        temp += "    {\% endfor \%}\n";
                        temp += "{\% endfor \%}";
                        break;
                    case Printer::PrinterType::LATEX:
                        temp += "\\documentclass{article}\n";
                        temp += "\\usepackage[utf8]{inputenc}\n";
                        temp += "\\usepackage[english]{babel}\n";
                        temp += "\\usepackage{booktabs}\n";
                        temp += "\\usepackage{amsmath}\n";
                        temp += "\\usepackage{float}\n";
                        temp += "\\newcommand{\\bCon}[" + std::to_string(dimension) + "]{";
                        unsigned int index=1;
                        dimension = dimension % 2 ? dimension : dimension + 1;
                        for (auto i = 1; i < dimension; i+=2) {
                            temp += "\\frac{#" + 
                                std::to_string(i+1) + "}{#" + std::to_string(i) + "}";
                            if (i < dimension/2)
                                temp += "{|}";
                        }
                        temp += "}";
                        temp += "\\begin{document}\n";
                        temp += "\\tableofcontents\n";
                        temp += "    \\begin{center}\n";
                        temp += "{\% for connective, content in axiomatizations \%}";
                        temp += "\\section{Axiomatization of $(|{ connective }|)$ }\n";
                        temp += "\\subsection{Table}\n";
                        temp += "(|{ content.table }|)\n";
                        temp += "\\subsection{Axioms}\n";
                        temp += "    {\% for axiom in content.axioms \%}\n";
                        temp += "\\[\n";
                        temp += "\\bCon";
                        temp += "        {\% for fmlas in axiom \%}\n";
                        temp += "{\\text{ (|{ fmlas }|) }}";
                        temp += "        {\% endfor \%}\n";
                        temp += "\\]";
                        temp += "    {\% endfor \%}\n";
                        temp += "{\% endfor \%}\n";
                        temp += "    \\end{center}\n";
                        temp += "\\end{document}";
                        break;
                }
                return temp;
            }

            void handle(const std::string& yaml_path, 
                    Printer::PrinterType output_type, 
                    bool verbose=true,
                    std::optional<std::string> template_path=std::nullopt,
                    std::optional<std::string> dest_path=std::nullopt) {
                YAMLCppParser parser;
                nlohmann::json result_data;
                result_data["axiomatizations"] = {};
                result_data["tables"] = {};
                try {
                    auto root = parser.load_from_file(yaml_path);
                    auto semantics_node = parser.hard_require(root, SEMANTICS_TITLE);
                    // get latex
                    if (auto latex_node = root[LATEX_TITLE]) {
                        for (auto it = latex_node.begin(); it != latex_node.end(); ++it)
                           _tex_translation[it->first.as<std::string>()] = it->second.as<std::string>(); 
                    }
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
                        auto fmla_parser = parser.make_fmla_parser(tt_node.first);
                        auto conn_compound = tt_node.first.as<std::string>();
                        auto compound = std::dynamic_pointer_cast<Compound>(fmla_parser->parse(conn_compound));
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
                                compound->connective()->arity(), _str_to_val, conn_name);
                        auto table_print = tt.print(_val_to_str).str();
                        if (verbose)
                            spdlog::info("Input table for connective " + conn_name + " is\n" + table_print);
                        AppsFacade apps_facade;
                        auto sequents = apps_facade.axiomatize_truth_table(real_values,
                                *compound->connective(),
                                vars,
                                compound,
                                judgements_in_semantics,
                                jvc,
                                tt);

                        tt.set_values_names(_val_to_str);

                        PrinterFactory printer_factory;
                        auto printer = printer_factory.make_printer(output_type, _tex_translation);

                        auto tt_name_trans = tt.name();
                        if (output_type == Printer::PrinterType::LATEX) {
                            auto itt = _tex_translation.find(tt_name_trans);
                            if (itt != _tex_translation.end())
                                tt_name_trans = itt->second;
                        }

                        std::string out = "";
                        result_data["axiomatizations"][tt_name_trans] = 
                        {{"axioms", std::vector<std::vector<std::string>>{}},
                         {"table", printer->print(tt)}
                        };
                        for (const auto& seq : sequents) {
                            out += "- " + seq.to_string() + "\n"; 
                            result_data["axiomatizations"][tt_name_trans]["axioms"].push_back(printer->print(seq));
                        }
                        if (verbose)
                            spdlog::info("Here is an axiomatization for " + tt_name_trans + ", with " 
                                        + std::to_string(sequents.size()) + " sequent(s):\n" + out);
                    }
                    // get template if exists
                    std::string template_source;
                    if (template_path) {
                        template_source = *template_path;
                    } else if (auto temp = root[TEMPLATE_TITLE]) {
                        template_source = temp.as<std::string>(); 
                    } else {
                        template_source = get_default_template(output_type);
                    }
                    // generate report
                    AppReport report;
                    auto r = report.produce(template_source, result_data, template_path.has_value(), dest_path);
                    std::cout << r << std::endl;
                    if (verbose)
                        spdlog::info("Done.");
                } catch (ParseException& pe) {
                    spdlog::critical(pe.message());
                } catch (YAML::ParserException& ye) {
                    spdlog::critical(ye.what());
                }
            }
    };

    class TTSymmetricalDualizerCLIHandler {
        private:
            const std::string LATEX_TITLE = "latex";
            const std::string SEMANTICS_VALUES_TITLE = "values";
            const std::string TTS_TITLE = "truth-tables";
            const std::string MOVEMENTS_TITLE = "movements";
            const std::string TRANSFORMATIONS_TITLE = "transformations";
            const std::string COMPOUND_TITLE = "compound";
            const std::string TEMPLATE_TITLE = "template";
            const std::string COMPONENTS_TITLE = "components";
            std::map<std::string, std::string> _tex_translation;
            std::map<int, std::string> _val_to_str;
            std::map<std::string, int> _str_to_val;
        public:

            std::string get_default_template(
                    Printer::PrinterType output_type, unsigned int dimension = 4) {
                std::string temp;
                switch(output_type) {
                    case Printer::PrinterType::LATEX: {
                        temp += "\\documentclass{article}\n";
                        temp += "\\usepackage[utf8]{inputenc}\n";
                        temp += "\\usepackage[english]{babel}\n";
                        temp += "\\usepackage{booktabs}\n";
                        temp += "\\usepackage{hyperref}\n";
                        temp += "\\usepackage{amsmath}\n";
                        temp += "\\usepackage{float}\n";
                        temp += "\\usepackage{graphicx}\n";
                        temp += "\\newcommand{\\bCon}[" + std::to_string(dimension) + "]{";
                        unsigned int index=1;
                        //dimension = dimension % 2 ? dimension : dimension + 1;
                        temp += "\\scalebox{1.5}{$\n";
                        temp += "\\begin{smallmatrix}\n";
                        for (auto i = 2; i <= dimension; i+=2) {
                            temp += "#"+std::to_string(i);
                            if (i != dimension) temp += "&\\mid&";
                        }
                        temp += "\\\\\n";
                        temp += "\\midrule\n";
                        for (auto i = 1; i < dimension; i+=2) {
                            temp += "#"+std::to_string(i);
                            if (i != dimension-1) temp += "&\\mid&";
                        }
                        temp += "\\end{smallmatrix}\n";
                        temp += "$}";
                        temp += "}";
                        temp += "\\begin{document}\n";
                        temp += "\\tableofcontents\n";
                        temp += "    \\begin{center}\n";
                        temp += "{\% for ttname, transformations in transfs \%}";
                        temp += "   \\section{(|{ ttname }|)}\n";
                        temp += "   {\% for tname, components in transformations \%}";
                        temp += "       \\subsection{(|{ ttname }|)((|{ tname }|))}\n";
                        temp += "       (|{ components.table }|)\n"; 
                        temp += "       {\% for axiom in components.calculus \%}\n";
                        temp += "(|{ axiom.name }|)\n";
                        temp += "\\[\n";
                        temp += "\\bCon";
                        temp += "           {\% for fmlas in axiom.schema \%}\n";
                        temp += "{\\text{ (|{ fmlas }|) }}";
                        temp += "           {\% endfor \%}\n";
                        temp += "\\]";
                        temp += "       {\% endfor \%}\n";
                        temp += "   {\% endfor \%}";
                        temp += "{\% endfor \%}";
                        temp += "    \\end{center}\n";
                        temp += "\\end{document}";
                    break;}
                    case Printer::PrinterType::PLAIN:
                    break;
                }
                return temp;
            }

            void handle(const std::string& yaml_path, 
                    Printer::PrinterType output_type, 
                    bool verbose=true,
                    std::optional<std::string> template_path=std::nullopt,
                    std::optional<std::string> dest_path=std::nullopt) {
                YAMLCppParser parser;
                nlohmann::json result_data;
                try {
                    auto root = parser.load_from_file(yaml_path);
                    // get latex
                    if (auto latex_node = root[LATEX_TITLE]) {
                        for (auto it = latex_node.begin(); it != latex_node.end(); ++it)
                           _tex_translation[it->first.as<std::string>()] = it->second.as<std::string>(); 
                    }
                    // values
                    auto values_node = parser.hard_require(root, SEMANTICS_VALUES_TITLE);
                    auto values = values_node.as<std::vector<std::string>>();
                    std::set<int> real_values;
                    auto nvalues = values.size();
                    for (int i {0}; i < nvalues; ++i) {
                        _str_to_val[values[i]] = i;
                        _val_to_str[i] = values[i];
                        real_values.insert(i);
                    }
                    // movements
                    std::map<std::string, std::vector<std::pair<int, int>>> changes;
                    auto changes_node = parser.hard_require(root, MOVEMENTS_TITLE);
                    for (auto it_mv = changes_node.begin(); it_mv != changes_node.end(); ++it_mv) {
                        auto name = it_mv->first.as<std::string>();
                        auto moves = it_mv->second;
                        changes[name] = std::vector<std::pair<int,int>>();
                        for (auto it_moves = moves.begin(); it_moves != moves.end(); ++it_moves) {
                            std::vector<int> moves_pair = it_moves->as<std::vector<int>>();
                            std::pair<int, int> p {moves_pair[0], moves_pair[1]};
                            changes[name].push_back(p);
                        }
                    }
                    // transformations
                    std::map<std::string, SymmetricalCalculiDualization::Transformation> transformations;
                    auto transf_node = parser.hard_require(root, TRANSFORMATIONS_TITLE);
                    for (auto it_tf = transf_node.begin(); it_tf != transf_node.end(); ++it_tf) {
                        auto name = it_tf->first.as<std::string>();
                        auto transf = it_tf->second;
                        SymmetricalCalculiDualization::Transformation transformation;
                        transformation.name = name;
                        for (auto it_transf = transf.begin(); it_transf != transf.end(); ++it_transf) {
                            auto what_moves_name = it_transf->first.as<std::string>();
                            if (what_moves_name != COMPOUND_TITLE and what_moves_name != COMPONENTS_TITLE)
                                throw std::invalid_argument("Unknown specification of transformation: " + what_moves_name);
                            SymmetricalCalculiDualization::WhatMoves what_moves =
                                what_moves_name == COMPOUND_TITLE ? SymmetricalCalculiDualization::WhatMoves::COMPOUND :
                                SymmetricalCalculiDualization::WhatMoves::COMPONENTS;
                            auto changes_spec = it_transf->second;
                            SymmetricalCalculiDualization::Movement movement;
                            movement.what_moves = what_moves;
                            for (auto it_change = changes_spec.begin(); it_change != changes_spec.end(); ++it_change) {
                                auto changes_pos = changes[it_change->as<std::string>()];
                                movement.movement = changes_pos;
                            }
                            transformation.movements.push_back(movement);
                        }
                        transformations[name] = transformation;
                    }
                    spdlog::debug("Quantity of transformations: " + std::to_string(transformations.size()));
                    SymmetricalCalculiDualization dualizer {transformations};
                    PrinterFactory printer_factory;
                    auto printer = printer_factory.make_printer(output_type, _tex_translation);
                    // tts
                    auto tts_node = parser.hard_require(root, TTS_TITLE);
                    for (auto it_tt = tts_node.begin(); it_tt != tts_node.end(); ++it_tt) {
                        // capture connective and variables
                        auto [compound, collected_vars] = parser.parse_connective_representant(it_tt->first);
                        spdlog::info("Processing " + compound->connective()->symbol());
                        // parse tt and interpret connectives
                        NDTruthTable tt = parser.parse_nd_truth_table(it_tt->second, real_values,
                                compound->connective()->arity(), _str_to_val, compound->connective()->symbol());
                        std::cout << tt.print().str() << std::endl;
                        tt.set_fmla(compound);
                        auto transformation_results = dualizer.create_oppositions(std::make_shared<NDTruthTable>(tt));
                        nlohmann::json response_name;
                        for (auto& [tt, calculus, transformation] : transformation_results) {
                            spdlog::info("Printing transformation " + transformation.name);
                            tt.set_values_names(_val_to_str);
                            response_name[transformation.name]["table"] = printer->print(tt);
                            response_name[transformation.name]["calculus"] = {};
                            for (auto r : calculus.rules()) {
                                nlohmann::json rule_json ({
                                    {"name", r.name()},        
                                    {"schema", printer->print(r.sequent())},        
                                });
                                response_name[transformation.name]["calculus"].push_back(rule_json);
                            }
                        }
                        result_data["transfs"][tt.name()] = response_name;
                    }
                    // report
                    // get template if exists
                    std::string template_source;
                    if (template_path) {
                        template_source = *template_path;
                    } else if (auto temp = root[TEMPLATE_TITLE]) {
                        template_source = temp.as<std::string>(); 
                    } else {
                        template_source = get_default_template(output_type);
                    }
                    // generate report
                    AppReport report;
                    auto r = report.produce(template_source, result_data, template_path.has_value(), dest_path);
                    std::cout << r << std::endl;
                } catch (ParseException& pe) {
                    spdlog::critical(pe.message());
                } catch (YAML::Exception& ye) {
                    spdlog::critical(ye.what());
                }
            }
    };


    class TTDeterminizerCLIHandler {

        private:

            const std::string SEMANTICS_TITLE = "semantics";
            const std::string SEMANTICS_VALUES_TITLE = "values";
            const std::string SEMANTICS_JUDGEMENTS_TITLE = "judgements";
            const std::string CONDITIONS_TITLE = "conditions";
            const std::string TRUTH_TABLES_TITLE = "truth-tables";
            const std::string TRUTH_TABLES_BASE_TITLE = "base";
            const std::string TRUTH_TABLES_NAME_TITLE = "name";
            const std::string TEMPLATE_TITLE = "template";
            const std::string LATEX_TITLE = "latex";

            std::map<int, std::string> _val_to_str;
            std::map<std::string, int> _str_to_val;
            std::vector<CognitiveAttitude> _judgements;
            std::map<std::string, std::shared_ptr<Connective>> _connectives;
            std::map<std::string, std::vector<Prop>> _props;
            std::map<std::string, NDTruthTable> _start_tts;
            std::map<std::string, std::vector<NdSequent<std::set>>> _sequents_per_connective;

            std::map<std::string, std::string> _tex_translation;

        public:

            std::string get_default_template(const std::vector<std::string>& keys, 
                    Printer::PrinterType output_type) {
                std::string temp;
                switch(output_type) {
                    case Printer::PrinterType::PLAIN:
                        temp += "Check the result below:\n";
                        for (const auto& k : keys) {
                            temp += "(|{ " + k + " }|)\n";
                        }
                        break;
                    case Printer::PrinterType::LATEX:
                        temp += "\\documentclass{article}\n";
                        temp += "\\usepackage[utf8]{inputenc}\n";
                        temp += "\\usepackage[english]{babel}\n";
                        temp += "\\usepackage{booktabs}\n";
                        temp += "\\begin{document}\n";
                        temp += "    \\begin{center}\n";
                        for (const auto& k : keys) {
                            temp += "        (|{ " + k + " }|)\n";
                            temp += "        \\vspace{1em}\n";
                        }
                        temp += "    \\end{center}\n";
                        temp += "\\end{document}";
                        break;
                }
                return temp;
            }

            void handle(const std::string& yaml_path, 
                    Printer::PrinterType output_type, 
                    bool verbose=true,
                    std::optional<std::string> template_path=std::nullopt,
                    std::optional<std::string> dest_path=std::nullopt) {
                YAMLCppParser parser;
                nlohmann::json result_data;
                try {
                    auto root = parser.load_from_file(yaml_path);
                    // get latex
                    if (auto latex_node = root[LATEX_TITLE]) {
                        for (auto it = latex_node.begin(); it != latex_node.end(); ++it)
                           _tex_translation[it->first.as<std::string>()] = it->second.as<std::string>(); 
                    }
                    // get semantics
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
                    // parse tts
                    std::vector<std::string> tt_names;
                    auto truth_tables_node = parser.hard_require(root, TRUTH_TABLES_TITLE);
                    for (auto it_tt = truth_tables_node.begin(); it_tt != truth_tables_node.end(); ++it_tt) {
                        // capture connective and variables
                        auto [compound, collected_vars] = parser.parse_connective_representant(it_tt->first);
                        auto conn_name = compound->connective()->symbol(); 
                        std::vector<Prop> vars;
                        for (const auto& v : collected_vars)
                            vars.push_back(*v);
                        _connectives[conn_name] = compound->connective();
                        _props[conn_name] = vars;                   
                        // parse base table
                        std::optional<NDTruthTable> base_tt = std::nullopt;
                        if (auto base_node = it_tt->second[TRUTH_TABLES_BASE_TITLE]) {
                            NDTruthTable tt = parser.parse_nd_truth_table(base_node, real_values,
                                    compound->connective()->arity(), _str_to_val);
                            base_tt = std::optional<NDTruthTable>(tt);
                        }
                        // parse conditions
                        auto conditions_node = parser.hard_require(it_tt->second, CONDITIONS_TITLE);
                        std::vector<NdSequent<std::set>> conn_parsed_sequents;
                        for (const auto& seq_node : conditions_node) {
                            // each seq_node is a list of lists
                            std::vector<FmlaSet> seq_places_sets;
                            for (const auto& seq_place_node : seq_node) {
                                FmlaSet fmlas_in_place;
                                auto fmlas_str = seq_place_node.as<std::vector<std::string>>();
                                for (const auto& fmla_str : fmlas_str) {
                                    auto fmla_parser = parser.make_fmla_parser(seq_place_node);
                                    auto parsed_fmla = fmla_parser->parse(fmla_str);
                                    fmlas_in_place.insert(parsed_fmla);
                                }
                                seq_places_sets.push_back(fmlas_in_place);
                            }
                            conn_parsed_sequents.push_back(NdSequent<std::set>(seq_places_sets));
                        }
                        _sequents_per_connective[conn_name] = conn_parsed_sequents;
                        
                        if (verbose)
                            spdlog::info("Determinizing " + conn_name + "...");

                        AppsFacade apps_facade;
                        auto table = apps_facade.determinize_truth_table(nvalues, _props[conn_name], 
                                *_connectives[conn_name],
                                _judgements, _sequents_per_connective[conn_name], base_tt);
                        if (auto tt_name = it_tt->second[TRUTH_TABLES_NAME_TITLE]) {
                            table.set_name(tt_name.as<std::string>());
                        } else {
                            table.set_name(conn_name);
                        }
                        table.set_values_names(_val_to_str);
                        
                        PrinterFactory printer_factory;
                        auto printer = printer_factory.make_printer(output_type, _tex_translation);
                        auto table_print = printer->print(table);

                        result_data[table.name()] = table_print;

                        if (verbose)
                            spdlog::info("The table for " + table.name() + " is" + "\n" + table_print);

                        tt_names.push_back(table.name());
                    }
                    // get template if exists
                    std::string template_source;
                    if (template_path) {
                        template_source = *template_path;
                    } else if (auto temp = root[TEMPLATE_TITLE]) {
                        template_source = temp.as<std::string>(); 
                    } else {
                        template_source = get_default_template(tt_names, output_type);
                    }
                    // generate report
                    AppReport report;
                    auto r = report.produce(template_source, result_data, template_path.has_value(), dest_path);
                    std::cout << r << std::endl;
                } catch (ParseException& pe) {
                    spdlog::critical(pe.message());
                } catch (YAML::Exception& ye) {
                    spdlog::critical(ye.what());
                }
            };
    };

    class AnalyticProofSearchCLIHandler {
        public:
            void handle(const std::string& yaml_path, 
                    Printer::PrinterType output_type, 
                    bool verbose=true,
                    std::optional<std::string> template_path=std::nullopt,
                    std::optional<std::string> dest_path=std::nullopt) {
                YAMLCppParser parser;
                nlohmann::json result_data;
                std::vector<MultipleConclusionRule> calculus_rules;
                const std::string SEQUENT_DSET_CORRESPOND_TITLE = "sequent_dset_correspondence";

                try {
                    auto root = parser.load_from_file(yaml_path);
                    auto simplify_max_level = parser.hard_require(root, "simplify_max_level").as<int>();
                    auto simplify_overlap = parser.hard_require(root, "simplify_overlap").as<bool>();
                    auto simplify_dilution = parser.hard_require(root, "simplify_dilution").as<bool>();
                    auto simplify_by_cuts = parser.hard_require(root, "simplify_by_cuts").as<bool>();
                    auto simplify_by_subrule_deriv = parser.optional_require<unsigned int>(root, "simplify_by_subrule_deriv", false);
                    auto prem_conc_corr_node = parser.hard_require(root, "prem_conc_correspondence");
                    auto seq_dset_corr = parser.hard_require(root, SEQUENT_DSET_CORRESPOND_TITLE)
                        .as<std::vector<int>>();
                    std::vector<std::pair<int,int>> prem_conc_corr;
                    for (auto it = prem_conc_corr_node.begin(); it != prem_conc_corr_node.end(); it++) {
                        auto prem_conc = it->as<std::vector<int>>();
                        prem_conc_corr.push_back({prem_conc[0], prem_conc[1]});
                    }
                    auto calculus_node = parser.hard_require(root, "calculus");
                    for (auto it = calculus_node.begin(); it != calculus_node.end(); ++it) {
                        auto name =  it->first.as<std::string>();
                        auto sequent =  parser.parse_nd_sequent(it->second);
                        MultipleConclusionRule rule {name, *sequent, prem_conc_corr};
                        calculus_rules.push_back(rule);
                    }
                    MultipleConclusionCalculus calculus {calculus_rules};

                    AppsFacade apps_facade;
                    calculus = apps_facade.simplify_mult_conc_axiomatizer(calculus, prem_conc_corr, seq_dset_corr,
                            simplify_overlap, simplify_dilution, simplify_by_cuts, simplify_by_subrule_deriv);

                    spdlog::info("Below are the result simplification by overlap, dilution, cut");
                    auto set_rules = calculus.rules();
                    for (auto r : set_rules) {
                        std::cout << r.sequent() << std::endl;
                    }

                    auto analiticity_formulas_node = parser.hard_require(root, "analyticity_formulas");
                    FmlaSet analiticity_formulas = parser.parse_fmla_set(analiticity_formulas_node);
                    // simplification
                    spdlog::info("Below are the result of the simplification attempt");
                    const auto [simp_calculus, removed_rules, level] = simplify_by_derivation(calculus, analiticity_formulas, 0, simplify_max_level);
                    for (const auto& removed_rule : removed_rules) {
                        const auto& [rule, derivation] = removed_rule;
                        std::cout << "derived " << rule.name() << std::endl;
                        auto derivtree = derivation->print().str();
                        std::cout << derivtree << std::endl;
                    }
                    spdlog::info("Final calculus:");
                    set_rules = simp_calculus.rules();
                    for (auto r : set_rules) {
                        std::cout << r.name() << ":=" << r.sequent() << std::endl;
                    }
                    spdlog::info("Below are the requested derivations in the original calculus");
                    // derivations
                    if (auto derive_node = root["derive"]) {
                        for (auto it = derive_node.begin(); it != derive_node.end(); ++it) {
                            auto name =  it->first.as<std::string>();
                            auto sequent =  parser.parse_nd_sequent(it->second);
                            MultipleConclusionRule rule {name, *sequent, prem_conc_corr};
                            auto derivation = calculus.derive(rule, analiticity_formulas);
			    if (derivation != nullptr) {
				    if (derivation->closed)
					std::cout << name + " is derivable." << std::endl;
				    else
					std::cout << name + " is underivable." << std::endl;
			    }
                            auto derivtree = derivation->print().str();
                            std::cout << derivtree << std::endl;
                        }
                    }
                    
                } catch (ParseException& pe) {
                    spdlog::critical(pe.message());
                } catch (YAML::Exception& ye) {
                    spdlog::critical(ye.what());
                }
            }

            struct DerivedRule {
                MultipleConclusionRule rule;
                std::shared_ptr<MultipleConclusionCalculus::DerivationTreeNode> derivation;
                DerivedRule(const decltype(rule)& _rule, const decltype(derivation)& _derivation):
                    rule {_rule}, derivation {_derivation} {/* empty */}
                bool operator<(const DerivedRule& other) const { return this->rule < other.rule; }
            };

            std::tuple<MultipleConclusionCalculus, std::set<DerivedRule>, unsigned int>
                simplify_by_derivation(const MultipleConclusionCalculus& calculus, const FmlaSet& phi,
                        unsigned int depth, std::optional<unsigned int> max_depth = std::nullopt) const {
                if (calculus.size() == 0)
                    return {calculus, {}, depth};
                if (max_depth and depth >= *max_depth)
                    return std::tuple<MultipleConclusionCalculus, 
                           std::set<DerivedRule>, unsigned int>(calculus, {}, *max_depth);
                MultipleConclusionCalculus simplified_calc = calculus;
                std::set<DerivedRule> rules_removed = {};
                auto rules = calculus.rules();
                int  max_depth_so_far = depth;
                for (auto i {0}; i < rules.size(); ++i) {
                    auto rules_simp = rules;
                    rules_simp.erase(rules_simp.begin() + i);
                    MultipleConclusionCalculus simp_calc {rules_simp};
                    auto derivation = simp_calc.derive(rules[i], phi);
                    if (derivation->closed) {
                        spdlog::debug("Derived " + rules[i].name() + ": " + rules[i].sequent().to_string() + 
                                " in depth " + std::to_string(depth) + " using " + 
                                std::to_string(simp_calc.size()) + " rules ");
                        const auto [rec_calculus, rec_rules_rem, rec_depth] = 
                            simplify_by_derivation(simp_calc, phi, depth + 1, max_depth);
                        if (rec_depth > max_depth_so_far) {
                            max_depth_so_far = rec_depth;
                            simplified_calc = rec_calculus; 
                            rules_removed = rec_rules_rem;
                            rules_removed.insert(DerivedRule{rules[i], derivation});
                            if (max_depth and rec_depth >= *max_depth)
                                break;
                        }
                    }
                }
                return std::make_tuple(simplified_calc, rules_removed, max_depth_so_far);
            }
            
    };

    /* Handler for the sequent rule soundness app.
     * */
    class CloneMembershipCLIHandler {

        private:
            const std::string BASE_FUNCTIONS = "base";
            const std::string SEARCH_FUNCTIONS = "search";
            const std::string VALUES = "values";
            const std::string MAX_DEPTH = "max_depth";
            std::map<std::string, std::string> _tex_translation;

        public:

            void handle(const std::string& yaml_path, 
                    Printer::PrinterType output_type, 
                    bool verbose=true,
                    std::optional<std::string> template_path=std::nullopt,
                    std::optional<std::string> dest_path=std::nullopt) {
                YAMLCppParser parser;
                nlohmann::json result_data;
                try {
                    auto root = parser.load_from_file(yaml_path);
                    auto max_depth_opt = parser.optional_require<int>(root, MAX_DEPTH, 0);
                    auto functions_root = parser.hard_require(root, BASE_FUNCTIONS);

                    std::map<int, std::string> val_to_str;
                    std::map<std::string, int> str_to_val;

                    auto values = parser.hard_require(root, VALUES).as<std::vector<std::string>>();
                    std::set<int> real_values;
                    auto nvalues = values.size();
                    for (int i {0}; i < nvalues; ++i) {
                        str_to_val[values[i]] = i;
                        val_to_str[i] = values[i];
                        real_values.insert(i);
                    }
                    std::vector<NDTruthTable> functions = parser.parse_nd_truth_table_vector(functions_root, real_values, str_to_val);

                    auto search_functions_root = parser.hard_require(root, SEARCH_FUNCTIONS);
                    std::vector<NDTruthTable> search_functions = parser.parse_nd_truth_table_vector(search_functions_root, real_values, str_to_val);

                    AppsFacade apps_facade;

                    if (max_depth_opt)
                        spdlog::info("Notice: max depth of search is " + std::to_string(*max_depth_opt));

                    for (auto& sf : search_functions) {
                        spdlog::info("Searching for " + sf.name());
                        auto [opt_result, opt_clone] = apps_facade.clone_membership(nvalues, functions, sf, max_depth_opt);
                        if (not opt_result) 
                            spdlog::info("Not in clone.");
                        else {
                            spdlog::info("In clone! How:");
                            for (auto r : *opt_result) {
                                std::cout << *r.fmla() << std::endl;
                                std::cout << r.print().str() << std::endl;
                            }
                        }
                            
                    }
        
                } catch (ParseException& pe) {
                    spdlog::critical(pe.message());
                } catch (YAML::Exception& ye) {
                    spdlog::critical(ye.what());
                }
            }
    };    
    
    /* Handler for the sequent rule soundness app.
     * */
    class MonadicMatrixAxiomatizerCLIHandler {

        private:
            const std::string RULES_TITLE = "rules";
            const std::string DISCR_TITLE = "discriminator";
            const std::string INFER_COMPLEMENTS_TITLE = "infer_complements";
            const std::string TEMPLATE_TITLE = "template";
            const std::string SIMPLIFY_OVERLAP = "simplify_overlap";
            const std::string SIMPLIFY_DILUTION = "simplify_dilution";
            const std::string SIMPLIFY_SUBRULES = "simplify_subrules";
            const std::string SIMPLIFY_SUBRULES_DERIV = "simplify_subrules_deriv";
            const std::string SIMPLIFY_DERIVATION = "simplify_derivation";
            const std::string SIMPLIFY_CUTS = "simplify_by_cuts";
            const std::string DERIVE = "derive";
            const std::string SEQUENT_DSET_CORRESPOND_TITLE = "sequent_dset_correspondence";
            const std::string PREM_CONC_CORRESPOND_TITLE = "premises_conclusions_correspondence";
            const std::string LATEX_TITLE = "latex";
            const std::string B_ENTAILMENT_OUTPUT = "b_entailment_output";
            std::map<std::string, std::string> _tex_translation;

        public:
            std::string get_default_template(Printer::PrinterType output_type, unsigned int dimension=4, bool b_entailment_output=false) {
                std::string temp;
                switch(output_type) {
                    case Printer::PrinterType::YAML:
			temp += "calculus:\n";
                        temp += "{\% if exists(\"axiomatization\") \%}";
                        temp += "{\% for groups, schemas in axiomatization \%}";
                        temp += "{\% for schema in schemas \%}";
                        temp += "  (|{ schema.name }|): [";
                        temp += "{\% for fmlas in schema.schema \%}";
			temp += "(|{ fmlas }|)";
			temp += "{\% if not loop.is_last \%}";
			temp += ",";
			temp += "{\% endif \%}";
                        temp += "{\% endfor \%}]\n";
                        temp += "{\% endfor \%}";
                        temp += "{\% endfor \%}";
                        temp += "{\% else \%}"; //empty calculus
                        temp += "{\% endif \%}";
			temp += "analyticity_formulas: [(|{ analyticity_formulas }|)]\n"; 
			temp += "simplify_overlap: false\n";
			temp += "simplify_dilution: false\n";
			temp += "simplify_by_cuts: false\n";
			temp += "simplify_by_subrule_deriv: 0\n";
			temp += "prem_conc_correspondence: (|{ prem_conc_corr }|)\n";
			temp += "sequent_dset_correspondence: (|{ seq_dset_corr  }|)\n";
			temp += "simplify_max_level: 0\n";
			temp += "derive:\n";
			temp += "    # write queries as rulename: rule-description";
                        break;
                    case Printer::PrinterType::PLAIN:
                        temp += "Check the result below:\n";
                        temp += "{\% if exists(\"axiomatization\") \%}";
                        temp += "{\% for groups, schemas in axiomatization \%}";
                        temp += "Group of schemas (|{ groups }|):\n";
                        temp += "    {\% for schema in schemas \%}\n";
                        temp += "- (|{ schema.name }|)";
                        temp += "- (|{ schema.schema }|)";
                        temp += "    {\% endfor \%}\n";
                        temp += "{\% endfor \%}";
                        temp += "{\% else \%}";
                        temp += "Empty calculus.";
                        temp += "{\% endif \%}";
                        break;
                    case Printer::PrinterType::LATEX:
                        temp += "\\documentclass{article}\n";
                        temp += "\\usepackage[utf8]{inputenc}\n";
                        temp += "\\usepackage[english]{babel}\n";
                        temp += "\\usepackage{booktabs}\n";
                        temp += "\\usepackage{amsmath}\n";
                        temp += "\\usepackage{amssymb}\n";
                        temp += "\\usepackage{float}\n";
                        temp += "\\usepackage{graphicx}\n";
                        temp += "\\newcommand{\\Set}[1]{\n";
                        temp += "\\{#1\\}";
                        temp += "}\n";
			if (b_entailment_output) {
				temp += "\\newcommand{\\bCon}[" + std::to_string(dimension) + "]{";
				unsigned int index=1;
				//dimension = dimension % 2 ? dimension : dimension + 1;
				temp += "\\scalebox{1.5}{$\n";
				temp += "\\begin{smallmatrix}\n";
				for (auto i = 2; i <= dimension; i+=2) {
				    temp += "#"+std::to_string(i);
				    if (i != dimension) temp += "&;&";
				}
				temp += "\\\\\n";
				temp += "\\midrule\n";
				for (auto i = 1; i < dimension; i+=2) {
				    temp += "#"+std::to_string(i);
				    if (i != dimension-1) temp += "&;&";
				}
				temp += "\\end{smallmatrix}\n";
				temp += "$}";
				temp += "}";
			} else {
				temp += "\\newcommand{\\bCon}[" + std::to_string(dimension) + "]{";
				unsigned int index=1;
				temp += "\\scalebox{1.5}{$\n";
				temp += "\\begin{smallmatrix}\n";
				for (auto i = 1; i < dimension; i+=2) {
				    temp += "#"+std::to_string(i);
				    if (i != dimension-1) temp += "&;&";
				}
				temp += "\\\\\n";
				temp += "\\midrule\n";
				for (auto i = 2; i <= dimension; i+=2) {
				    temp += "#"+std::to_string(i);
				    if (i != dimension) temp += "&;&";
				}
				temp += "\\end{smallmatrix}\n";
				temp += "$}";
				temp += "}";
			}
                        // begin document
                        temp += "\\begin{document}\n";
                        temp += "\\tableofcontents\n";
                        temp += "    \\begin{center}\n";
                        // section of desigs
                        temp += "{\% for dset in dsets \%}";
                        temp += "\\Set{(|{dset}|)}";
                        temp += "{\% endfor \%}";
                        // section of truth-tables
                        temp += "\\section{Interpretation}\n";
                        temp += "{\% for conn, tt in interps \%}";
                        temp += "(|{ tt }|)";
                        temp += "{\% endfor \%}\n";
                        // schemas, one per section
                        temp += "{\% if exists(\"axiomatization\") \%}";
                        temp += "{\% for group, schemas in axiomatization \%}";
                        temp += "\\section{Schemas of $(|{ group }|)$ }\n";
                        temp += "Size: (|{ length(schemas) }|)\n";
                        temp += "    {\% for schema in schemas \%}\n";
                        temp += "(|{ schema.name }|)\n";
                        temp += "\\[\n";
                        temp += "\\bCon";
                        temp += "        {\% for fmlas in schema.schema \%}\n";
                        temp += "{\\text{ (|{ fmlas }|) }}";
                        temp += "        {\% endfor \%}\n";
                        temp += "\\]";
                        temp += "    {\% endfor \%}\n";
                        temp += "{\% endfor \%}\n";
                        temp += "{\% else \%}";
                        temp += "Empty calculus.";
                        temp += "{\% endif \%}";
                        temp += "\\end{center}\n";
                        temp += "\\end{document}";
                        break;
                }
                return temp;
            }

            void handle(const std::string& yaml_path, 
                    Printer::PrinterType output_type, 
                    bool verbose=true,
                    std::optional<std::string> template_path=std::nullopt,
                    std::optional<std::string> dest_path=std::nullopt) {
                YAMLCppParser parser;
                nlohmann::json result_data;
                try {
                    auto root = parser.load_from_file(yaml_path);

                    // get latex
                    if (auto latex_node = root[LATEX_TITLE]; output_type == Printer::PrinterType::LATEX) {
                        for (auto it = latex_node.begin(); it != latex_node.end(); ++it)
                           _tex_translation[it->first.as<std::string>()] = it->second.as<std::string>(); 
                    }

                    auto pnmatrix = parser.parse_gen_matrix(root);
                    auto disc_node = parser.hard_require(root, DISCR_TITLE);
                    auto simplify_overlap = parser.hard_require(root, SIMPLIFY_OVERLAP).as<bool>();
                    auto simplify_dilution = parser.hard_require(root, SIMPLIFY_DILUTION).as<bool>();
                    auto simplify_by_cuts = parser.hard_require(root, SIMPLIFY_CUTS).as<bool>();
                    auto simplify_subrules_derivation = parser.optional_require<unsigned int>(root, SIMPLIFY_SUBRULES_DERIV, std::nullopt);
                    auto simplify_derivation = parser.optional_require<unsigned int>(root, SIMPLIFY_DERIVATION, std::nullopt);
                    auto monadic_discriminator = parser.parse_monadic_discriminator(disc_node, pnmatrix);
                    auto seq_dset_corr = parser.optional_require<std::vector<int>>(root, SEQUENT_DSET_CORRESPOND_TITLE, std::nullopt);
                    auto prem_conc_corr_node = parser.optional_require<std::vector<std::vector<int>>>(root, PREM_CONC_CORRESPOND_TITLE, std::nullopt);

		    std::optional<std::vector<std::pair<int,int>>> prem_conc_corr = std::nullopt;
		    if (prem_conc_corr_node) {
			    prem_conc_corr = std::vector<std::pair<int,int>>{};
			    for (auto it = prem_conc_corr_node->begin(); it != prem_conc_corr_node->end(); it++) {
				//auto prem_conc = it->as<std::vector<int>>();
				prem_conc_corr->push_back({(*it)[0], (*it)[1]});
			    }
		    }

                    if (verbose)
                        spdlog::info("Input tables: \n" + pnmatrix->print());

                    AppsFacade apps_facade;
                    auto axiomatization_with_axiomatizer = apps_facade.monadic_gen_matrix_mult_conc_axiomatizer(pnmatrix, 
                            monadic_discriminator, seq_dset_corr, prem_conc_corr, 
                            simplify_overlap, simplify_dilution, simplify_subrules_derivation, 
                            simplify_derivation, simplify_by_cuts);
		    auto axiomatization = axiomatization_with_axiomatizer.first;
		    auto axiomatizer = axiomatization_with_axiomatizer.second;
                    
                    PrinterFactory printer_factory;
                    auto printer = printer_factory.make_printer(output_type, _tex_translation);

                    if (not axiomatization.empty()) {
                        result_data["axiomatization"] = {}; 
                        for (auto [k, calculus] : axiomatization) {
                            result_data["axiomatization"][k] = nlohmann::json::array();
                            for (auto r : calculus.rules()) {
                                nlohmann::json rule_json ({
                                    {"name", r.name()},        
                                    {"schema", printer->print(r.sequent())},        
                                });
                                result_data["axiomatization"][k].push_back(rule_json);
                            }
                            }                    
                    }
			
		    auto discriminator_formulas = monadic_discriminator.get_formulas();
		    std::string analyticity_formulas = "";
		    int i = 0;
		    for (const auto& fm : discriminator_formulas) {
			    analyticity_formulas += "\"" + printer->print(fm) + "\"";
			    i++;
			    if (i < discriminator_formulas.size()) analyticity_formulas += ",";
		    }
		    result_data["analyticity_formulas"] = analyticity_formulas;
		    result_data["prem_conc_corr"] = axiomatizer.prem_conc_pos_corresp();
		    result_data["seq_dset_corr"] = axiomatizer.seq_dset_corr();

                    result_data["interps"] = {};
                    for (auto [c, interp] : *pnmatrix->interpretation()) {
                        auto tt = interp->truth_table();
                        auto ttstr = printer->print(*tt);
                        result_data["interps"][_tex_translation[c]] = ttstr;
                    }

                    std::vector<std::set<std::string>> dsets_transl;
                    for (const auto& dset : pnmatrix->distinguished_sets()) {
                        std::set<std::string> dset_trans;
                        for (const auto& dsetv : dset) {
                            dset_trans.insert(pnmatrix->val_to_str()[dsetv]);
                        }
                        dsets_transl.push_back(dset_trans);
                    }
                    result_data["dsets"] = dsets_transl;

                    // if derive
                    if (auto derive_node = root[DERIVE]) {
                        auto discriminator_fmlas = monadic_discriminator.get_formulas();
                        std::set<MultipleConclusionRule> full_calculus_rules; 
                        for (auto [k, calculus] : axiomatization) {
                            auto calculus_rules = calculus.rules();
                            full_calculus_rules.insert(calculus_rules.begin(), calculus_rules.end());
                        }
                        MultipleConclusionCalculus full_calculus {
                            std::vector<MultipleConclusionRule>{full_calculus_rules.begin(), 
                                full_calculus_rules.end()}
                        };

                        for (auto it = derive_node.begin(); it != derive_node.end(); ++it) {
                            auto name =  it->first.as<std::string>();
                            auto sequent =  parser.parse_nd_sequent(it->second);
                            MultipleConclusionRule rule (name, *sequent, axiomatizer.prem_conc_pos_corresp());
                            auto derivation = full_calculus.derive(rule, discriminator_fmlas);
                            if (derivation->closed)
                                std::cout << name + " is derivable." << std::endl;
                            else
                                std::cout << name + " is underivable." << std::endl;
                            auto derivtree = derivation->print().str();
                            std::cout << derivtree << std::endl;
                        }
                    }


                    /////// templating / reporting
                    // get template if exists
                    std::string template_source;
                    if (template_path) {
                        template_source = *template_path;
                    } else if (auto temp = root[TEMPLATE_TITLE]) {
                        template_source = temp.as<std::string>(); 
                    } else {
                        template_source = get_default_template(output_type, pnmatrix->distinguished_sets().size(), false);
                    }
                    // generate report
                    AppReport report;
                    auto r = report.produce(template_source, result_data, template_path.has_value(), dest_path);
                    std::cout << r << std::endl;                   
                } catch (ParseException& pe) {
                    spdlog::critical(pe.message());
                } catch (YAML::Exception& ye) {
                    spdlog::critical(ye.what());
                }
            }
    };
    
    /* Handler for the sequent rule soundness app.
     * */
    class SequentRuleSoundnessCLIHandler {

        private:
            const std::string RULES_TITLE = "rules";
            const std::string INFER_COMPLEMENTS_TITLE = "infer_complements";
            const std::string SEQUENT_DSET_CORRESPOND_TITLE = "sequent_dset_correspondence";
            const std::string MAX_COUNTER_MODELS_TITLE = "max_counter_models";

        public:
            void handle(const std::string& yaml_path) {
                YAMLCppParser parser;
                try {
                    auto root = parser.load_from_file(yaml_path);
                    auto pnmatrix = parser.parse_gen_matrix(root);
                    std::vector<NdSequentRule<std::set>> rules;
                    auto rules_node = parser.hard_require(root, RULES_TITLE);
                    for (const auto& rule_node : rules_node) {
                        auto rule = parser.parse_nd_sequent_rule(rule_node.first.as<std::string>(), 
                                rule_node.second);
                        rules.push_back(*rule);
                    }
                    int max_counter_models = parser.hard_require(root, MAX_COUNTER_MODELS_TITLE).as<int>();
                    auto seq_dset_corr = parser.hard_require(root, SEQUENT_DSET_CORRESPOND_TITLE)
                        .as<std::vector<int>>();
                    AppsFacade apps_facade;
                    for (const auto& rule : rules) {
                        spdlog::info("Checking for rule " + rule.name() + "...");
                        try {
                            auto soundness_results = apps_facade.sequent_rule_soundness_check_gen_matrix(
                                        pnmatrix, seq_dset_corr, {rule}, max_counter_models,
                                        std::make_optional<progresscpp::ProgressBar>(70)
                                    );
                            auto result = soundness_results[rule.name()];
                            if (not result) {
                                spdlog::info("Sound.");
                            } else {
                                spdlog::info("Not sound. Consider the following configuration(s):");
                                for (const auto& ce : *result) {
                                    spdlog::info("\n" + ce.val.print(pnmatrix->val_to_str()).str());
                                }
                            }
                        } catch(std::exception e) {}
                    }
                } catch (ParseException& pe) {
                    spdlog::critical(pe.message());
                } catch (YAML::Exception& ye) {
                    spdlog::critical(ye.what());
                }
            }
    };

};

#endif
