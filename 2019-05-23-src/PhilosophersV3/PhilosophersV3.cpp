// PhilosophersV3.cpp : Defines the entry point for the console application.
//
// Nesta versão tira-se partido do seguinte princípio:
//
// Se numerarmos os recursos de 0 a N-1 (ou qualquer outra numeração crescente)
// não pode haver deadlock se os recursos forem pedidos pela mesma ordem por todos os intervenientes
// Para o conseguir troca-se a lógica de obtenção do filósofo 4 (admitindo que são 5).
//

#include "stdafx.h"

#define NSTICKS 5

HANDLE  sticks[NSTICKS];

UINT WINAPI Philosopher(LPVOID arg) {
	DWORD i = (DWORD)arg;
	/*_tprintf(_T("filosofo  %d\n"), i);*/

	while (TRUE) {

		// try to eat...
		if (i == NSTICKS - 1) {
			WaitForSingleObject(sticks[0], INFINITE);
			Sleep(10);
			WaitForSingleObject(sticks[i], INFINITE);
		}
		else {
			WaitForSingleObject(sticks[i], INFINITE);
			Sleep(10);
			WaitForSingleObject(sticks[(i + 1) % NSTICKS], INFINITE);
		}

		// eat...

		_tprintf(_T("Philosofer %d eating...\n"), i);
		//Sleep( 2 + rand() % 30 /*2000+ (rand() % 3000)*/);
		// finish eating...

		ReleaseMutex(sticks[i]);
		ReleaseMutex(sticks[(i + 1) % NSTICKS]);

		// thinking ...
		_tprintf(_T("Philosofer %d thinking...\n"), i);
		//Sleep( 2 + rand() % 30 /*2000+ (rand() % 3000)*/);
	}
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	int i = 0;


	/* inicialize sticks */
	for (i = 0; i < NSTICKS; ++i)
		sticks[i] = CreateMutex(NULL, FALSE, NULL);


	/* create Philosophers */
	for (i = 0; i < NSTICKS; ++i)
		_beginthreadex(NULL, 0, Philosopher, (LPVOID)i, 0, NULL);

	getchar();
	return 0;
}


