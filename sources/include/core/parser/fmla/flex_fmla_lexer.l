%{
	#include <iostream>
	#include <cstdlib>
	#include "core/parser/fmla/flex_fmla_lexer.h"
	#include "core/parser/fmla/fmla_parser.h"
	#include "bison_fmla_parser.hpp"
	#include "location.hh"
    #include "spdlog/spdlog.h"

	using namespace std;

	// Original yyterminate() macro returns int. Since we're using Bison 3 variants
	// as tokens, we must redefine it to change type from `int` to `BisonFmlaParserGen::semantic_type`
	#define yyterminate() ltsy::BisonFmlaParserGen::make_TOK_END(loc);

    // Track current scanner location
    #define YY_USER_ACTION loc.columns (yyleng);

    void lexer_error(const char*, ltsy::location&, std::optional<ltsy::FmlaParser::Location> global_location);
%}

%option nodefault
%option noyywrap
%option c++
%option yyclass="FlexFmlaLexer"
%option prefix="ltsy_"

ws          [\t ]+
ignore      ({ws}|\n)
digit       [0-9]
integer     {digit}+
alpha       [a-zA-Z]
symbol      [\#\$\!\@\%\&\*\<\>\?\~\-\+\/]
id          ({alpha}|{symbol})({alpha}|{digit}|{symbol})*

%%

%{
    // A handy shortcut to the location held by the driver.
    ltsy::location& loc = parserwrapper.location;
    // Global location (from an outside parser, like YAML parser)
    auto global_location = parserwrapper.global_location();
    // Code run each time yylex is called.
    loc.step();

%}

<<EOF>>     { return yyterminate(); }
{ignore}    { loc.step(); }
\n+         { loc.lines(yyleng); loc.step(); }
or          { return ltsy::BisonFmlaParserGen::make_TOK_OR_CON(loc); }
and         { return ltsy::BisonFmlaParserGen::make_TOK_AND_CON(loc); }
->          { return ltsy::BisonFmlaParserGen::make_TOK_IMP_CON(loc); }
neg         { return ltsy::BisonFmlaParserGen::make_TOK_NEG_CON(loc); }
_           { return ltsy::BisonFmlaParserGen::make_TOK_BOT_CON(loc); }
\^          { return ltsy::BisonFmlaParserGen::make_TOK_TOP_CON(loc); }
\(          { return ltsy::BisonFmlaParserGen::make_TOK_LPAR(loc); }
\)          { return ltsy::BisonFmlaParserGen::make_TOK_RPAR(loc); }
,           { return ltsy::BisonFmlaParserGen::make_TOK_COMMA(loc); }
`{id}`      { return ltsy::BisonFmlaParserGen::make_TOK_CUSTOM_CON(yytext, loc); }
{id}        { return ltsy::BisonFmlaParserGen::make_TOK_IDENTIFIER(yytext, loc); }
.           { 
                lexer_error(yytext, loc, global_location);
            }
%%

void lexer_error(const char* text, ltsy::location& loc, std::optional<ltsy::FmlaParser::Location> global_location) {
    if (global_location != std::nullopt) {
        std::string error_msg = "near (" + std::to_string(global_location.value().row) + "," + std::to_string(global_location.value().col) + ")" + 
            ", at (" + std::to_string(loc.begin.line) + "," + std::to_string(loc.begin.column) + "), unknown sequence " + text;
        spdlog::warn(error_msg);
    } else {
        spdlog::warn("at (" + std::to_string(loc.begin.line) + "," + std::to_string(loc.begin.column) + "), unknown sequence " + text);
    }
}


