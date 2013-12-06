/* shader_preprocessor_rules.flex */

%s IN_DIRECTIVE IN_VAR_DECL

%option prefix="yyspp"
%option outfile="lex.yyspp.cpp"
%option nounistd
%option noyywrap
%option yylineno

%{
#include <common\common.h>
#include <common\filehandle.h>
#include "shader_preprocessor.h"
#include "shader_preprocessor_local.h"
%}

WSPACE      [\t\n ]+
IDENT       [a-zA-Z][a-zA-Z0-9]*
INTEGER     -?([1-9][0-9]*|0)
INCL_PATH   (<|\").*(>|\")

%%

 /* 
 ================================================================================
 INCLUDE FILES
 ================================================================================
 */
"#include"                  { BEGIN(IN_DIRECTIVE); return R::SPP::Tokens::TOK_INCLUDE; }
<IN_DIRECTIVE>[\t ]+        /* ignore spaces, tabs */
<IN_DIRECTIVE>{INCL_PATH}   { BEGIN(INITIAL); return R::SPP::Tokens::TOK_STRING; }
<IN_DIRECTIVE>"\n"          { BEGIN(INITIAL); return R::SPP::Tokens::TOK_EOL; }
<IN_DIRECTIVE>.             { return R::SPP::Tokens::TOK_UNKNOWN; }

"attribute"             { BEGIN(IN_VAR_DECL); return R::SPP::Tokens::TOK_ATTRIB; }
<IN_VAR_DECL>{WSPACE}   /* ignore whitespace */
<IN_VAR_DECL>"("        { return R::SPP::Tokens::TOK_LPAREN; }
<IN_VAR_DECL>")"        { return R::SPP::Tokens::TOK_RPAREN; }
<IN_VAR_DECL>{INTEGER}  { return R::SPP::Tokens::TOK_INTEGER; }

<IN_VAR_DECL>"bool"     { return R::SPP::Tokens::TOK_VAR_TYPE; }
<IN_VAR_DECL>"int"      { return R::SPP::Tokens::TOK_VAR_TYPE; }
<IN_VAR_DECL>"uint"     { return R::SPP::Tokens::TOK_VAR_TYPE; }
<IN_VAR_DECL>"float"    { return R::SPP::Tokens::TOK_VAR_TYPE; }
<IN_VAR_DECL>"double"   { return R::SPP::Tokens::TOK_VAR_TYPE; }
<IN_VAR_DECL>"vec2"     { return R::SPP::Tokens::TOK_VAR_TYPE; }
<IN_VAR_DECL>"vec3"     { return R::SPP::Tokens::TOK_VAR_TYPE; }
<IN_VAR_DECL>"vec4"     { return R::SPP::Tokens::TOK_VAR_TYPE; }
<IN_VAR_DECL>"mat2"     { return R::SPP::Tokens::TOK_VAR_TYPE; }
<IN_VAR_DECL>"mat3"     { return R::SPP::Tokens::TOK_VAR_TYPE; }
<IN_VAR_DECL>"mat4"     { return R::SPP::Tokens::TOK_VAR_TYPE; }

<IN_VAR_DECL>{IDENT}    { return R::SPP::Tokens::TOK_IDENT; }
<IN_VAR_DECL>";"        { BEGIN(INITIAL); return R::SPP::Tokens::TOK_SEMICOL; }
<IN_VAR_DECL>.          { return R::SPP::Tokens::TOK_UNKNOWN; }

<<EOF>>                 { return R::SPP::Tokens::TOK_EOF; }
"\n"                    { return R::SPP::Tokens::TOK_DEFAULT; }
.                       { return R::SPP::Tokens::TOK_DEFAULT; }

%%

namespace R {
namespace SPP {

bool YYSPP_PushFile(const char* filename) {
    COM::FileHandle file(fopen(filename, "r"));
    if(!file.Handle()) {
        common.printf("YYSPP_PushFile: unable to open file %s\n", filename);
        return false;
    }
    printf("include: %s\n", filename);
    yysppin = file.Release();
    yyspppush_buffer_state(yyspp_create_buffer(yysppin, YY_BUF_SIZE));
    return true;
}

bool YYSPP_PopFile() {
    yyspppop_buffer_state();
    return YY_CURRENT_BUFFER;
}

bool PreprocessShaderSource(
        const std::string& in, 
        std::string& out, 
        std::vector<AttributeLocation>& attribLocs)
{
    yyspp_scan_string(in.c_str());
    return SPP_StartParsing(out, attribLocs);
}

} // namespace SPP
} // namespace R
