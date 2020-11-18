#ifndef __CLI11_APPS__
#define __CLI11_APPS__

#include "external/CLI11/CLI11.hpp"
#include "cli/clidefs.h"
#include "cli/clihandlers.h"
#include "core/printers/factory.h"

namespace ltsy {

    /**
     * Generic extension of CLI::App.
     * */
    class CLI11App : public CLI::App {
        public:
            CLI11App (const std::string& name,
                    const std::string& desc) : CLI::App {desc, name} {}

            CLI11App (const std::string& name) : CLI::App {name} {}
    };

    /**
     * Truth-table determinizer app.
     * */
    class TTDeterminizerCLI11App : public CLI11App {
        public:

        private:
            std::string _file_path;
            Printer::PrinterType _output_type = Printer::PrinterType::PLAIN;

            std::map<std::string, Printer::PrinterType> output_type_mapping
                {{"plain", Printer::PrinterType::PLAIN}, {"latex", Printer::PrinterType::LATEX}};

        public:

            TTDeterminizerCLI11App() : CLI11App (CLIDefs::TT_DET_APP_NAME, CLIDefs::TT_DET_APP_DESC) {
                this->add_option("-f,--file", _file_path, "YAML input file")
                   ->required()
                   ->check(CLI::ExistingFile);
                this->add_option("-o, --output", _output_type, "Output type")
                    ->check(CLI::CheckedTransformer(output_type_mapping, CLI::ignore_case));
                this->callback([&]() {
                    TTDeterminizerCLIHandler handler;
                    handler.handle(_file_path, _output_type);
                });
            }
    };

    /**
     * Truth-table axiomatizer app.
     * */
    class TTAxiomatizerCLI11App : public CLI11App {
        private:
            std::string _file_path;

        public:
            TTAxiomatizerCLI11App() : CLI11App {CLIDefs::TT_AXI_APP_NAME, CLIDefs::TT_AXI_APP_DESC} {
                this->add_option("-f,--file", _file_path, "YAML input file")
                   ->required()
                   ->check(CLI::ExistingFile);
                this->callback([&]() {
                    TTAxiomatizerCLIHandler handler;
                    handler.handle(_file_path);
                });
            }
    };

    /**
     * Soundness check app.
     * */
    class SequentRuleSoundnessCLI11App : public CLI11App {
        private:
            std::string _file_path;

        public:
            SequentRuleSoundnessCLI11App() : CLI11App {CLIDefs::RSOUND_APP_NAME, CLIDefs::RSOUND_APP_DESC} {
                this->add_option("-f,--file", _file_path, "YAML input file")
                   ->required()
                   ->check(CLI::ExistingFile);
                this->callback([&]() {
                    SequentRuleSoundnessCLIHandler handler;
                    handler.handle(_file_path);
                });
            }
    };

    /**
     * Main CLI App.
     * */
    class MainCLI11App : public CLI11App {

        public:
            MainCLI11App() : CLI11App {CLIDefs::CLI_APP_NAME} {
                this->add_subcommand(std::make_shared<TTDeterminizerCLI11App>());
                this->add_subcommand(std::make_shared<TTAxiomatizerCLI11App>());
                this->add_subcommand(std::make_shared<SequentRuleSoundnessCLI11App>());
            }
    };

};

#endif
