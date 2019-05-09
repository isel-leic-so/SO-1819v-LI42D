#include "stdafx.h"

HANDLE outFileHandle = INVALID_HANDLE_VALUE;
HANDLE LaunchProcess(char *cmdLine, char* outFile) {
	PROCESS_INFORMATION pi;
	STARTUPINFOA si = { sizeof(si) };
	
	SECURITY_ATTRIBUTES sa = { 0 };
	
	sa.nLength = sizeof(sa);
	if (outFile != NULL) {
		
		sa.bInheritHandle = TRUE;
		outFileHandle =
			CreateFileA(outFile, FILE_APPEND_DATA, 
				 FILE_SHARE_READ, &sa, 
				CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (outFileHandle != INVALID_HANDLE_VALUE) {
			si.dwFlags = STARTF_USESTDHANDLES;
			si.hStdOutput = outFileHandle;
			si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
			si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
		}
			
	}
	if (!CreateProcessA(
		NULL,
		cmdLine,
		NULL,
		NULL,
		TRUE,
		0,
		NULL,
		NULL,
		&si,
		&pi)) {
		return NULL;
	}

	CloseHandle(pi.hThread);
 
	if (outFileHandle != INVALID_HANDLE_VALUE) 
		CloseHandle(outFileHandle);
	 
	return pi.hProcess;
}