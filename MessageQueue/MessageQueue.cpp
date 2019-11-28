// MessageQueue.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>

#include "MessageQueue.h"
#include "MessageQueueImpl.h"

// local queue auxiliary functions

static VOID QueueInit(PMSG_QUEUE q, DWORD elemSize, DWORD capacity) {
	q->Capacity = capacity;
	q->ElemSize = elemSize;
	q->PutIdx = q->GetIdx = -1;
}

static VOID QueueGet(PMSG_QUEUE q, PVOID msg) {
	DWORD idx = InterlockedIncrement(&q->GetIdx);
	PBYTE msgStart = q->Msgs + (idx % q->Capacity)*q->ElemSize;
	memcpy(msg, msgStart, q->ElemSize);
}

static VOID QueuePut(PMSG_QUEUE q, PVOID msg) {
	DWORD idx = InterlockedIncrement(&q->PutIdx);
	PBYTE msgStart = q->Msgs + (idx % q->Capacity)*q->ElemSize;
	memcpy(msgStart, msg, q->ElemSize);
}


// Start of local blocking queue auxiliary functions
// this functions can ONLY be used between threads of the same process
//
static VOID MqInit( MQ_HANDLE mq, 
					PMSG_QUEUE queue, 
					DWORD elemSize, 
					DWORD capacity,
					PCSTR mqName
				   ) {
	
	QueueInit(queue, elemSize, capacity);
	mq->queue = queue;

	CHAR hasSpaceSemName[128], hasItemsSemName[128];

	if (mqName != NULL) {
		// in case the message queue has a name,
		// we create corresponding names for the mq semaphores
		sprintf_s(hasSpaceSemName, "SemHasSpace_%s", mqName);
		sprintf_s(hasItemsSemName, "SemHsItems_%s", mqName);
	}

	mq->hasSpace = CreateSemaphore(NULL, capacity, capacity,
		(mqName == NULL) ? NULL : hasSpaceSemName);
	mq->hasItems = CreateSemaphore(NULL, 0, capacity,
		(mqName == NULL) ? NULL : hasItemsSemName);

}

// public queue functions

MQ_HANDLE MqCreate(DWORD elemSize, DWORD capacity) {
	MQ_HANDLE mq = (MQ_HANDLE) malloc(sizeof(MQ));
	
	MqInit(mq,
		(PMSG_QUEUE)malloc(sizeof(MSG_QUEUE) + elemSize * capacity),
		elemSize,
		capacity,
		NULL /* no name in the local version */);
	return mq;
}

VOID MqGet(MQ_HANDLE mq, PVOID msg) {
	WaitForSingleObject(mq->hasItems, INFINITE);
	QueueGet((PMSG_QUEUE)mq->queue, msg);
	ReleaseSemaphore(mq->hasSpace, 1, NULL);
}

VOID MqPut(MQ_HANDLE mq, PVOID msg) {
	WaitForSingleObject(mq->hasSpace, INFINITE);
	QueuePut((PMSG_QUEUE)mq->queue, msg);
	ReleaseSemaphore(mq->hasItems, 1, NULL);
}

VOID MqDestroy(MQ_HANDLE mq) {	
	CloseHandle(mq->hasItems);
	CloseHandle(mq->hasSpace);
	free(mq->queue);
	free(mq);
}


// end of local message queue functions

// Now are the versions that support interprocess communication

/*
	Creates a message queue given it's name, the size of their elements ("elemSize")
	and the queue size ("capacity"). The queue is on shared memory to support interprocess
	communication.
	
	Used by the server to create the queue where he receives the client commands
	and by the clients to create the response queues.
*/
SYSMQ_HANDLE SysMqCreate(PCSTR name, DWORD elemSize, DWORD capacity) {
	HANDLE map = NULL;
	SYSMQ_HANDLE mq = NULL;
	PVOID baseAddr = NULL;

	mq = (SYSMQ_HANDLE)malloc(sizeof(SYSMQ));
	if (mq == NULL) goto error;

	map = CreateFileMapping(
					INVALID_HANDLE_VALUE,
					NULL,
					PAGE_READWRITE,
					0,
					sizeof(MSG_QUEUE) + elemSize * capacity,
					name);
	if (map == NULL) goto error;

	baseAddr = MapViewOfFile(map, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	if (baseAddr == NULL) goto error;
	
	MqInit(&mq->base,
		(PMSG_QUEUE)baseAddr,
		elemSize,
		capacity,
		name);
	mq->map = map;
	return mq;
error:
	if (mq != NULL) free(mq);
	if (baseAddr != NULL) UnmapViewOfFile(baseAddr);
	if (map != NULL) CloseHandle(map);
	return NULL;
}

/*
   Retrieve an object representant of a previous created queue.
   Used by the clients to send commands to the server and by the 
   server to send the client response.
*/
SYSMQ_HANDLE SysMqOpen(PCSTR name) {
	HANDLE map = NULL;
	SYSMQ_HANDLE mq = NULL;
	PVOID baseAddr = NULL;

	mq = (SYSMQ_HANDLE)malloc(sizeof(SYSMQ));
	if (mq == NULL) goto error;
	map = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, name);
	if (map == NULL) goto error;
	baseAddr = MapViewOfFile(map, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (baseAddr == NULL) goto error;

	mq->base.queue = baseAddr;
	mq->map = map;
	CHAR hasSpaceSemName[128], hasItemsSemName[128];

	sprintf_s(hasSpaceSemName, "SemHasSpace_%s", name);
	sprintf_s(hasItemsSemName, "SemHsItems_%s", name);

	mq->base.hasItems = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, hasItemsSemName);
	mq->base.hasSpace = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, hasSpaceSemName);

	return mq;
error:
	if (mq != NULL) free(mq);
	if (baseAddr != NULL) UnmapViewOfFile(baseAddr);
	if (map != NULL) CloseHandle(map);
	return NULL;
	 
}

// Get a message from the queue
VOID SysMqGet(SYSMQ_HANDLE mq, PVOID msg) {
	MqGet(&mq->base, msg);
}

// Send a message to the queue
VOID SysMqPut(SYSMQ_HANDLE mq, PVOID msg) {
	MqPut(&mq->base, msg);
}

// Destroy a message Queue
VOID SysMqDestroy(SYSMQ_HANDLE mq) {
	CloseHandle(mq->base.hasItems);
	CloseHandle(mq->base.hasSpace);

	UnmapViewOfFile(mq->base.queue);
	CloseHandle(mq->map);
	free(mq);
}


// End of function use in interprocess communication