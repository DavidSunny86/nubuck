#pragma once

#ifdef NUBUCK_LIB
#define NUBUCK_API __declspec(dllexport)
#else
#define NUBUCK_API __declspec(dllimport)
#endif

#define NUBUCK_OPERATOR extern "C" __declspec(dllexport)