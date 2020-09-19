#ifndef __FMLA_PARSER__
#define __FMLA_PARSER__

#include "flex_fmla_lexer.h"
#include "bison_fmla_parser.hpp"
#include "core/syntax.h"
#include "core/exception/exceptions.h"
#include <optional>

namespace ltsy {

    class FmlaParser {

        public:

            struct Location {
                int row, col;
                Location() : row {-1}, col {-1} { /* empty */ } 
                Location(int _row, int _col) : row {_row}, col {_col} { /* empty */ } 
            };

        private:
            std::optional<Location> _global_location;

        public:


            FmlaParser() { /* empty */ }

            FmlaParser(const Location& global_location) : _global_location {std::optional<Location>{global_location}} { /* empty */ }

            /**
             * Parse a string into a formula.
             * */
            virtual std::shared_ptr<Formula> parse(const std::string&) = 0;
            
            inline decltype(_global_location) global_location() const { return _global_location; };
    };

    class BisonFmlaParser : public FmlaParser {

        private:
            
            FlexFmlaLexer lexer;
            BisonFmlaParserGen parser;
            std::shared_ptr<ltsy::Formula> parsed_fmla;

        public:

            ltsy::location location;

            BisonFmlaParser() : lexer (*this), parser(lexer, *this) {}
            BisonFmlaParser(const Location& global_location) : FmlaParser {global_location}, lexer(*this), parser(lexer, *this) { /* empty */ }

            void reset_stream(std::istream *is) {
                lexer.switch_streams(is, NULL);
            }

            void set_parsed_formula(decltype(parsed_fmla) parsed_fmla) {
                this->parsed_fmla = parsed_fmla;
            }

            void clean() {
                //this->parsed_fmla = nullptr;
                location = ltsy::location();

            }

            std::shared_ptr<Formula> parse(const std::string& str) override {
                clean();
                std::istringstream is {str};
                reset_stream(&is);
                parser.parse(); 
                if (this->parsed_fmla == nullptr)
                    throw ParseException("a parse error has occurred, check your output");
                return this->parsed_fmla;
            } 
    };

};

#endif
