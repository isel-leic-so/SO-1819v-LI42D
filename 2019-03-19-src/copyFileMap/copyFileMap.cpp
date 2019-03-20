/* copyFile.cpp : Defines the entry point for the console application. */

#include "stdafx.h"

#define MBYTE (1024*1024)
#define BLOCKSIZE (512*MBYTE) // 512Mb  copy  block

int MyCopyFile(LPCTSTR srcFile, LPCTSTR dstFile) {
	LARGE_INTEGER fileSize;
	HANDLE hSrc=INVALID_HANDLE_VALUE, hDst=INVALID_HANDLE_VALUE;
	HANDLE MapSrc=NULL, MapDst=NULL;
	BYTE *src = NULL, *dst = NULL;

	int error=0;

	
    hSrc = CreateFile(srcFile, 
				 GENERIC_READ ,
				 FILE_SHARE_READ,
				 NULL,
				 OPEN_EXISTING,
				 FILE_ATTRIBUTE_NORMAL,
				 NULL);
	if (hSrc == INVALID_HANDLE_VALUE) {
		error = -1;
		goto terminate;
	}
	 
	hDst = CreateFile(dstFile, 
				 GENERIC_WRITE | GENERIC_READ,
				 0,
				 NULL,
				 CREATE_ALWAYS,
				 FILE_ATTRIBUTE_NORMAL,
				 NULL);

	if (hDst == INVALID_HANDLE_VALUE) {
		error= -2;
		goto terminate;
	}

	if (!GetFileSizeEx(hSrc, &fileSize)) {
		error= -3;
		goto terminate;
	}
	// force the original file size on the destination file
	if (!SetFilePointerEx(hDst,fileSize,NULL,FILE_BEGIN)) {
		error= -3;
		goto terminate;
	}

	if (!SetEndOfFile(hDst)) {
		error= -3;
		goto terminate;
	}

	if ( (MapSrc = CreateFileMapping(hSrc,
				          NULL, 
						  PAGE_READONLY,
						  0,
						  0,
						  NULL)) == NULL) {
		error= -4;
		goto terminate;
	}
	if ( (MapDst= CreateFileMapping(hDst,
				          NULL, 
						  PAGE_READWRITE,
						  0,
						  0,
						  NULL)) == NULL) {
	
		error= -5;
		goto terminate;
	}

 
	LARGE_INTEGER remaining = fileSize;
	LARGE_INTEGER offset;
	offset.QuadPart = 0;

	// copy the file with successive views 
	// why not copy the file with a single full view?
	while (remaining.QuadPart > (LONGLONG)0) {

		printf("."); fflush(stdout);
		DWORD toTransfer = remaining.QuadPart < BLOCKSIZE 
			? (DWORD) remaining.QuadPart : BLOCKSIZE;
		if ((src = (LPBYTE) MapViewOfFile(MapSrc, 
							FILE_MAP_READ,
							offset.HighPart,
							offset.LowPart,
							toTransfer)) == NULL) {
				error= -7;
				goto terminate;
		}

	
		if ((dst = (BYTE *) MapViewOfFile(MapDst, 
						FILE_MAP_WRITE,
						offset.HighPart,
						offset.LowPart,
						toTransfer))== NULL) {
			error= -8;
			goto terminate;
		}

		memcpy(dst, src, toTransfer);

		offset.QuadPart += toTransfer;
		remaining.QuadPart -= toTransfer;
		UnmapViewOfFile(src);
		UnmapViewOfFile(dst);
	}
	printf("\n");
terminate:
	if (src!=NULL)	UnmapViewOfFile(src);
	if (dst!=NULL)  UnmapViewOfFile(dst);
	if (MapSrc!=NULL)	CloseHandle(MapSrc);
	if (MapDst!=NULL)  CloseHandle(MapDst);
	if (hSrc != INVALID_HANDLE_VALUE) CloseHandle(hSrc);
	if (hDst != INVALID_HANDLE_VALUE) CloseHandle(hDst);
	return error;
}

int _tmain(int argc, _TCHAR* argv[])
{
	int res;

	DWORD ti, tf;
 
	if (argc != 3) {
		_tprintf(_T("Usage: copyfile <src> <dst>\n"));
		return 1;
	}
	
	ti = GetTickCount();
	if ((res=MyCopyFile(argv[1], argv[2])) < 0) {
		_tprintf(_T("error %d copying file. System error=%d"), res, 
			GetLastError());
		exit(1);
	}
	tf = GetTickCount();
	_tprintf(_T("\ndone in %dms!\n"), tf-ti);
	return 0;
}

