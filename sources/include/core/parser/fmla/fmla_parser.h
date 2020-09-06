#ifndef __FMLA_PARSER__
#define __FMLA_PARSER__

#include "flex_fmla_lexer.h"
#include "bison_fmla_parser.hpp"
#include "core/syntax.h"

namespace ltsy {

    class FmlaParser {
        
        public:

            /**
             * Parse a string into a formula.
             * */
            virtual std::shared_ptr<Formula> parse(const std::string&) = 0;

    };

    class BisonFmlaParser : public FmlaParser {

        private:
            
            FlexFmlaLexer lexer;
            BisonFmlaParserGen parser;
            std::shared_ptr<ltsy::Formula> parsed_fmla;

        public:

            BisonFmlaParser() : parser(lexer, *this) {}

            void reset_stream(std::istream *is) {
                lexer.switch_streams(is, NULL);
            }

            void set_parsed_formula(decltype(parsed_fmla) parsed_fmla) {
                this->parsed_fmla = parsed_fmla;
            }

            std::shared_ptr<Formula> parse(const std::string& str) override {
                std::istringstream is {str};
                reset_stream(&is);
                parser.parse(); 
                return this->parsed_fmla;
            } 
    };

};

#endif
