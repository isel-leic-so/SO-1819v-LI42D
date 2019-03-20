
#include <windows.h>
#include <psapi.h>
#include <stdio.h>

#define DATALEN (1024*1024*400)

static BYTE data[DATALEN] = { 1 };

VOID ShowPhysicalAvaiableMem(LPCSTR msg) {
	MEMORYSTATUSEX ms;
	PERFORMANCE_INFORMATION pi;
	const int MB = 1024 * 1024;

	ms.dwLength = sizeof(ms);

	printf("%s:\n", msg);
	
	if (!GlobalMemoryStatusEx(&ms))
		printf("Error %d getting available memory\n\n", GetLastError());
	else {
		__int64 used_mem = (ms.ullTotalPhys - ms.ullAvailPhys) / MB;
		__int64 free_mem =  ms.ullAvailPhys / MB;

		
		printf("Used Memory: %.1lfGB\n", (double) used_mem/1024);
		printf("Free Memory: %.1lfGB\n\n", (double) free_mem/1024);
		
	}
}

ULONG ReadData() {
	ULONG sum = 0;
	ULONG i;

	for (i = 0; i < DATALEN; ++i)
		sum += data[i];
	return sum;
}

VOID WriteData(int v)
{
	ULONG i;
	for (i = 0; i < DATALEN; ++i)
		data[i] = v;
}

VOID Pause(LPCSTR phaseName) {
	printf("Prima RETURN para %s\t[%u]\n", phaseName, GetCurrentProcessId());
	getchar();
}

int main()
{
	ShowPhysicalAvaiableMem("Inicio");
	Pause("ler");
	ReadData();
	ShowPhysicalAvaiableMem("Depois de ler");
	Pause("escrever");
	WriteData(2);
	ShowPhysicalAvaiableMem("Depois de escrever");
	Pause("terminar");
	return 0;
}
