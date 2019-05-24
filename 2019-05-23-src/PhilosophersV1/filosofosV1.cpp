// filosofosV3.cpp : Defines the entry point for the console application.
//
// Pseudo-solução obtendo um "pauzinho" de cada vez..
// pode gerar deadlock!
//
// Descomente as chamadas a SwitchToThread para ver o efeito
//
// Pode-se pensar em embrulhar numa secção crítica a obtenção dos 2 pauzinhos
// Tal solução não permite o máximo paralelismo pois inibe outros de obter pauzinhos
// se alguem ficar bloqueado na secção crítica.
//
// para ver o efeito

#include "stdafx.h"

#define NSTICKS 5
HANDLE  sticks[NSTICKS];
HANDLE  philosofers[NSTICKS];

UINT WINAPI Philosofer(LPVOID arg) {
	DWORD i = (DWORD) arg;
	/*_tprintf(_T("filosofo  %d\n"), i);*/
	 
	while (TRUE) {
		// try to eat...
		_tprintf(_T("Philosofer %d try to eat...\n"), i);
		WaitForSingleObject(sticks[i], INFINITE);
		SwitchToThread();
		WaitForSingleObject(sticks[(i+1) % NSTICKS], INFINITE);

		// eat...
		_tprintf(_T("Philosofer %d eating...\n"), i);
		//Sleep(2000+ (rand() % 3000));

		// finish eating...
		ReleaseMutex(sticks[i]);
		ReleaseMutex(sticks[(i+1) % NSTICKS]);
		
		// thinking ...
		_tprintf(_T("Philosofer %d thinking...\n"), i);
	}
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	int i=0;

	
    /* inicialize sticks */
	for (i=0; i < NSTICKS; ++i)
		sticks[i] =  CreateMutex(NULL, FALSE, NULL);

	/* create Philosophers */
	for (i=0; i < NSTICKS; ++i)
		philosofers[i] =  (HANDLE) _beginthreadex(NULL,0, Philosofer, (LPVOID) i, 0, NULL);   

	getchar();
	return 0;
}

