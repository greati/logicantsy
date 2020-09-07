#ifndef __CLI11_APPS__
#define __CLI11_APPS__

#include "external/CLI11/CLI11.hpp"
#include "cli/clidefs.h"

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
        private:
            std::string _file_path;

        public:
            TTDeterminizerCLI11App() : CLI11App (CLIDefs::TT_DET_APP_NAME, CLIDefs::TT_DET_APP_DESC) {
                this->add_option("-f,--file", _file_path, "YAML input file")
                   ->required()
                   ->check(CLI::ExistingFile);
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
            }
    };

};

#endif
