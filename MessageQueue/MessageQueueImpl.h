#pragma once

typedef struct {
	DWORD Capacity;
	DWORD ElemSize;
	DWORD GetIdx;
	DWORD PutIdx;
	BYTE  Msgs[1];
} MSG_QUEUE, *PMSG_QUEUE;


