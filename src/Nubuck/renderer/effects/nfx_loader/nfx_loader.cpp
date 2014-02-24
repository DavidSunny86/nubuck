#include <Nubuck\common\common.h>
#include <renderer\effects\statedesc.h>
#include "nfx_local.h"

static const char*  g_filename;
static int          g_nextToken;

GLboolean    nfx_val_bool;
GLint        nfx_val_int;
GLfloat      nfx_val_float;
GLenum       nfx_val_enum;

#define TOKTOSTRING_CASE(tok) case tok: return #tok
static const char* TokToString(int tok) {
    switch(tok) {
    TOKTOSTRING_CASE(NFX_TOK_UNKNOWN);
	TOKTOSTRING_CASE(NFX_TOK_SEMICOL);
	TOKTOSTRING_CASE(NFX_TOK_LBRACE);
	TOKTOSTRING_CASE(NFX_TOK_RBRACE);
	TOKTOSTRING_CASE(NFX_TOK_QUOTE);
    TOKTOSTRING_CASE(NFX_TOK_DOT);
    TOKTOSTRING_CASE(NFX_TOK_EQUALS);
	TOKTOSTRING_CASE(NFX_TOK_IDENT);
	TOKTOSTRING_CASE(NFX_TOK_FX);
	TOKTOSTRING_CASE(NFX_TOK_PASS);
	TOKTOSTRING_CASE(NFX_TOK_VS_SRC);
	TOKTOSTRING_CASE(NFX_TOK_FS_SRC);
	TOKTOSTRING_CASE(NFX_TOK_GS_SRC);
	TOKTOSTRING_CASE(NFX_TOK_SORTKEY);
    TOKTOSTRING_CASE(NFX_TOK_VAL_BOOL);
    TOKTOSTRING_CASE(NFX_TOK_VAL_INT);
    TOKTOSTRING_CASE(NFX_TOK_VAL_FLOAT);
    TOKTOSTRING_CASE(NFX_TOK_VAL_ENUM);
    TOKTOSTRING_CASE(NFX_TOK_STRING);
    default:
        assert(0 && "TokToString: unknown token");
    };
}

static void NextToken(void) {
    g_nextToken = yynfxlex();
}

static bool ExpectToken(int tok) {
    if(tok != g_nextToken) {
        const char* tokName = TokToString(tok);
        common.printf("%s:%d: expected '%s', got '%s'\n", g_filename, yynfxlineno, tokName, yynfxtext);
        return false;
    }
    return true;
}

static int Member(int fidx, const char* memName) {
    const StateFieldDesc& f = g_stateDesc[fidx];
    for(unsigned i = f.lchild; i <= f.rchild; ++i) {
        if(!strcmp(memName, g_stateDesc[i].name)) return i;
    }
    return -1;
}

static int FieldIndex(int fidx, const char* name) {
    if(0 <= fidx) return Member(fidx, name);
    if(!strcmp(g_stateDesc[0].name, name)) return 0;
    return -1;
}

static bool ParseValueAssignment(R::PassDesc& desc, int fidx) {
    NextToken();
    unsigned off = g_stateDesc[fidx].offset;
    unsigned uint = 0;
    switch(g_stateDesc[fidx].type) {
    case SFT_BOOL:
        if(!ExpectToken(NFX_TOK_VAL_BOOL)) return false;
        memcpy((char*)&desc.state + off, &nfx_val_bool, sizeof(GLboolean));
        break;
    case SFT_UINT:
        if(!ExpectToken(NFX_TOK_VAL_INT)) return false;
        if(0 > nfx_val_int) common.printf("%s:%d: WARNIGN, assigning int to unsigned int\n", g_filename, yynfxlineno);
        uint = (unsigned)nfx_val_int;
        memcpy((char*)&desc.state + off, &uint, sizeof(GLuint));
        break;
    case SFT_INT:
        if(!ExpectToken(NFX_TOK_VAL_INT)) return false;
        memcpy((char*)&desc.state + off, &nfx_val_int, sizeof(GLint));
        break;
    case SFT_FLOAT:
        if(!ExpectToken(NFX_TOK_VAL_FLOAT)) return false;
        memcpy((char*)&desc.state + off, &nfx_val_float, sizeof(GLfloat));
        break;
    case SFT_ENUM:
        if(!ExpectToken(NFX_TOK_VAL_ENUM)) return false;
        memcpy((char*)&desc.state + off, &nfx_val_enum, sizeof(GLenum));
        break;
    case SFT_STRUCT:
        common.printf("%s:%d: assigning value to struct\n", g_filename, yynfxlineno);
        return false;
    default:
        assert(0);
    }
    NextToken();
    if(!ExpectToken(NFX_TOK_SEMICOL)) return false;
    return true;
}

bool ParseField(R::PassDesc& desc, int fidx);

static bool ParseBlock(R::PassDesc& desc, int fidx) {
    bool done = false;
    while(!done) {
        NextToken();
        if(NFX_TOK_IDENT == g_nextToken) {
            if(!ParseField(desc, fidx)) return false;
        } else if(NFX_TOK_RBRACE == g_nextToken) {
            done = true;
        } else {
            common.printf("%s:%d: expected '%s' or '%s', got '%s'\n",
                g_filename, yynfxlineno, 
                TokToString(NFX_TOK_IDENT), TokToString(NFX_TOK_RBRACE), 
                yynfxtext);
            return false;
        }
    }
    return true;
}

static bool ParseField(R::PassDesc& desc, int fidx) {
    fidx = FieldIndex(fidx, yynfxtext);
    if(0 > fidx) {
        common.printf("%s:%d: unknown state '%s'\n", g_filename, yynfxlineno, yynfxtext);
        return false;
    }
    bool done = false;
    while(!done) {
        NextToken();
        if(NFX_TOK_DOT == g_nextToken) { // member field
            NextToken();
            if(!ExpectToken(NFX_TOK_IDENT)) return false;
            fidx = FieldIndex(fidx, yynfxtext);
            if(0 > fidx) {
                common.printf("%s:%d: unknown state '%s'\n", g_filename, yynfxlineno, yynfxtext);
                return false;
            }
        } else if(NFX_TOK_LBRACE == g_nextToken) { // field block
            if(!ParseBlock(desc, fidx)) return false;
            done = true;
        } else if(NFX_TOK_EQUALS == g_nextToken) {
            if(!ParseValueAssignment(desc, fidx)) return false;
            done = true;
        } else {
            common.printf("%s:%d: expected '%s' or '%s', got '%s'\n",
                g_filename, yynfxlineno, 
                TokToString(NFX_TOK_DOT), TokToString(NFX_TOK_EQUALS), 
                yynfxtext);
            return false;
        }
    } // while(!done) (parsing field)
    return true;
}

static bool ParseShaderSource(R::PassDesc& desc, R::Shader::Type type) {
    NextToken();
    if(!ExpectToken(NFX_TOK_EQUALS)) return false;
    NextToken();
    if(NFX_TOK_STRING == g_nextToken) {
        unsigned tokLen = strlen(yynfxtext);
        assert('"' == yynfxtext[0] && '"' == yynfxtext[tokLen - 1]);
        std::string source(yynfxtext + 1, tokLen - 2);
        desc.filenames[type] = source;
        NextToken();
        if(!ExpectToken(NFX_TOK_SEMICOL)) return false;
    } else {
        common.printf("%s:%d: expected '%s', got '%s'\n",
            g_filename, yynfxlineno,
            TokToString(NFX_TOK_STRING),
            yynfxtext);
        return false;
    }
    return true;
}

static bool ParsePass(R::PassDesc& desc) {
    NextToken();
    if(!ExpectToken(NFX_TOK_IDENT)) return false;
    desc.name = yynfxtext;
    
    NextToken();
    if(!ExpectToken(NFX_TOK_LBRACE)) return false;

    bool done = false;
    while(!done) {
        NextToken();
        if(NFX_TOK_IDENT == g_nextToken) { // field[...] = val;
            if(!ParseField(desc, -1)) return false;
        } else if(NFX_TOK_VS_SRC == g_nextToken) {
            if(!ParseShaderSource(desc, R::Shader::VERTEX)) return false;
        } else if(NFX_TOK_FS_SRC == g_nextToken) {
            if(!ParseShaderSource(desc, R::Shader::FRAGMENT)) return false;
        } else if(NFX_TOK_GS_SRC == g_nextToken) {
            if(!ParseShaderSource(desc, R::Shader::GEOMETRY)) return false;
        } else if(NFX_TOK_RBRACE == g_nextToken) {
            done = true;
        } else {
            common.printf("%s:%d: expected '%s' or '%s', got '%s'\n",
                g_filename, yynfxlineno, 
                TokToString(NFX_TOK_IDENT), TokToString(NFX_TOK_RBRACE), 
                yynfxtext);
            return false;
        }
    } // while(!done) (parsing pass' body)

    return true;
}

static void ClearPassDesc(R::PassDesc& desc) {
    SetDefaultState(desc.state);
    desc.type = R::DEFAULT;
    desc.flags = 0;
    desc.filenames[R::Shader::VERTEX] = "";
    desc.filenames[R::Shader::FRAGMENT] = "";
    desc.filenames[R::Shader::GEOMETRY] = "";
}

bool NFX_StartParsing(const char* filename, R::EffectDesc& desc) {
    g_filename  = filename;
    g_nextToken = 0;

    desc.sortKey    = 0;
    desc.name       = "";
    desc.passes.clear();

    NextToken();
    if(!ExpectToken(NFX_TOK_FX)) return false;

    NextToken();
    if(!ExpectToken(NFX_TOK_IDENT)) return false;
    desc.name = yynfxtext;

    NextToken();
    if(!ExpectToken(NFX_TOK_LBRACE)) return false;

    bool done = false;
    while(!done) {
        NextToken();
        if(NFX_TOK_PASS == g_nextToken) {
            R::PassDesc passDesc;
            ClearPassDesc(passDesc);
            if(!ParsePass(passDesc)) return false;
            desc.passes.push_back(passDesc);
        } else if(NFX_TOK_SORTKEY == g_nextToken) {
            NextToken();
            if(!ExpectToken(NFX_TOK_EQUALS)) return false;
            NextToken();
            if(!ExpectToken(NFX_TOK_VAL_INT)) return false;
            if(0 > nfx_val_int) common.printf("%s:%d: WARNING, assigning int to unsigned int as sortkey\n");
            desc.sortKey = (unsigned)nfx_val_int;
            NextToken();
            if(!ExpectToken(NFX_TOK_SEMICOL)) return false;
        } else if(NFX_TOK_RBRACE == g_nextToken) {
            done = true;
        } else {
            common.printf("%s:%d: expected { '%s', '%s', '%s' }, got '%s'\n",
                g_filename, yynfxlineno, 
                TokToString(NFX_TOK_PASS), TokToString(NFX_TOK_SORTKEY), TokToString(NFX_TOK_RBRACE), 
                yynfxtext);
            return 0;
        }
    } // while(!done)
    return true;
}