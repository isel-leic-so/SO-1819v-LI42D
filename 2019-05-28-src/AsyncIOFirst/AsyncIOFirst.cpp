// AsyncIOFirst.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

VOID ShowBuffer(PBYTE buf, INT size);


// This is a real asynchronous read funtion since it returns 
// immediately after I/O is started.
// It doesn't alert to an eventual synchronous completion.
// 
// We assume the file handle has been opened with FILE_FLAG_OVERLAPPED as TRUE
BOOL BeginRead(HANDLE hFile, 
	            DWORD ofs, 
				PBYTE buf, 
				DWORD size,		// transfer size 
				OUT LPOVERLAPPED ovr	// the function fill the passed overlapped
				) {
	DWORD res;
	
	
	// Initialize the OVERLAPPED structure to tell the system where to start
	// reading the data.
	 
	ZeroMemory(ovr, sizeof(OVERLAPPED));
	ovr->Offset = ofs;
	 
	// Start reading the data asynchronously.
	if (!ReadFile(hFile, buf, size, NULL, ovr)) {
		if ((res = GetLastError()) != ERROR_IO_PENDING) {
			return FALSE;
		}	 
	}

	return TRUE;
}

DWORD  EndRead(HANDLE hFile, LPOVERLAPPED ovr) {
	// The thread can’t continue until we know thet all the requested data has
	// been read into our buffer.
	DWORD bytesRead;

	BOOL res = GetOverlappedResult(hFile,ovr, &bytesRead, TRUE);

	if (!res) return -1;  
	return bytesRead;
}


int _tmain(int argc, _TCHAR* argv[])
{
	// DWORD used for the number of bytes read.
	DWORD bytesRead, offset = 0;
	// Create a buffer to hold data.
	BYTE buffer[128];


	// Open the file for asynchronous file I/O.
	HANDLE hFile = CreateFile(_T("Debug/AsyncIoFirst.pch"),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		NULL);


	if (hFile == INVALID_HANDLE_VALUE) {
		_tprintf(_T("Error %d opening file!\n"), GetLastError());
		return 1;
	}

	OVERLAPPED ovr;

	printf("Start async read!\n");
	if (!BeginRead(hFile, offset, buffer, sizeof(buffer), &ovr) ) {
		_tprintf(_T("Error reading asynchronously!\n"));
		return 1;
	}
	
	// Code below ReadFile executes while the system reads the file’s data
	// into buffer.
	// ......

	printf("Do other work!\n");
	Sleep(3000);

	printf("Get read result!\n");
	// The thread can’t continue until we know thet all the requested data has
	// been read into our buffer.
	if ((bytesRead = EndRead(hFile, &ovr)) > 0) 
		ShowBuffer(buffer, bytesRead);
	 

	return 0;
}

