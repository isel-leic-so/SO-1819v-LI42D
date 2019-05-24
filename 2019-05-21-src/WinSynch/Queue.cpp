#include "stdafx.h"

#include "../Include/WinSynch.h"
#include "../Include/List.h"

typedef struct QUEUE_ITEM {
	LIST_ENTRY link;
	LPVOID itemData;
} QUEUE_ITEM, *PQUEUE_ITEM;


FORCEINLINE
PQUEUE_ITEM CreateQueueEntry(LPVOID itemData) {
	PQUEUE_ITEM req = (PQUEUE_ITEM)malloc(sizeof(QUEUE_ITEM));
	req->itemData = itemData;
	return req;
}


// Assumed to be called with queue mutex owned
BOOL QueueIsEmpty(PQUEUE queue) {
	return  queue->nItems == 0;
}

VOID QueueInit(PQUEUE queue) {
	InitializeListHead(&queue->list);
	queue->itemsAvaiable = CreateSemaphore(NULL, 0, LONG_MAX, NULL);
    queue->mutex = CreateMutex(NULL, FALSE, NULL);
	queue->terminateQueue = CreateEvent(NULL, TRUE, FALSE, NULL);
	queue->terminated = FALSE;
	queue -> nItems = 0;
}



LPVOID QueueGet(PQUEUE queue) {
    HANDLE handles[2];
	PLIST_ENTRY node = NULL;

    handles[0] = queue->itemsAvaiable;
    handles[1] = queue->terminateQueue;
  
	DWORD index = WaitForMultipleObjects(2, handles, FALSE,
		INFINITE);

	WaitForSingleObject(queue->mutex, INFINITE);
	// see if there are remaing requests
	if (index == 0 || index == 1 && queue->nItems > 0) {
		// Se ainda existe algum pedido pendente, obtê-lo
		node = RemoveHeadList(&queue->list);
		queue->nItems--;
	}
    ReleaseMutex(queue->mutex);

	if (node == NULL) // close queue without pending works
		return NULL;

	PQUEUE_ITEM entry = CONTAINING_RECORD(node, QUEUE_ITEM, link);
	LPVOID data = entry->itemData;
	free(entry);
	return data;
}

//
// Put another item on the queue
//
BOOL QueuePut(PQUEUE queue, LPVOID  data) {
	PQUEUE_ITEM entry = CreateQueueEntry(data);
	BOOL terminated;
	WaitForSingleObject(queue->mutex, INFINITE);
	if (!(terminated = queue->terminated))  {
		InsertTailList(&queue->list, &entry->link);
		queue->nItems++;
	}
	ReleaseMutex(queue->mutex);
	if (terminated) return FALSE;
	ReleaseSemaphore(queue->itemsAvaiable,1, NULL);
	return TRUE;
}

//
// Called to start the termination protocol
// From this point no more QueuePut are accepted
//
VOID QueueClose(PQUEUE queue) {
	WaitForSingleObject(queue->mutex, INFINITE);
	if (!queue->terminated) {
		queue->terminated = TRUE;
		SetEvent(queue->terminateQueue);
	}
	ReleaseMutex(queue->mutex);
}

//
// Supposed to be called just once by the Queue owner
BOOL QueueDestroy(PQUEUE queue) {
	BOOL destroyed = FALSE;
	WaitForSingleObject(queue->mutex, INFINITE);
	if (queue->terminated && QueueIsEmpty(queue)) {
		CloseHandle(queue->itemsAvaiable);
		CloseHandle(queue->terminateQueue);
		destroyed = TRUE;
	}
	ReleaseMutex(queue->mutex);
	if (destroyed)
		CloseHandle(queue->mutex);
	return destroyed;
}