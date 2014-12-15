#pragma once

/*
this header file includes a set of headers commonly used
by most Nubuck clients
*/

#include <Nubuck\nubuck.h>
#include <Nubuck\nubuck_api.h>

#include <Nubuck\math\math.h>
#include <Nubuck\math\vector2.h>
#include <Nubuck\math\vector3.h>
#include <Nubuck\math\matrix3.h>
#include <Nubuck\math\matrix4.h>
#include <Nubuck\math\quaternion.h>

#include <Nubuck\polymesh.h>

#include <Nubuck\animation\animation.h>

#include <Nubuck\operators\operator.h>

/*
NOTE:
the client typically just passes a reference to the invoker
to the system, so a forward decl suffices.
*/
// #include <Nubuck\operators\operator_invoker.h>