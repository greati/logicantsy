%{
	#include <iostream>
	#include <cstdlib>
	#include "core/parser/fmla/flex_fmla_lexer.h"
	#include "core/parser/fmla/fmla_parser.h"
	#include "bison_fmla_parser.hpp"
	#include "location.hh"

	using namespace std;

	// Original yyterminate() macro returns int. Since we're using Bison 3 variants
	// as tokens, we must redefine it to change type from `int` to `BisonFmlaParserGen::semantic_type`
	#define yyterminate() ltsy::BisonFmlaParserGen::make_TOK_END(loc);

    // Track current scanner location
    #define YY_USER_ACTION loc.columns (yyleng);

    void lexer_error(const char*, ltsy::location&);
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
    // Code run each time yylex is called.
    loc.step();

%}

<<EOF>>     { return yyterminate(); }
{ignore}    { loc.step(); }
\n+         { loc.lines(yyleng); loc.step(); }
\\\/        { return ltsy::BisonFmlaParserGen::make_TOK_OR_CON(loc); }
\/\         { return ltsy::BisonFmlaParserGen::make_TOK_AND_CON(loc); }
->          { return ltsy::BisonFmlaParserGen::make_TOK_IMP_CON(loc); }
~           { return ltsy::BisonFmlaParserGen::make_TOK_NEG_CON(loc); }
_           { return ltsy::BisonFmlaParserGen::make_TOK_BOT_CON(loc); }
\^          { return ltsy::BisonFmlaParserGen::make_TOK_TOP_CON(loc); }
\(          { return ltsy::BisonFmlaParserGen::make_TOK_LPAR(loc); }
\)          { return ltsy::BisonFmlaParserGen::make_TOK_RPAR(loc); }
,           { return ltsy::BisonFmlaParserGen::make_TOK_COMMA(loc); }
`{id}`      { return ltsy::BisonFmlaParserGen::make_TOK_CUSTOM_CON(yytext, loc); }
{id}        { return ltsy::BisonFmlaParserGen::make_TOK_IDENTIFIER(yytext, loc); }
.           { 
                lexer_error(yytext, loc);
            }
%%

void lexer_error(const char* text, ltsy::location& loc) {
    std::cout << "(WARNING at [" << loc.begin.line << "," << loc.begin.column << 
        "]) unknown sequence " << text << endl;
}


