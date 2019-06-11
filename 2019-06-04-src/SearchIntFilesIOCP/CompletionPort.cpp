// CompletionPort.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "SearchIntFilesIOCP.h"
#include "winerror.h"

#define STATUS_OK 0
#define MAX_THREADS 16

static HANDLE iocpThreads[MAX_THREADS];
static HANDLE completionPort;


// I/O Wrapers

HANDLE CreateNewCompletionPort(DWORD dwNumberOfConcurrentThreads) {
	return CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, dwNumberOfConcurrentThreads);
}

BOOL AssociateDeviceWithCompletionPort(HANDLE hComplPort, HANDLE hDevice, DWORD CompletionKey) {
	HANDLE h = CreateIoCompletionPort(hDevice, hComplPort, CompletionKey, 0);
	return h == hComplPort;
}

HANDLE OpenAsync(PCSTR fName, DWORD permissions) {
	HANDLE hFile = CreateFileA(fName, permissions,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE) return NULL;
	if (!AssociateDeviceWithCompletionPort(completionPort, hFile, 
		(ULONG_PTR)hFile)) {
		CloseHandle(hFile);
		return NULL;
	}
	return hFile;
}


BOOL ReadAsync(HANDLE hFile, DWORD toRead, POPER_CTX opCtx) {
	if (!ReadFile(hFile, opCtx->buffer, toRead, NULL, &opCtx->ovr)) {
		return GetLastError() == ERROR_IO_PENDING;
	}
	return TRUE;
}


// IOCP Worker function

// 
// Search on I/O buffer
//
DWORD SearchBuffer(PINT buffer, DWORD nInts, INT refNum) {
	DWORD ocorrences = 0;
	for (DWORD i = 0; i < nInts; ++i)
		if (buffer[i] == refNum) ocorrences++;
	return ocorrences;
}

VOID DispatchAndReleaseOper(POPER_CTX opCtx, DWORD status) {
	opCtx->cb(opCtx->userCtx, status, opCtx->ocorrences);
	DestroyOpContext(opCtx);
}

VOID ProcessRequest(POPER_CTX opCtx, DWORD transferedBytes) {
	if (transferedBytes == 0) { // operation done, call callback!
		DispatchAndReleaseOper(opCtx, STATUS_OK);
		return;
	}
	DWORD nValues = transferedBytes / sizeof(int);
	opCtx->ocorrences += SearchBuffer((PINT)opCtx->buffer, nValues, opCtx->refNum);
	
	// adjust current read position
	LARGE_INTEGER li;
	opCtx->currPos += transferedBytes;
	li.QuadPart = opCtx->currPos;
	// adjust overlapped offset
	opCtx->ovr.Offset = li.LowPart;
	opCtx->ovr.OffsetHigh = li.HighPart;
	if (!ReadAsync(opCtx->fIn, BUFFER_SIZE, opCtx)) {
		// error on operation, abort calling user callback
		DispatchAndReleaseOper(opCtx, GetLastError());
	}
}

DWORD WINAPI IOCP_ThreadFunc(LPVOID arg) {
	DWORD transferedBytes;
	ULONG_PTR completionKey;
	POPER_CTX opCtx;

	while (TRUE) {
		BOOL res = GetQueuedCompletionStatus(completionPort,
			&transferedBytes, &completionKey, (LPOVERLAPPED *)&opCtx, INFINITE);
		 
		if (!res) {
			transferedBytes = 0;
			DWORD error = GetLastError();
			if (error != ERROR_HANDLE_EOF) {
				// operation error, abort calling callback
				DispatchAndReleaseOper(opCtx, error);
				continue;
			}
		}
		ProcessRequest(opCtx, transferedBytes);
	}
	return 0;
}

BOOL IOCP_Init() {
	completionPort = CreateNewCompletionPort(0);
	if (completionPort == NULL) return FALSE;
	for (int i = 0; i < MAX_THREADS; ++i) {
		iocpThreads[i] = CreateThread(NULL, 0, IOCP_ThreadFunc, NULL, 0, NULL);
	}
	return TRUE;
}

 
