#pragma once

#ifdef CALC_EXPORTS
#define CALC_API _declspec(dllexport)
#else
#define CALC_API _declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

CALC_API
long __stdcall factorial(int n);

#ifdef __cplusplus
}
#endif
