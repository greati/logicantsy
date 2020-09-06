%{
	#include <iostream>
	#include <cstdlib>
	#include "core/parser/fmla/flex_fmla_lexer.h"
	//?#include "interpreter.h"
	#include "bison_fmla_parser.hpp"
	#include "location.hh"

	using namespace std;

	// Original yyterminate() macro returns int. Since we're using Bison 3 variants
	// as tokens, we must redefine it to change type from `int` to `BisonFmlaParserGen::semantic_type`
	#define yyterminate() ltsy::BisonFmlaParserGen::make_TOK_END(ltsy::location());

	// This will track current scanner location.
	// Action is called when length of the token is known.
	//#define YY_USER_ACTION m_driver.increaseLocation(yyleng);
	
	// !!!WARNING!!!
	// Location API is used, but the location is not initialized, 'cause I'm lazy. When making
	// a token with make_{something} method you can pass detailed token location. Current location
	// is accessible with m_driver.location() method. All puzzle elements are there - just
	// pass location value in every action code block below. I'm going to waste more time writing
	// this excuse than putting this boilerplate below...
	//
	// Location class can be found in location.hh and posistion.hh files. It's just a bit too much
	// boilerplate for this small example. Bummer.
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
<<EOF>>     { return yyterminate(); }
{ignore}     { ; }
\\\/        { return ltsy::BisonFmlaParserGen::make_TOK_OR_CON(ltsy::location()); }
\/\         { return ltsy::BisonFmlaParserGen::make_TOK_AND_CON(ltsy::location()); }
->          { return ltsy::BisonFmlaParserGen::make_TOK_IMP_CON(ltsy::location()); }
~           { return ltsy::BisonFmlaParserGen::make_TOK_NEG_CON(ltsy::location()); }
_           { return ltsy::BisonFmlaParserGen::make_TOK_BOT_CON(ltsy::location()); }
\^          { return ltsy::BisonFmlaParserGen::make_TOK_TOP_CON(ltsy::location()); }
\(          { return ltsy::BisonFmlaParserGen::make_TOK_LPAR(ltsy::location()); }
\)          { return ltsy::BisonFmlaParserGen::make_TOK_RPAR(ltsy::location()); }
,           { return ltsy::BisonFmlaParserGen::make_TOK_COMMA(ltsy::location()); }
`{id}`      { return ltsy::BisonFmlaParserGen::make_TOK_CUSTOM_CON(yytext, ltsy::location()); }
{id}        { return ltsy::BisonFmlaParserGen::make_TOK_IDENTIFIER(yytext, ltsy::location()); }
.           { 
                cout << "Scanner: unknown character [" << yytext << "]" << endl; 
            }
%%
