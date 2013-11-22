/* nfx_rules.flex */

%s IN_MULTILN_COMMENT

%option nounistd
%option noyywrap
%option yylineno

%{
#include "nfx_local.h"
%}

IDENT   [a-zA-Z][a-zA-Z0-9]*

%%

 /* ignore c-style comments, cnf. flex manual on conditions */
"/*"                                { BEGIN(IN_MULTILN_COMMENT); }
<IN_MULTILN_COMMENT>[^*\n]*         /* ignore anything except '*' */
<IN_MULTILN_COMMENT>"*"+[^*/\n]*    /* ignore any '*'s not followed by '/' */
<IN_MULTILN_COMMENT>"*"+"/"         { BEGIN(INITIAL); }

"//".*"\n" /* ignore c++-style comments */


[\t\n ]+ /* ignore whitespace */

";"         { return NFX_TOK_SEMICOL; }
"{"         { return NFX_TOK_LBRACE; }
"}"         { return NFX_TOK_RBRACE; }
\"          { return NFX_TOK_QUOTE; }
"."         { return NFX_TOK_DOT; }
"="         { return NFX_TOK_EQUALS; }

"fx"        { return NFX_TOK_FX; }
"pass"      { return NFX_TOK_PASS; }

{IDENT}     { return NFX_TOK_IDENT; }

"GL_TRUE"   { nfx_val_bool = GL_TRUE; return NFX_TOK_VAL_BOOL; }
"GL_FALSE"  { nfx_val_bool = GL_FALSE; return NFX_TOK_VAL_BOOL; }

.           { return NFX_TOK_UNKNOWN; }

%%

bool NFX_Parse(const char* filename, R::EffectDesc& desc) {
    yyin = fopen(filename, "r");
    return NFX_StartParsing(filename, desc);
}
