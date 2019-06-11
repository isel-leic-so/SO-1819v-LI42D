// SearchIntFilesIOCP.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SearchIntFilesIOCP.h"

POPER_CTX CreateOpContext(HANDLE fIn, DWORD refNum, AsyncCallback cb, LPVOID userCtx) {
	POPER_CTX op = (POPER_CTX)calloc(1, sizeof(OPER_CTX));
	op->cb = cb;
	op->userCtx = userCtx;
	op->fIn = fIn;
	op->refNum = refNum;
 
	return op;
}

VOID DestroyOpContext(POPER_CTX ctx) {
	CloseHandle(ctx->fIn);
	free(ctx); 
}


VOID Callback(LPVOID ctx, DWORD status, UINT64 ocorrs) {
	printf("Foram encontradas %lld ocorrencias", ocorrs);
}

BOOL SearchFileAsync(PCSTR fileName, DWORD refNum,
	AsyncCallback cb, LPVOID userCtx) {
	 
	HANDLE fIn = OpenAsync(fileName, GENERIC_READ);
	if (fIn == NULL) return FALSE;

	POPER_CTX opCtx = CreateOpContext(fIn, refNum, cb, userCtx);
	if (ReadAsync(opCtx->fIn, BUFFER_SIZE, opCtx)) return TRUE;
	CloseHandle(fIn);
	DestroyOpContext(opCtx);
	return FALSE;
}

int main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("usage: SearchIntFilesIOCP <file> <refNum>\n");
		return 1;
	}
	IOCP_Init();
	if (!SearchFileAsync(argv[1], atoi(argv[2]), Callback, NULL)) {
		printf("error starting async oper!\n");
		return 1;
	}

	if (!SearchFileAsync(argv[1], atoi(argv[2]), Callback, NULL)) {
		printf("error starting async oper!\n");
		return 1;
	}



	printf("press enter to terminate\n");
	getchar();

    return 0;
}

