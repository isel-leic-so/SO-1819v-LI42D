// CopyFileAsync.cpp : Defines the entry point for the console application.
//
// Assynchronous file copy with overlapped I/O and multiple buffers with event synchronization
//
// JM, 2015

#include "stdafx.h"

#define BUFFER_SIZE (4096*16)
#define NOPERS 1

/* I/O operation type */
typedef enum OperType { READ, WRITE } OperType;

typedef struct Operation {
	OVERLAPPED overlapped;
	OperType type;
	BYTE buffer[BUFFER_SIZE];
} Operation;


/*
* Copy global state
*/

/* Input file size */
LARGE_INTEGER inFileSize;
LARGE_INTEGER currInPos;

/* pending operations */
int readOpers, writeOpers;

/* total operations */
int nOpers;

/* parallel operations */
Operation opers[NOPERS];

/* file handles */
HANDLE fIn = INVALID_HANDLE_VALUE;
HANDLE fOut = INVALID_HANDLE_VALUE;

/*
* Auxiliary functions
*/
HANDLE OpenOrCreateWithAsyncModeAccess(TCHAR *fileName, DWORD creationMode) {
	HANDLE hFile = CreateFile(fileName,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		creationMode,
		FILE_FLAG_OVERLAPPED,
		NULL);
	return hFile;
}

HANDLE OpenAsync(TCHAR *fileName) {
	return OpenOrCreateWithAsyncModeAccess(fileName, OPEN_EXISTING);
}

HANDLE CreateAsync(TCHAR *fileName) {
	return OpenOrCreateWithAsyncModeAccess(fileName, CREATE_ALWAYS);
}


/*
* Start I/O operations
* Mantain the copy process while operations are done synchronously
*/
BOOL DoIO(Operation *oper, DWORD nBytes) {
	DWORD res;
	DWORD nTransfer;
	nOpers++;
	while (TRUE)  {

		switch (oper->type) {
		case READ:
			if (currInPos.QuadPart >= inFileSize.QuadPart)
				return TRUE;


			readOpers++;
			oper->overlapped.Offset = currInPos.LowPart;
			oper->overlapped.OffsetHigh = currInPos.HighPart;

			currInPos.QuadPart += nBytes; // adjust input position
			if (!ReadFile(fIn, oper->buffer, nBytes, 
				&nTransfer, &oper->overlapped)) {
				if ((res = GetLastError()) != ERROR_IO_PENDING) {
					_tprintf(_T("Error %d reading file at offset %d!\n"), res, currInPos.LowPart);
					return FALSE;
				}
				return TRUE;
			}

			_tprintf(_T("Synchronous read!\n"));
			readOpers--;
			oper->type = WRITE;

			break;
		case WRITE:
			writeOpers++;
			if (!WriteFile(fOut, oper->buffer, nBytes,
				&nTransfer, &oper->overlapped)) {
				if ((res = GetLastError()) != ERROR_IO_PENDING) {
					_tprintf(_T("Error %d writing file!\n"), res);
					return FALSE;
				}
				return TRUE;
			}
			writeOpers--;
			_tprintf(_T("Synchronous write!\n"));
			oper->type = READ;
			break;
		}

	}
}

/*
  Called when an input operation is concluded
*/
BOOL AsyncReadDone(Operation *oper) {
	// DWORD used for the number of bytes transfered in I/O operation.
	DWORD nTransfered;

	readOpers--;

	if (!GetOverlappedResult(fIn, &oper->overlapped, &nTransfered, FALSE)) {
		printf("Error %d getting read result!\n", GetLastError());
		return FALSE;
	}
	oper->type = WRITE;
	return DoIO(oper, nTransfered);
}

/*
Called when an output operation is concluded
*/
BOOL AsyncWriteDone(Operation *oper) {
	DWORD nTransfered;

	if (!GetOverlappedResult(fOut, &oper->overlapped, &nTransfered, FALSE)) {
		printf("Error %d getting write result!\n", GetLastError());
		return FALSE;
	}

	writeOpers--;
	oper->type = READ;
	DoIO(oper, BUFFER_SIZE);
	return TRUE;
}

VOID OperInit(Operation *oper) {
	ZeroMemory(&oper->overlapped, sizeof(OVERLAPPED));
	oper->overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	oper->type = READ;
}

#define OperGetEvent(i) (opers[i].overlapped.hEvent)



//
// AsyncCopy implementation
//
BOOL CopyAsync() {
	/* array of handles for WaitForMultipleObjects */
	HANDLE waitOn[NOPERS];

	if (!GetFileSizeEx(fIn, &inFileSize)) {
		_tprintf(_T("Error %d getting input file size!\n"), GetLastError());
		return FALSE;
	}

	if (!SetFilePointerEx(fOut, inFileSize, NULL, FILE_BEGIN))
	{
		_tprintf(_T("Error %d setting output file size!\n"), GetLastError());
		return FALSE;
	}
	SetEndOfFile(fOut);

	// Start copy
	for (int i = 0; i < NOPERS; ++i) {
		OperInit(opers + i);
		waitOn[i] = OperGetEvent(i);
		DoIO(&opers[i], BUFFER_SIZE);
	}

	// Start reading the data asynchronously.
	do {
		DWORD res = WaitForMultipleObjects(NOPERS, waitOn, 
			FALSE, INFINITE);
	 
		if (res < WAIT_OBJECT_0 || 
			res >= WAIT_OBJECT_0 + NOPERS) {
			_tprintf(_T("wait error %d copying file!\n"), GetLastError());
			return FALSE;
		}

		res -= WAIT_OBJECT_0;
		if (opers[res].type == READ) { // partial input done
			//printf("partial read done at %d!\n", overlappedIn[res].Offset);
			if (!AsyncReadDone(&opers[res]))
				break;
		}
		else { // partial write done
			//printf("partial write done at %d!\n", overlappedOut[res].Offset);
			if (!AsyncWriteDone(&opers[res]))
				break;
		}
	} while (writeOpers > 0 || readOpers > 0);
	 
	return TRUE;
}


int _tmain(int argc, _TCHAR* argv[])
{
	if (argc != 3) {
		_tprintf(_T("usage: asyncopy <fin> <fout>\n"));
		return 1;
	}

	char dir[128];

	GetCurrentDirectoryA(128, dir);

	// Open the input file for asynchronous file I/O.
	fIn = OpenAsync(argv[1]);
	if (fIn == INVALID_HANDLE_VALUE) {
		_tprintf(_T("Error %d opening file!\n"), GetLastError());
		return 1;
	}

	// Create the output file for asynchronous file I/O.
	fOut = CreateAsync(argv[2]);
	if (fOut == INVALID_HANDLE_VALUE) {
		_tprintf(_T("Error %d creating file!\n"), GetLastError());
		return 1;
	}

	LONGLONG start = GetTickCount64();
	CopyAsync();
	_tprintf(_T("Done in %I64ums!\n"), GetTickCount64() - start);
	_tprintf(_T("in a total of %d operations!\n"), nOpers);
	return 0;
}


