#pragma once

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
    SFT_BOOL,
    SFT_UINT,
    SFT_INT,
    SFT_FLOAT,
    SFT_ENUM
};

struct StateFieldDesc {
    const char* name;
    int         offset;
    int    		lchild;
    int    		rchild;
    int    		type;
};

extern StateFieldDesc g_stateDesc[];