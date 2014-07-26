/* statedesc_gen_rules.flex */

%option prefix="yystg"
%option outfile="lex.yystg.cpp"
%option nounistd
%option yylineno

%{
#include "statedesc_gen_local.h"

// replaces %option noyywrap, avoids compiler warning
// "not enough actual paramters for macro"
extern "C" int yywrap() { return 1; }
%}

IDENT [a-zA-Z][a-zA-Z0-9]*

%%

<<EOF>> { return STG_TOK_EOF; }

[\t\n ]+ /* ignore whitespace */

";"         { return STG_TOK_SEMICOL; }
"{"         { return STG_TOK_LBRACE; }
"}"         { return STG_TOK_RBRACE; }

"struct"    { return STG_TOK_STRUCT; }

"GLboolean" { stg_fieldType = SFT_BOOL; return STG_TOK_VARTYPE; }
"GLuint"    { stg_fieldType = SFT_UINT; return STG_TOK_VARTYPE; }
"GLint"     { stg_fieldType = SFT_INT; return STG_TOK_VARTYPE; }
"float"     { stg_fieldType = SFT_FLOAT; return STG_TOK_VARTYPE; }
"GLenum"    { stg_fieldType = SFT_ENUM; return STG_TOK_VARTYPE; }

{IDENT}     { return STG_TOK_IDENT; }

.           { return STG_TOK_UNKNOWN; }

%%

bool STG_Parse(const char* inname, const char* outname) {
    yystgin = fopen(inname, "r");
    return STG_StartParsing(inname, outname);
}
