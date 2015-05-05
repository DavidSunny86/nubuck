#pragma once

#include <string>
#include <renderer\shader\shader.h>

extern char*    yyspptext;
extern int      yyspplineno;
int             yyspplex(void);
void            yyspppop_buffer_state(void);

namespace R {
namespace SPP {

struct Tokens {
    enum {
        TOK_UNKNOWN = 0,
        TOK_DEFAULT,
        TOK_SEMICOL,
        TOK_LPAREN,
        TOK_RPAREN,
        TOK_VAR_TYPE,
        TOK_INCLUDE,
        TOK_ATTRIB,
        TOK_MATERIAL_UFORM,
        TOK_STRING,
        TOK_IDENT,
        TOK_INTEGER,
        TOK_EOL,
        TOK_EOF
    };
};

bool YYSPP_PushFile(const char* filename);
bool YYSPP_PopFile();

bool SPP_StartParsing(
    std::string& out,
    std::vector<AttributeLocation>& attribLocs,
    std::vector<std::string>& materialUniforms);

} // namespace SPP
} // namespace R