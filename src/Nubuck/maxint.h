#pragma once

/*
this must be the first header included in source files
that use both the windows api and leda, to avoid the
redefinition of MAXINT.
*/

#include <Windows.h> // includes basetsd.h that unconditionally defines MAXINT