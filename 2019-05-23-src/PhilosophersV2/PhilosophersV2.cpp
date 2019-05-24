// PhilosophersV2.cpp : Defines the entry point for the console application.
//
// Solução do problema dos Filósofos  
// utilizando a primitiva de espera múltipla da API do Windows

#include "stdafx.h"

#define NSTICKS 5
HANDLE sticks[NSTICKS];


UINT WINAPI Philosopher(LPVOID arg) {
	DWORD i = (DWORD)arg;

	while (TRUE) {
		HANDLE mysticks[] = { sticks[i], sticks[(i + 1) % NSTICKS] };

		// try to eat...
		_tprintf(_T("Philosofer %d try to eat...\n"), i);

		int res = WaitForMultipleObjects(2, mysticks, TRUE, INFINITE);

		// eat...
		_tprintf(_T("Philosofer %d eating...\n"), i);

		// finish eating...
		ReleaseMutex(mysticks[0]);
		ReleaseMutex(mysticks[1]);

		// thinking ...
		_tprintf(_T("Philosofer %d thinking...\n"), i);
	}
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	int i = 0;

	/* inicialize sticks */
	for (i = 0; i < NSTICKS; ++i)
		sticks[i] = CreateMutex(NULL, FALSE,  NULL);

	/* create Philosophers */
	for (i = 0; i < NSTICKS; ++i)
		_beginthreadex(NULL, 0, Philosopher, (LPVOID)i, 0, NULL);

	getchar();
	return 0;
}

