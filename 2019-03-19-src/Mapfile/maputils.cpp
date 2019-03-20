#include "stdafx.h"

#include "mapfile.h"

// auxiliary method to convert the file access mode to file mapping access
static DWORD FileToMapAccess(int fa) {
	return ((fa & GENERIC_WRITE) != 0) ? PAGE_READWRITE : PAGE_READONLY;
}

// auxiliary method to convert the file access mode to view  access
static DWORD MapToViewAccess(int fa) {
	int va = ((fa & GENERIC_READ) != 0) ? FILE_MAP_READ : 0;
	if ((fa & GENERIC_WRITE) != 0) va |= FILE_MAP_WRITE;
	return va;
}

//--------------------------------------------------------------------------------
// This function create a memory mapping of the file with the given name. 
// The other parameters include the access type (read/readwrite), mode(create or open), 
// mapping size and an optional name of the section object
// Parameters detail:
// "fileName": the name of the file to be memory mapped. 
//           If NULL a shared memory region is created instead
// "access": the intended access (GENERIC_READ, GENERIC_WRITE or both)
// "mode":   the creation mode (OPEN_EXISTING, CREATE_ALLWAYS)
// "name":   the name of the created/used section object. If NULL an anonymous
//           section object will be created
// "size":   the size of the mapped region
// "fm":     the struct to fill with mapping or error state
// Return:
//   the function returns TRUE if the mapping creation succeeded, FALSE if an error ocurrs.
//   In that case the error state is filled
//---------------------------------------------------------------------
BOOL MapFile(LPCTSTR fileName, int access, int mode, 
	LPCTSTR name, SIZE_T size, PFILEMAP fm) {
	HANDLE fh = INVALID_HANDLE_VALUE;
	HANDLE mh = NULL;
	LPVOID mapAddress = NULL;

	if (fileName != NULL) {
		if ((fh = CreateFile(fileName, access, 0, 
			NULL, mode, 0, NULL)) == INVALID_HANDLE_VALUE)
			goto error;

		if (size == 0) {
			LARGE_INTEGER fileSize;
			if (!GetFileSizeEx(fh, &fileSize)) goto error;
			size = (SIZE_T)fileSize.QuadPart;
		}
	}
	LARGE_INTEGER aux;
	aux.QuadPart = size;

	if ((mh = CreateFileMapping(fh, NULL, 
		FileToMapAccess(access), aux.HighPart, aux.LowPart, name)) == NULL)
		goto error;

	if ((mapAddress = MapViewOfFile(mh, MapToViewAccess(access), 0, 0, 0)) == NULL)
		goto error;

	fm->baseAddress = (BYTE*)mapAddress;
	fm->mapHandle = mh;
	fm->error = 0;
	fm->size = size;
	// After creation of section object, the file handle can be closed
	CloseHandle(fh);
	return TRUE;

error:
	fm->error = GetLastError();
	if (mapAddress != NULL) UnmapViewOfFile(mapAddress);
	if (mh != NULL) CloseHandle(mh);
	if (fh != INVALID_HANDLE_VALUE) CloseHandle(fh);
	return FALSE;
}

//---------------------------------------------------------------------------
// Destroy the mapping resources (mapped region and the associated section object)
//------------------------------------------------------------------------------
VOID UnmapFile(PFILEMAP fm) {
	UnmapViewOfFile(fm->baseAddress);
	CloseHandle(fm->mapHandle);
}
