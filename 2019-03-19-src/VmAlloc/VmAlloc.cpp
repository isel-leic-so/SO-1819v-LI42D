// src.cpp : Defines the entry point for the console application.
//
// Checking the allocation in two separated phases: reserve, then committ
//
// Check system change with task manager and resource monitor.
//
// JM, 2015
//

#include "stdafx.h"

// 512 MBytes allocation!
#define MEM_SIZE (512*1024*1024)
 

int _tmain(int argc, _TCHAR* argv[])
{
	SYSTEM_INFO inf;
	DWORD i;

	// Just show the minimum reserve size (granularity size) end page size
	GetSystemInfo(&inf);
	 
	_tprintf(_T("region minimum size=%X\n"), inf.dwAllocationGranularity);
	_tprintf(_T("page size=%X\n"), inf.dwPageSize);

	_tprintf(_T("Start: press return to continue...")); getchar();

	LPBYTE buf1 = (LPBYTE) VirtualAlloc(NULL, MEM_SIZE, MEM_RESERVE, 
		PAGE_READWRITE);
	if (buf1 == NULL) {
		_tprintf(_T("Error %d reserving region!\n"), GetLastError());
		return 0;
	}

	_tprintf(_T("Reserved: press return to continue..."));getchar();

	LPBYTE buf2 = (LPBYTE) VirtualAlloc(buf1, MEM_SIZE, MEM_COMMIT, 
		PAGE_READWRITE);
	// buf1 and buf2 must be the same!
	_ASSERT(buf1 == buf2);
	_tprintf(_T("buf1=%p, buf2=%p\n"), buf1, buf2);
	_tprintf(_T("Commited: press return to continue..."));
	getchar();

	for(i=0; i < MEM_SIZE/inf.dwPageSize; ++i)
		buf1[i*inf.dwPageSize] = (BYTE) i;
	_tprintf(_T("Changed: press return to continue..."));
	getchar();

	VirtualFree(buf1, 0, MEM_DECOMMIT);
	_tprintf(_T("Decommited: press return to continue..."));
	getchar();

	VirtualFree(buf1, 0, MEM_RELEASE);
	_tprintf(_T("Released: press return to continue..."));
	getchar();

	return 0;
}


