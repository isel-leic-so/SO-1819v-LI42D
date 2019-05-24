#include "stdafx.h"

#include <stdio.h>
#include "../Include/WinSynch.h"
#include "../Include/List.h"



// Assumed to be called with queue mlock owned
BOOL SQueueIsEmpty(PSQUEUE queue) {
	return  queue->nItems == 0;
}

VOID SQueueInit(PSQUEUE queue, DWORD maxItems) {
	InitializeListHead(&queue->list);
	queue->maxItems = maxItems;
	queue->itemsAvaiable = CreateSemaphore(NULL, 0, maxItems, NULL);
	queue->spaceAvaiable = CreateSemaphore(NULL, maxItems, maxItems, NULL);
	queue->terminated = FALSE;
	queue->closingQueue = CreateEvent(NULL, TRUE, FALSE, NULL);
	InitializeCriticalSection(&queue->lock);
	queue->nItems = 0;
}



PLIST_ENTRY SQueueGet(PSQUEUE queue) {
	HANDLE handles[] = { queue->itemsAvaiable, queue->closingQueue };
	printf("enter wait!\n");
	int index = WaitForMultipleObjects(2, handles, FALSE, INFINITE);
	index -= WAIT_OBJECT_0;
	printf("leaving wait with index == %d!\n", index);
	EnterCriticalSection(&queue->lock);
	
	if (index==1 ) {
		printf("Empty queue!\n");
		fflush(stdout);
		LeaveCriticalSection(&queue->lock);
		return NULL;
	}
	//printf("Queue nItems = %d\n", queue->nItems);
	
	PLIST_ENTRY node = RemoveHeadList(&queue->list);
	queue->nItems--;
	LeaveCriticalSection(&queue->lock);
	ReleaseSemaphore(queue->spaceAvaiable, 1, NULL);
	return node;
}

//
// Put another item on the queue
//
BOOL SQueuePut(PSQUEUE queue, PLIST_ENTRY  node) {
	BOOL itemPutted;

	WaitForSingleObject(queue->spaceAvaiable, INFINITE);
	EnterCriticalSection(&queue->lock);
	if ((itemPutted = !queue->terminated)) {
		InsertTailList(&queue->list, node);
		queue->nItems++;
	
	}	
	 
	LeaveCriticalSection(&queue->lock);
	if (itemPutted) ReleaseSemaphore(queue->itemsAvaiable, 1, NULL);
	return itemPutted;
}


BOOL SQueueClose(PSQUEUE queue) {
	BOOL res = FALSE;
	EnterCriticalSection(&queue->lock);
	if (!queue->terminated) {
		res = queue->terminated = true;
		// to implement termination protocol
		SetEvent(queue->closingQueue);
	}
	LeaveCriticalSection(&queue->lock);
	return res;

}

//
// Supposed to be called just once by the Queue owner
VOID SQueueDestroy(PSQUEUE queue) {
	CloseHandle(queue->itemsAvaiable);
	CloseHandle(queue->spaceAvaiable);
	DeleteCriticalSection(&queue->lock); 
}