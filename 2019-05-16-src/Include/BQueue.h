#pragma once

#ifdef THREADPOOL_EXPORTS
#define UTILS_API __declspec(dllexport)
#else
#define UTILS_API __declspec(dllimport)
#endif

#include "List.h"

typedef struct {
	LIST_ENTRY list;
	HANDLE mutex;
	HANDLE hasItems;
} BQUEUE, *PBQUEUE;


UTILS_API VOID BQ_Init(PBQUEUE q);
UTILS_API VOID BQ_Put(PBQUEUE q, LPVOID item);
UTILS_API LPVOID BQ_Get(PBQUEUE q);
UTILS_API BOOL BQ_IsEmpty(PBQUEUE q);