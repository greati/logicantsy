%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.0"
%defines
%define api.parser.class { BisonFmlaParserGen }

%define api.token.constructor
%define api.value.type variant
%define parse.assert
%define api.namespace { ltsy }
%code requires
{
    #include <iostream>
    #include <string>
    #include <vector>
    #include <stdint.h>
    #include "core/syntax.h"
//    #include "command.h"

    using namespace std;

    namespace ltsy {
        class FlexFmlaLexer;
        class BisonFmlaParser;
        //void BisonFmlaParser::set_parsed_formula(std::shared_ptr<ltsy::Formula>);
    }
}

// Bison calls yylex() function that must be provided by us to suck tokens
// from the scanner. This block will be placed at the beginning of IMPLEMENTATION file (cpp).
// We define this function here (function! not method).
// This function is called only inside Bison, so we make it static to limit symbol visibility for the linker
// to avoid potential linking conflicts.
%code top
{
    #include <iostream>
    #include "core/parser/fmla/flex_fmla_lexer.h"
    #include "bison_fmla_parser.hpp"
    #include "core/parser/fmla/fmla_parser.h"
//    #include "interpreter.h"
    #include "location.hh"
    
    // yylex() arguments are defined in parser.y
    static ltsy::BisonFmlaParserGen::symbol_type yylex(ltsy::FlexFmlaLexer &scanner, ltsy::BisonFmlaParser &parserwrapper) {
        return scanner.get_next_token();
    }
    
    // you can accomplish the same thing by inlining the code using preprocessor
    // x and y are same as in above static function
    // #define yylex(x, y) scanner.get_next_token()
    
    using namespace ltsy;

    auto and_con = std::make_shared<ltsy::Connective>("and", 2);
    auto or_con = std::make_shared<ltsy::Connective>("or", 2);
    auto imp_con = std::make_shared<ltsy::Connective>("->", 2);
    auto neg_con = std::make_shared<ltsy::Connective>("neg", 1);
    auto bot_con = std::make_shared<ltsy::Connective>("_", 0);
    auto top_con = std::make_shared<ltsy::Connective>("^", 0);
}

%lex-param { ltsy::FlexFmlaLexer &scanner }
%lex-param { ltsy::BisonFmlaParser &parserwrapper }
%parse-param { ltsy::FlexFmlaLexer &scanner }
%parse-param { ltsy::BisonFmlaParser &parserwrapper }
%locations
%define parse.trace
%define parse.error verbose

%define api.token.prefix {TOK_}

%token TOK_END 0 "end of file"
%token <std::string> TOK_IDENTIFIER  "id";
%token <std::string> TOK_CUSTOM_CON "custom_op";
%token TOK_LPAR "leftpar";
%token TOK_RPAR "rightpar";
%token TOK_COMMA "comma";
%token TOK_OR_CON "or";
%token TOK_AND_CON "and";
%token TOK_IMP_CON "imp";
%token TOK_NEG_CON "neg";
%token TOK_TOP_CON "top";
%token TOK_BOT_CON "bot";

//%type< ltsy::Command > command;
%type<std::shared_ptr<ltsy::Formula>> fmla;
%type<std::shared_ptr<ltsy::Formula>> term;
%type<std::vector<std::shared_ptr<ltsy::Formula>>> term_list;
%type<std::vector<std::shared_ptr<ltsy::Formula>>> term_list_comma;

%start fmla

%right TOK_OR_CON TOK_AND_CON
%right TOK_IMP_CON TOK_CUSTOM_CON
%left TOK_NEG_CON

%%

fmla : term { $$ = $1; parserwrapper.set_parsed_formula($$); }
term : TOK_LPAR term TOK_RPAR { $$ = $2; }
     | TOK_IDENTIFIER { $$ = std::make_shared<ltsy::Prop>($1); }
     | term TOK_CUSTOM_CON term { 
            auto custom_con = std::make_shared<ltsy::Connective>($2, 2);
            std::vector<std::shared_ptr<ltsy::Formula>> args {$1, $3};
            $$ = std::make_shared<ltsy::Compound>(custom_con, args); 
        }
     | term TOK_OR_CON term {
            std::vector<std::shared_ptr<ltsy::Formula>> args {$1, $3};
            $$ = std::make_shared<ltsy::Compound>(or_con, args); 
        }
     | term TOK_AND_CON term {
            std::vector<std::shared_ptr<ltsy::Formula>> args {$1, $3};
            $$ = std::make_shared<ltsy::Compound>(and_con, args); 
        }
     | term TOK_IMP_CON term {
            std::vector<std::shared_ptr<ltsy::Formula>> args {$1, $3};
            $$ = std::make_shared<ltsy::Compound>(imp_con, args); 
        }
     | TOK_NEG_CON term {
            std::vector<std::shared_ptr<ltsy::Formula>> args {$2};
            $$ = std::make_shared<ltsy::Compound>(neg_con, args); 
        }
     | TOK_BOT_CON {
            std::vector<std::shared_ptr<ltsy::Formula>> args;
            $$ = std::make_shared<ltsy::Compound>(bot_con, args); 
        } 
     | TOK_TOP_CON {
            std::vector<std::shared_ptr<ltsy::Formula>> args;
            $$ = std::make_shared<ltsy::Compound>(top_con, args); 
        } 
     | TOK_IDENTIFIER TOK_LPAR term_list TOK_RPAR {
            auto args = $3;
            auto num_params = args.size();
            auto prefixed_con = std::make_shared<ltsy::Connective>($1, num_params);
            $$ = std::make_shared<ltsy::Compound>(prefixed_con, args); 
        }

term_list : /* empty */ { $$ = std::vector<std::shared_ptr<ltsy::Formula>> {}; }
          | term_list_comma { $$ = $1; }

term_list_comma : term { $$ = std::vector<std::shared_ptr<ltsy::Formula>> {$1}; }
                | term TOK_COMMA term_list {
                    auto args = $3;
                    args.insert(args.begin(), $1);
                    $$ = args;
                }
%%

// Bison expects us to provide implementation - otherwise linker complains
void ltsy::BisonFmlaParserGen::error(const location &loc , const std::string &message) {
    std::cout << "(ERROR at [" << loc.begin.line << "," << loc.begin.column << 
        "]) " << message << endl;
}
