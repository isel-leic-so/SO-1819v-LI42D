// TLSTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

static int tlsIdx;

DWORD WINAPI threadFunc(LPVOID arg)  {
	TlsSetValue(tlsIdx, (LPVOID)2);

	printf("On new thread: TlsGetValue=%d\n", TlsGetValue(tlsIdx));
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	tlsIdx = TlsAlloc();
	
	TlsSetValue(tlsIdx, (LPVOID)1);

	HANDLE th = CreateThread(NULL, 0, threadFunc, NULL, 0, NULL);
	WaitForSingleObject(th, INFINITE);
	printf("On primary thread: TlsGetValue=%d\n", TlsGetValue(tlsIdx));
	return 0;
}

