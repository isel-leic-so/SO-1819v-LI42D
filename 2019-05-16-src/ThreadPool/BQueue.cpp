#include "stdafx.h"
#include <stdlib.h>
#include <crtdbg.h>

#include "../Include/BQueue.h"

typedef struct {
	LIST_ENTRY link;
	LPVOID item;
} QNODE, *PQNODE;

static PQNODE createNode(LPVOID item) {
	PQNODE n = (PQNODE)malloc(sizeof(QNODE));
	n->item = item;
	return n;
}

static VOID destroyNode(PQNODE n) {
	free(n);
}


UTILS_API VOID BQ_Init(PBQUEUE q) {
	InitializeListHead(&q->list);
	q->mutex = CreateMutex(NULL, FALSE, NULL);
	q->hasItems = CreateSemaphore(NULL, 0, MAXINT, NULL);
}
typedef struct {
	LIST_ENTRY list;
	HANDLE mutex;
	HANDLE hasItems;
} BQUEUE, *PBQUEUE;

typedef struct {
	LIST_ENTRY link;
	LPVOID item;
} QNODE, *PQNODE;

UTILS_API VOID BQ_Put(PBQUEUE q, LPVOID item) {
	PQNODE node = createNode(item);

	WaitForSingleObject(q->mutex, INFINITE);
	InsertTailList(&q->list, &node->link);
	ReleaseMutex(q->mutex);

	ReleaseSemaphore(q->hasItems, 1, NULL);

}

UTILS_API LPVOID BQ_Get(PBQUEUE q) {
	
	WaitForSingleObject(q->hasItems, INFINITE);
	WaitForSingleObject(q->mutex, INFINITE);

	PQNODE node = (PQNODE)RemoveHeadList(&q->list);
	PVOID val = node->item;
	ReleaseMutex(q->mutex);
	
	return val;
}

BOOL BQ_IsEmpty(PBQUEUE q) {
	return IsListEmpty(&q->list);
}