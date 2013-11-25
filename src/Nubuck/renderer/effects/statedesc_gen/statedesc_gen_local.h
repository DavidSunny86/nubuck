#pragma once

#include <renderer\effects\statedesc.h>

enum {
    STG_TOK_UNKNOWN = 0,
    STG_TOK_EOF,
    STG_TOK_SEMICOL,
    STG_TOK_LBRACE,
    STG_TOK_RBRACE,
    STG_TOK_STRUCT,
    STG_TOK_VARTYPE,
    STG_TOK_IDENT
};

extern char*    yystgtext;
extern int      yystglineno;
int             yystglex(void);

extern StateFieldType stg_fieldType;

bool STG_StartParsing(const char* inname, const char* outname);
