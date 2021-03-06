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
            const std::string DERIVE = "derive";
            const std::string SEQUENT_DSET_CORRESPOND_TITLE = "sequent_dset_correspondence";
            const std::string PREM_CONC_CORRESPOND_TITLE = "premisses_conclusions_correspondence";
            const std::string LATEX_TITLE = "latex";
            std::map<std::string, std::string> _tex_translation;

        public:
            std::string get_default_template(Printer::PrinterType output_type, unsigned int dimension=4) {
                std::string temp;
                switch(output_type) {
                    case Printer::PrinterType::PLAIN:
                        temp += "Check the result below:\n";
                        temp += "{\% for groups, schemas in axiomatization \%}";
                        temp += "Group of schemas (|{ groups }|):\n";
                        temp += "    {\% for schema in schemas \%}\n";
                        temp += "- (|{ schema.name }|)";
                        temp += "- (|{ schema.schema }|)";
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
                        temp += "\\usepackage{graphicx}\n";
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
                        // begin document
                        temp += "\\begin{document}\n";
                        temp += "\\tableofcontents\n";
                        temp += "    \\begin{center}\n";
                        // section of truth-tables
                        temp += "\\section{Interpretation}\n";
                        temp += "{\% for conn, tt in interps \%}";
                        temp += "(|{ tt }|)";
                        temp += "{\% endfor \%}\n";
                        // schemas, one per section
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

                    auto pnmatrix = parser.parse_gen_matrix(root);
                    auto disc_node = parser.hard_require(root, DISCR_TITLE);
                    auto simplify_overlap = parser.hard_require(root, SIMPLIFY_OVERLAP).as<bool>();
                    auto simplify_dilution = parser.hard_require(root, SIMPLIFY_DILUTION).as<bool>();
                    auto simplify_subrules = parser.hard_require(root, SIMPLIFY_SUBRULES).as<bool>();
                    auto simplify_subrules_derivation = parser.optional_require<unsigned int>(root, SIMPLIFY_SUBRULES_DERIV);
                    auto simplify_derivation = parser.optional_require<unsigned int>(root, SIMPLIFY_DERIVATION);
                    auto monadic_discriminator = parser.parse_monadic_discriminator(disc_node, pnmatrix);
                    auto seq_dset_corr = parser.hard_require(root, SEQUENT_DSET_CORRESPOND_TITLE)
                        .as<std::vector<int>>();
                    auto prem_conc_corr_node = parser.hard_require(root, PREM_CONC_CORRESPOND_TITLE);
                    std::vector<std::pair<int,int>> prem_conc_corr;
                    for (auto it = prem_conc_corr_node.begin(); it != prem_conc_corr_node.end(); it++) {
                        auto prem_conc = it->as<std::vector<int>>();
                        prem_conc_corr.push_back({prem_conc[0], prem_conc[1]});
                    }

                    if (verbose)
                        spdlog::info("Input tables: \n" + pnmatrix->print());

                    AppsFacade apps_facade;
                    auto axiomatization = apps_facade.monadic_gen_matrix_mult_conc_axiomatizer(pnmatrix, 
                            monadic_discriminator, seq_dset_corr, prem_conc_corr, 
                            simplify_overlap, simplify_dilution, simplify_subrules, simplify_subrules_derivation, 
                            simplify_derivation);
                    
                    PrinterFactory printer_factory;
                    auto printer = printer_factory.make_printer(output_type, _tex_translation);

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

                    result_data["interps"] = {};
                    for (auto [c, interp] : *pnmatrix->interpretation()) {
                        auto tt = interp->truth_table();
                        auto ttstr = printer->print(*tt);
                        result_data["interps"][_tex_translation[c]] = ttstr;
                    }

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
                            MultipleConclusionRule rule {name, *sequent, prem_conc_corr};
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
                        template_source = get_default_template(output_type, pnmatrix->distinguished_sets().size());
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
