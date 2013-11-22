#include <common\common.h>
#include "nfx_local.h"

static const char*  g_filename;
static int          g_nextToken;

GLboolean   nfx_val_bool;
GLenum      nfx_val_enum;

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
    TOKTOSTRING_CASE(NFX_TOK_VAL_BOOL);
    TOKTOSTRING_CASE(NFX_TOK_VAL_ENUM);
    default:
        assert(0 && "TokToString: unknown token");
    };
}

static void NextToken(void) {
    g_nextToken = yylex();
}

static bool ExpectToken(int tok) {
    if(tok != g_nextToken) {
        const char* tokName = TokToString(tok);
        printf("%s:%d: expected '%s', got '%s'\n", g_filename, yylineno, tokName, yytext);
        return false;
    }
    return true;
}

/*
g_stateDesc describes the State structure  used by the renderer (cnf. file TODO)
It's used to get the offset of fields by name.
g_stateDesc is an array of elements of type StateFieldDesc. 
Let F be the field of State described by g_stateDesc[i], for some i.
If F is a structure, g_stateDesc[F.lchild] ... g_stateDesc[F.rchild] are
the member fields of F (F.rchild is inclusive).
*/

enum StateFieldType {
    SFT_STRUCT  = 0,
    SFT_BOOL    = 1
};

struct StateFieldDesc {
    const char* name;
    int         offset;
    int    		lchild;
    int    		rchild;
    int    		type;
};

StateFieldDesc g_stateDesc[] = {
    { "state", 0, 1, 1, SFT_STRUCT },
    { "culling", offsetof(R::State, culling), 2, 2, SFT_STRUCT },
    { "hw", offsetof(R::State, culling) + offsetof(R::State::Culling, hw), 3, 3, SFT_STRUCT },
    { "enabled", offsetof(R::State, culling) + offsetof(R::State::Culling, hw) + offsetof(R::State::Culling::Hardware, enabled), 0, 0, SFT_BOOL }
};

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
    switch(g_stateDesc[fidx].type) {
    case SFT_BOOL:
        if(!ExpectToken(NFX_TOK_VAL_BOOL)) return false;
        memcpy(&desc.state, &nfx_val_bool, sizeof(GLboolean));
        break;
    case SFT_STRUCT:
        printf("%s:%d: assigning value to struct\n", g_filename, yylineno);
        return false;
    default:
        assert(0);
    }
    NextToken();
    if(!ExpectToken(NFX_TOK_SEMICOL)) return false;
    return true;
}

static bool ParseField(R::PassDesc& desc, int fidx) {
    fidx = FieldIndex(fidx, yytext);
    if(0 > fidx) {
        printf("%s:%d: unknown state '%s'\n", g_filename, yylineno, yytext);
        return false;
    }
    bool done = false;
    while(!done) {
        NextToken();
        if(NFX_TOK_DOT == g_nextToken) { // member field
            NextToken();
            if(!ExpectToken(NFX_TOK_IDENT)) return false;
            fidx = FieldIndex(fidx, yytext);
            if(0 > fidx) {
                printf("%s:%d: unknown state '%s'\n", g_filename, yylineno, yytext);
                return false;
            }
        } else if(NFX_TOK_LBRACE == g_nextToken) { // field block
            NextToken();
            if(NFX_TOK_IDENT == g_nextToken) {
                if(!ParseField(desc, fidx)) return false;
                NextToken();
                if(!ExpectToken(NFX_TOK_RBRACE)) return false;
                done = true;
            } else if(NFX_TOK_RBRACE == g_nextToken) {
                // done
            } else {
                printf("%s:%d: expected '%s' or '%s', got '%s'\n",
                    g_filename, yylineno, 
                    TokToString(NFX_TOK_IDENT), TokToString(NFX_TOK_RBRACE), 
                    yytext);
                return false;
            }
        } else if(NFX_TOK_EQUALS == g_nextToken) {
            if(!ParseValueAssignment(desc, fidx)) return false;
            done = true;
        } else {
            printf("%s:%d: expected '%s' or '%s', got '%s'\n",
                g_filename, yylineno, 
                TokToString(NFX_TOK_DOT), TokToString(NFX_TOK_EQUALS), 
                yytext);
            return false;
        }
    } // while(!done) (parsing field)
    return true;
}

static bool ParsePass(R::PassDesc& desc) {
    NextToken();
    if(!ExpectToken(NFX_TOK_IDENT)) return false;
    desc.name = yytext;
    
    NextToken();
    if(!ExpectToken(NFX_TOK_LBRACE)) return false;

    bool done = false;
    while(!done) {
        NextToken();
        if(NFX_TOK_IDENT == g_nextToken) { // field[...] = val;
            if(!ParseField(desc, -1)) return false;
        } else if(NFX_TOK_RBRACE == g_nextToken) {
            done = true;
        } else {
            printf("%s:%d: expected '%s' or '%s', got '%s'\n",
                g_filename, yylineno, 
                TokToString(NFX_TOK_IDENT), TokToString(NFX_TOK_RBRACE), 
                yytext);
            return false;
        }
    } // while(!done) (parsing pass' body)

    return true;
}

bool NFX_StartParsing(const char* filename, R::EffectDesc& desc) {
    printf("NFX_StartParsing\n");

    g_filename  = filename;
    g_nextToken = 0;

    desc.sortKey    = 0;
    desc.name       = "";
    desc.passes.clear();

    NextToken();
    if(!ExpectToken(NFX_TOK_FX)) return false;

    NextToken();
    if(!ExpectToken(NFX_TOK_IDENT)) return false;
    desc.name = yytext;

    NextToken();
    if(!ExpectToken(NFX_TOK_LBRACE)) return false;

    NextToken();
    if(NFX_TOK_PASS == g_nextToken) {
        R::PassDesc passDesc;
        if(!ParsePass(passDesc)) return false;
        desc.passes.push_back(passDesc);
    } else if(NFX_TOK_RBRACE == g_nextToken) {
        // done
    } else {
        printf("%s:%d: expected '%s' or '%s', got '%s'\n",
            g_filename, yylineno, 
            TokToString(NFX_TOK_PASS), TokToString(NFX_TOK_RBRACE), 
            yytext);
        return 0;
    }
    return true;
}