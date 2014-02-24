#include <assert.h>
#include <vector>
#include <Nubuck\common\common.h>
#include "shader_preprocessor_local.h"

static int g_nextToken = R::SPP::Tokens::TOK_UNKNOWN;

#define TOKTOSTRING_CASE(tok) case tok: return #tok;
static const char* TokToString(int tok) {
    switch(tok) {
    TOKTOSTRING_CASE(R::SPP::Tokens::TOK_UNKNOWN)
    TOKTOSTRING_CASE(R::SPP::Tokens::TOK_DEFAULT)
    TOKTOSTRING_CASE(R::SPP::Tokens::TOK_SEMICOL)
    TOKTOSTRING_CASE(R::SPP::Tokens::TOK_LPAREN)
    TOKTOSTRING_CASE(R::SPP::Tokens::TOK_RPAREN)
    TOKTOSTRING_CASE(R::SPP::Tokens::TOK_VAR_TYPE)
    TOKTOSTRING_CASE(R::SPP::Tokens::TOK_INCLUDE)
    TOKTOSTRING_CASE(R::SPP::Tokens::TOK_ATTRIB)
    TOKTOSTRING_CASE(R::SPP::Tokens::TOK_STRING)
    TOKTOSTRING_CASE(R::SPP::Tokens::TOK_IDENT)
    TOKTOSTRING_CASE(R::SPP::Tokens::TOK_INTEGER)
    TOKTOSTRING_CASE(R::SPP::Tokens::TOK_EOL)
    TOKTOSTRING_CASE(R::SPP::Tokens::TOK_EOF)
    }
    assert(0 && "TokToString: unknown token");
    return NULL;
}

static void NextToken(void) {
    g_nextToken = yyspplex();
}

static bool ExpectToken(int tok) {
    if(tok != g_nextToken) {
        const char* tokName = TokToString(tok);
        common.printf("%d: expected '%s', got '%s'\n", yyspplineno, tokName, yyspptext);
        return false;
    }
    return true;
}

static bool ExpectTokens(int tok0, int tok1) {
    if(tok0 != g_nextToken && tok1 != g_nextToken) {
        const char* tokName0 = TokToString(tok0);
        const char* tokName1 = TokToString(tok1);
        common.printf("%d: expected '%s' or '%s', got '%s'\n", yyspplineno, tokName0, tokName1, yyspptext);
        return false;
    }
    return true;
}

static bool ParseInclude(void) {
    NextToken();
    if(!ExpectToken(R::SPP::Tokens::TOK_STRING)) return false;
    std::string filename(yyspptext + 1, strlen(yyspptext) - 2);
    std::string path = common.BaseDir() + filename;
    R::SPP::YYSPP_PushFile(path.c_str());
    return true;
}

static bool ParseAttributeDecl(std::string& out, R::AttributeLocation& loc) {
    NextToken();
    if(!ExpectToken(R::SPP::Tokens::TOK_LPAREN)) return false;
    NextToken();
    if(!ExpectToken(R::SPP::Tokens::TOK_INTEGER)) return false;
    loc.loc = atoi(yyspptext);
    NextToken();
    if(!ExpectToken(R::SPP::Tokens::TOK_RPAREN)) return false;
    NextToken();
    if(!ExpectToken(R::SPP::Tokens::TOK_VAR_TYPE)) return false;
    std::string type = yyspptext;
    NextToken();
    if(!ExpectToken(R::SPP::Tokens::TOK_IDENT)) return false;
    loc.name = yyspptext;
    NextToken();
    if(!ExpectToken(R::SPP::Tokens::TOK_SEMICOL)) return false;
    out += "attribute " + type + " " + loc.name + ";";
    return true;
}

namespace R {
namespace SPP {

bool SPP_StartParsing(std::string& out, std::vector<AttributeLocation>& attribLocs) {
    attribLocs.clear();
    bool done = false;
    while(!done) {
        NextToken();

        if(Tokens::TOK_INCLUDE == g_nextToken) {
            if(!ParseInclude()) return false;
        } else if(Tokens::TOK_ATTRIB == g_nextToken) {
            AttributeLocation loc;
            if(!ParseAttributeDecl(out, loc)) return false;
            attribLocs.push_back(loc);
        } else if(Tokens::TOK_EOF == g_nextToken && !YYSPP_PopFile()) {
            break;
        } else {
            out += yyspptext;
        }
    }
    return true;
}

} // namespace SPP
} // namespace R