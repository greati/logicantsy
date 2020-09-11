#ifndef __CLI_HANDLERS__
#define __CLI_HANDLERS__

#include "yaml/yamlcpp_parser.h"
#include "spdlog/spdlog.h"
#include "tt_determination/ndsequents.h"

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
            std::vector<std::shared_ptr<Connective>> connectives;
            std::vector<std::shared_ptr<Prop>> props;

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
                    // parse sequents, for each connective
                    auto sequents_node = parser.hard_require(root, SEQUENTS_TITLE);
                    std::cout << "Successful parsing." << std::endl;    
                } catch (ParseException& pe) {
                    spdlog::critical(pe.message());
                }

            };
    };

};

#endif
