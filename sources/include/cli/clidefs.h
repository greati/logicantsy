#ifndef __CLI_DEFS__
#define __CLI_DEFS__

namespace ltsy {

    /**
     * Definitions for CLI.
     * */
    class CLIDefs {
        public:
            inline static const std::string CLI_APP_NAME = "logicantsy - Command line interface";
            inline static const std::string TT_DET_APP_NAME = "determinize-tt";
            inline static const std::string TT_DET_APP_DESC = "Start from a (partial) fully non-deterministic k-ary n-valued truth-table and\nimpose axiomatic sequents to reduce non-determinism.";
            inline static const std::string TT_AXI_APP_NAME = "axiomatize-tt";
            inline static const std::string TT_AXI_APP_DESC = "Produce axiomatic sequents given a (partial) fully non-deterministic\nk-ary n-valued truth-table.";
    };

};
#endif
