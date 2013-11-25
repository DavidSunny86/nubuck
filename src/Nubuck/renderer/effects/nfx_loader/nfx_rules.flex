/* nfx_rules.flex */

%s IN_MULTILN_COMMENT

%option prefix="yynfx"
%option outfile="lex.yynfx.cpp"
%option nounistd
%option noyywrap
%option yylineno

%{
#include <stdlib.h>
#include <common\common.h>
#include "nfx_local.h"
%}

IDENT   [a-zA-Z][a-zA-Z0-9]*
INT     "-"?[0-9]+
FLOAT   [0-9]+"."[0-9]+
STRING  "\"".*"\""

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
"vs"        { return NFX_TOK_VS_SRC; }
"fs"        { return NFX_TOK_FS_SRC; }
"gs"        { return NFX_TOK_GS_SRC; }
"sortkey"   { return NFX_TOK_SORTKEY; }

{IDENT}     { return NFX_TOK_IDENT; }

"GL_TRUE"   { nfx_val_bool = GL_TRUE; return NFX_TOK_VAL_BOOL; }
"GL_FALSE"  { nfx_val_bool = GL_FALSE; return NFX_TOK_VAL_BOOL; }

{INT}       { nfx_val_int = atoi(yynfxtext); return NFX_TOK_VAL_INT; }
{FLOAT}     { nfx_val_float = atof(yynfxtext); return NFX_TOK_VAL_FLOAT; }

{STRING}    { return NFX_TOK_STRING; }

.           { return NFX_TOK_UNKNOWN; }

 /* BEGIN GL ENUMS */

"GL_ALWAYS"                 { nfx_val_enum = GL_ALWAYS;                 return NFX_TOK_VAL_ENUM; }
"GL_NEVER"                  { nfx_val_enum = GL_NEVER;                  return NFX_TOK_VAL_ENUM; }
"GL_EQUAL"                  { nfx_val_enum = GL_EQUAL;                  return NFX_TOK_VAL_ENUM; }
"GL_NOTEQUAL"               { nfx_val_enum = GL_NOTEQUAL;               return NFX_TOK_VAL_ENUM; }
"GL_LEQUAL"                 { nfx_val_enum = GL_LEQUAL;                 return NFX_TOK_VAL_ENUM; }
"GL_LESS"                   { nfx_val_enum = GL_LESS;                   return NFX_TOK_VAL_ENUM; }
"GL_GEQUAL"                 { nfx_val_enum = GL_GEQUAL;                 return NFX_TOK_VAL_ENUM; }
"GL_GREATER"                { nfx_val_enum = GL_GREATER;                return NFX_TOK_VAL_ENUM; }

"GL_ONE"                    { nfx_val_enum = GL_ONE;                    return NFX_TOK_VAL_ENUM; }
"GL_ZERO"                   { nfx_val_enum = GL_ZERO;                   return NFX_TOK_VAL_ENUM; }
"GL_SRC_ALPHA"              { nfx_val_enum = GL_SRC_ALPHA;              return NFX_TOK_VAL_ENUM; }
"GL_ONE_MINUS_SRC_ALPHA"    { nfx_val_enum = GL_ONE_MINUS_SRC_ALPHA;    return NFX_TOK_VAL_ENUM; }
"GL_DST_COLOR"              { nfx_val_enum = GL_DST_COLOR;              return NFX_TOK_VAL_ENUM; }

"GL_FRONT"                  { nfx_val_enum = GL_FRONT;                  return NFX_TOK_VAL_ENUM; }
"GL_BACK"                   { nfx_val_enum = GL_BACK;                   return NFX_TOK_VAL_ENUM; }

"GL_REPLACE"                { nfx_val_enum = GL_REPLACE;                return NFX_TOK_VAL_ENUM; }
"GL_INVERT"                 { nfx_val_enum = GL_INVERT;                 return NFX_TOK_VAL_ENUM; }

 /* END GL ENUMS */
%%

bool NFX_Parse(const char* filename, R::EffectDesc& desc) {
    yynfxin = fopen(filename, "r");
    if(!yynfxin) {
        common.printf("ERROR - unable to open file '%s'\n", filename);
        return false;
    }
    return NFX_StartParsing(filename, desc);
}
