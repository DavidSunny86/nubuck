// nfx_tokens.h

#include <renderer\effects\effect.h>

enum {
    NFX_TOK_UNKNOWN = 0,
	NFX_TOK_SEMICOL,
	NFX_TOK_LBRACE,
	NFX_TOK_RBRACE,
	NFX_TOK_QUOTE,
	NFX_TOK_DOT,
	NFX_TOK_EQUALS,

	NFX_TOK_IDENT,
	NFX_TOK_FX,
	NFX_TOK_PASS,

    NFX_TOK_VAL_BOOL,
    NFX_TOK_VAL_ENUM
};

extern char*    yytext;
extern int      yylineno;
int             yylex(void);

extern GLboolean    nfx_val_bool;
extern GLenum       nfx_val_enum;

bool NFX_StartParsing(const char* filename, R::EffectDesc& desc);
