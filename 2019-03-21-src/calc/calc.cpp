// calc.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "calc.h"

long __stdcall factorial(int n) {
	long f = 1;
	while (n > 1) {
		f *= n;
		n--;
	}
	return f;
}


