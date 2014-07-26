#include <assert.h>
#include <string>
#include <vector>
#include <queue>
#include <stack>
#include "statedesc_gen_local.h"

struct StateDescNode {
    StateDescNode*  parent;
    StateDescNode*  children;
    StateDescNode*  siblings;
    int             numChildren;
    std::string     typeName;
    std::string     name;
    StateFieldType  vartype;
    int             lchild, rchild;

    StateDescNode(void) : parent(NULL), children(NULL), siblings(NULL), numChildren(0) { }
};

StateFieldType stg_fieldType;

static StateDescNode*               g_root = NULL;
static std::vector<StateDescNode*>  g_allocNodes;

static const char*  g_inname = NULL;
static int          g_nextToken = STG_TOK_UNKNOWN;

#define STG_ENUM_TO_STRING_CASE(tok) case tok: return #tok
static const char* VarToString(int var) {
    switch(var) {
    STG_ENUM_TO_STRING_CASE(SFT_STRUCT);
    STG_ENUM_TO_STRING_CASE(SFT_BOOL);
    STG_ENUM_TO_STRING_CASE(SFT_UINT);
    STG_ENUM_TO_STRING_CASE(SFT_INT);
    STG_ENUM_TO_STRING_CASE(SFT_FLOAT);
    STG_ENUM_TO_STRING_CASE(SFT_ENUM);
    default:
        assert(0 && "VarToString: unknown variable type");
        return "unknown";
    };
}
static const char* TokToString(int tok) {
    switch(tok) {
    STG_ENUM_TO_STRING_CASE(STG_TOK_UNKNOWN);
    STG_ENUM_TO_STRING_CASE(STG_TOK_EOF);
    STG_ENUM_TO_STRING_CASE(STG_TOK_SEMICOL);
    STG_ENUM_TO_STRING_CASE(STG_TOK_LBRACE);
    STG_ENUM_TO_STRING_CASE(STG_TOK_RBRACE);
    STG_ENUM_TO_STRING_CASE(STG_TOK_STRUCT);
    STG_ENUM_TO_STRING_CASE(STG_TOK_VARTYPE);
    STG_ENUM_TO_STRING_CASE(STG_TOK_IDENT);
    default:
        assert(0 && "TokToString: unknown token");
        return "unknown";
    };
}

static void NextToken(void) {
    g_nextToken = yystglex();
}

static bool ExpectToken(int tok) {
    if(tok != g_nextToken) {
        const char* tokName = TokToString(tok);
        printf("%s:%d: expected '%s', got '%s'\n", g_inname, yystglineno, tokName, yystgtext);
        return false;
    }
    return true;
}

static StateDescNode* AllocNode(void) {
    StateDescNode* node = new StateDescNode();
    g_allocNodes.push_back(node);
    return node;
}

static void FreeNodes(void) {
    for(unsigned i = 0; i < g_allocNodes.size(); ++i)
        delete g_allocNodes[i];
    g_allocNodes.clear();
}

static StateDescNode* ParseStruct(void) {
    StateDescNode* node = AllocNode();
    NextToken();
    if(!ExpectToken(STG_TOK_IDENT)) return NULL;
    node->typeName = yystgtext;
    node->vartype = SFT_STRUCT;
    NextToken();
    if(!ExpectToken(STG_TOK_LBRACE)) return NULL;

    StateDescNode* child        = NULL;
    StateDescNode* lastChild    = NULL;

    bool done = false;
    while(!done) {
        child = NULL;
        NextToken();
        if(STG_TOK_VARTYPE == g_nextToken) {
            child = AllocNode();
            child->vartype = stg_fieldType;
            NextToken();
            if(!ExpectToken(STG_TOK_IDENT)) return NULL;
            child->name = yystgtext;
            NextToken();
            if(!ExpectToken(STG_TOK_SEMICOL)) return NULL;
        } else if(STG_TOK_STRUCT == g_nextToken) {
            if(!(child = ParseStruct())) return NULL;
        } else if(STG_TOK_RBRACE == g_nextToken) {
            NextToken();
            if(STG_TOK_IDENT == g_nextToken) {
                node->name = yystgtext;
                NextToken();
            }
            if(!ExpectToken(STG_TOK_SEMICOL)) return NULL;
            done = true;
        }
        if(child) {
            if(!node->children) node->children = child;
            if(lastChild) lastChild->siblings = child;
            lastChild = child;
            child->parent = node;
            node->numChildren++;
        }
    }
    return node;
}

static void STG_ComputeChildIndices(StateDescNode* root) {
    std::queue<StateDescNode*> Q;
    int idx = 1;
    Q.push(root);
    while(!Q.empty()) {
        StateDescNode* node = Q.front();
        Q.pop();
        node->lchild = idx;
        node->rchild = idx + node->numChildren - 1;
        idx += node->numChildren;

        StateDescNode* it = node->children;
        while(it) {
            Q.push(it);
            it = it->siblings;
        }
    }
}

static std::string STG_GetQualifier(StateDescNode* node) {
    std::stack<StateDescNode*> S;
    StateDescNode* it = node->parent;
    while(it) {
        S.push(it);
        it = it->parent;
    }
    std::string name = "R::";
    while(!S.empty()) {
        name += S.top()->typeName + "::";
        S.pop();
    }
    return name;
}

static std::string STG_ComputeOffsetString(StateDescNode* node) {
    if(!node->parent) return "0"; // offsetof() + ... + 0
    std::string structName = STG_GetQualifier(node->parent) + node->parent->typeName;
    std::string off = std::string("offsetof(") + structName + ", " + node->name + ") + ";
    off += STG_ComputeOffsetString(node->parent);
    return off;
}

static void STG_PrintOffsets(StateDescNode* root) {
    std::queue<StateDescNode*> Q;
    Q.push(root);
    while(!Q.empty()) {
        StateDescNode* node = Q.front();
        Q.pop();
        printf("node: '%s', off: '%s'\n", node->name.c_str(), STG_ComputeOffsetString(node).c_str());
        
        StateDescNode* it = node->children;
        while(it) {
            Q.push(it);
            it = it->siblings;
        }
    }
}

const char* prelude =
    "/*                                                             \n"
    "this file is automatically generated.                          \n"
    "changes will be overwritten.                                   \n"
    "*/                                                             \n"
    "#include <stddef.h>                                            \n"
    "#include <renderer\\effects\\statedesc.h>                      \n"
    "#include <renderer\\effects\\state.h>                          \n"
    "                                                               \n"
    "StateFieldDesc g_stateDesc[] = {                               \n";
const char* postlude =
    "{ NULL, 0, 0, 0, 0 }                                           \n"
    "};                                                             \n";

static void STG_WriteStateDesc(StateDescNode* root, const char* outname) {
    FILE* file = fopen(outname, "w");
    if(!file) {
        printf("unable to open file '%s'\n", outname);
        return;
    }

    fprintf(file, "%s", prelude);

    std::queue<StateDescNode*> Q;
    Q.push(root);
    while(!Q.empty()) {
        StateDescNode* node = Q.front();
        Q.pop();

        fprintf(file, "{ \"%s\", %s, %d, %d, %s }, \n",
            node->name.c_str(), STG_ComputeOffsetString(node).c_str(),
            node->lchild, node->rchild,
            VarToString(node->vartype));

        StateDescNode* it = node->children;
        while(it) {
            Q.push(it);
            it = it->siblings;
        }
    }

    fprintf(file, "%s", postlude);
    fclose(file);
}

bool STG_StartParsing(const char* inname, const char* outname) {
    g_allocNodes.resize(128);
    g_inname = inname;

    bool done = false;
    while(!done) {
        NextToken();
        if(STG_TOK_EOF == g_nextToken) done = true;
        if(STG_TOK_STRUCT == g_nextToken) {
            if(g_root) {
                printf("STG_StartParsing: multiple top-level structs in '%s', abort.\n", inname);
                return false;
            }
            if(!(g_root = ParseStruct())) return false;
        }
    }

    // add proxy name for root
    assert("" == g_root->name);
    g_root->name = "state";

    STG_ComputeChildIndices(g_root);
    STG_WriteStateDesc(g_root, outname);

    FreeNodes();

    return true;
}