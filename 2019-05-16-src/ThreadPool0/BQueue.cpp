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

#define MAXITEMS 20000

UTILS_API VOID BQ_Init(PBQUEUE q) {
	InitializeListHead(&q->list);
	q->mutex = CreateMutex(NULL, FALSE, NULL);
	q->hasItems = CreateSemaphore(NULL, 0, MAXITEMS, NULL);
	q->hasSpace = CreateSemaphore(NULL, MAXITEMS, MAXITEMS, NULL);
}

UTILS_API VOID BQ_Put(PBQUEUE q, LPVOID item) {
	PQNODE n = createNode(item);
	WaitForSingleObject(q->hasSpace, INFINITE);
	WaitForSingleObject(q->mutex, INFINITE);
	InsertTailList(&q->list, &n->link);
	ReleaseMutex(q->mutex);
	ReleaseSemaphore(q->hasItems, 1, NULL);
}

UTILS_API LPVOID BQ_Get(PBQUEUE q) {
	
	WaitForSingleObject(q->hasItems, INFINITE);
	WaitForSingleObject(q->mutex, INFINITE);

	PQNODE n = (PQNODE) RemoveHeadList(&q->list);
	_ASSERTE(&n->link != &q->list);
	_ASSERTE((PQNODE)&n->link == n);
	ReleaseMutex(q->mutex);
	LPVOID item = n->item;
	destroyNode(n);
	ReleaseSemaphore(q->hasSpace, 1, NULL);
	
	return item;
}

BOOL BQ_IsEmpty(PBQUEUE q) {
	return IsListEmpty(&q->list);
}