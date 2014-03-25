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
    NFX_TOK_VS_SRC,
    NFX_TOK_FS_SRC,
    NFX_TOK_GS_SRC,
    NFX_TOK_SORTKEY,

    NFX_TOK_VAL_BOOL,
    NFX_TOK_VAL_INT,
    NFX_TOK_VAL_FLOAT,
    NFX_TOK_VAL_ENUM,

    NFX_TOK_STRING,
    NFX_TOK_MLSTRING /* multiline string */
};

extern char*    yynfxtext;
extern int      yynfxlineno;
int             yynfxlex(void);

extern GLboolean    nfx_val_bool;
extern GLint        nfx_val_int;
extern GLfloat      nfx_val_float;
extern GLenum       nfx_val_enum;

bool NFX_StartParsing(const char* filename, R::EffectDesc& desc);
