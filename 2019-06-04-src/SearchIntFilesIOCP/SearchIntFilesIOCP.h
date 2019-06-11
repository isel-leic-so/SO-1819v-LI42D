#pragma once
 
extern HANDLE completionPort;

#define BUFFER_SIZE 4096

typedef struct OperCtx *POPER_CTX;
typedef VOID(*AsyncCallback)(LPVOID userCtx, DWORD status, UINT64 ocorrences);
 
typedef struct OperCtx {
	OVERLAPPED ovr;
	HANDLE fIn;
	BYTE buffer[BUFFER_SIZE];
	DWORD ocorrences;
	DWORD refNum;
	UINT64 currPos;
	AsyncCallback cb;
	LPVOID userCtx;
} OPER_CTX, *POPER_CTX;

BOOL IOCP_Init();
HANDLE OpenAsync(PCSTR fName, DWORD permissions);
BOOL ReadAsync(HANDLE hFile, DWORD toRead, POPER_CTX opCtx);
 

POPER_CTX CreateOpContext(HANDLE fIn, HANDLE fOut, AsyncCallback cb, LPVOID userCtx);
VOID DestroyOpContext(POPER_CTX ctx);