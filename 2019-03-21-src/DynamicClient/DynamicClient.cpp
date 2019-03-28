// DynamicClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <stdio.h>

typedef long (__stdcall *FACT_FUNC)(int n);

int main()
{
	HMODULE module = LoadLibraryA("../Debug/calc.dll");
	if (module == NULL) {
		printf("erro %d a carregar a dll", GetLastError());
		return 1;
	}
	printf("module=%p\n", module);
	FACT_FUNC factorial = (FACT_FUNC)
		GetProcAddress(module, "_factorial@4");

	if (factorial == NULL) {
		printf("erro %d a obter endereço da função", 
			GetLastError());
		return 2;
	}
	long f6 = factorial(6);
	printf("6!=%d\n", f6);
	FreeLibrary(module);
	return 0;
}
