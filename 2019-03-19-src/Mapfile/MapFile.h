// to avoid multiple includes 
#pragma once


// this struct maintains the state of an active mapping
// (or the error state if some error ocurrs in the map creation)
typedef  struct filemap {
	BYTE* baseAddress;		// base address of the map view 
	SIZE_T size;			// the size of the mapping (and view)
	HANDLE mapHandle;		// section object handle  
	DWORD  error;			// state (!= 0 means error)
} FILEMAP, *PFILEMAP;


// funções exportadas 

//--------------------------------------------------------------------------------
// This function create a simple memory mapping with a single viewof the file 
// with the given name. 
// The other parameters include the access type (read/readwrite), mode(create or open), 
// mapping size and an optional name of the section object
// Parameters detail:
// "fileName": the name of the file to be memory mapped with a single view 
//           If NULL a shared memory region is created instead
// "access": the pretended access (GENERIC_READ, GENERIC_WRITE or both)
// "mode":   the creation mode (OPEN_EXISTING, CREATE_ALLWAYS)
// "name":   the name of the created/used section object. If NULL an anonymous
//           section object will be created
// "size":   the size of the mapped region. if 0 all file is mapped.
// "fm":     the struct to fill with mapping or error state
// Return:
//   the function returns TRUE if the mapping creation succeeded, FALSE if an error ocurrs.
//   In that case the error state is filled
//---------------------------------------------------------------------
BOOL MapFile(LPCTSTR fileName, int access, int mode,  LPCTSTR name, SIZE_T size, PFILEMAP fm);

//---------------------------------------------------------------------------
// Destroy the mapping resources (mapped region and the associated section object)
//------------------------------------------------------------------------------
VOID UnmapFile(PFILEMAP fm);