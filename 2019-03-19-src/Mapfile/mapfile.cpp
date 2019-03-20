// mapfile.cpp : Defines the entry point for the console application.
//
// Exemplo de utilização da API de mapeamento de ficheiros em memória
//
// 

#include "stdafx.h"

//--------------------------------------------------
// Test program:
//    a file consisting of a sequence of ints is mapped.
//   the sequence is traversed and presented at standard output
//--------------------------------------------------------------
int _tmain(int argc, _TCHAR* argv[])
{
	 
	FILEMAP mapDesc;

	if (argc != 2) {
		_tprintf(_T("Usage: MapFile <fileName>\n"));
		return 1;
	}
	 
	if (!MapFile(argv[1], 
		GENERIC_READ|GENERIC_WRITE, OPEN_EXISTING, NULL, 0, 
		&mapDesc))
		_tprintf(_T("Error %d mapping file\n"), mapDesc.error);
	else {
		int nints = mapDesc.size / sizeof(int);
		int *curr = (int *)mapDesc.baseAddress;

		for (int i = 0; i < nints; ++i) {
			_tprintf(_T("%d\n"), curr[i]);
		}
		curr[8191] = 5;
	}
	 
	
	UnmapFile(&mapDesc);
	return 0;
}

