/////////////////////////////////////////////////////////////////
//
// CCISEL 
// 2007-2011
//
// UThread library:
//   User threads supporting cooperative multithreading.
//
// Authors:
//   Carlos Martins, Jo�o Trindade, Duarte Nunes, Jorge Martins
// 

#pragma once

#include <Windows.h>
#include "UThread.h"
#include "List.h"
#include <assert.h>
//
// The data structure representing the layout of a thread's execution context
// when saved in the stack.
//



#ifdef _WIN64
//RBX, RBP, RDI, RSI, R12, R13, R14, and R15 
typedef struct _UTHREAD_CONTEXT_64 {
	ULONGLONG R15;
	ULONGLONG R14;
	ULONGLONG R13;
	ULONGLONG R12;
	ULONGLONG RSI;
	ULONGLONG RDI;
	ULONGLONG RBX;
	ULONGLONG RBP;
	VOID(*RetAddr)();
} UTHREAD_CONTEXT_64, *PUTHREAD_CONTEXT_64;

#define PUTHREAD_CONTEXT PUTHREAD_CONTEXT_64
#define UTHREAD_CONTEXT UTHREAD_CONTEXT_64
#else
typedef struct _UTHREAD_CONTEXT_32 {
	ULONG EDI;
	ULONG ESI;
	ULONG EBX;
	ULONG EBP;
	VOID(*RetAddr)();
} UTHREAD_CONTEXT_32, *PUTHREAD_CONTEXT_32;

#define PUTHREAD_CONTEXT PUTHREAD_CONTEXT_32
#define UTHREAD_CONTEXT UTHREAD_CONTEXT_32
#endif

//
// The descriptor of a user thread, containing an intrusive link (through which
// the thread is linked in the ready queue), the thread's starting function and
// argument, the memory block used as the thread's stack and a pointer to the
// saved execution context.
//
typedef struct _UTHREAD {
	PUTHREAD_CONTEXT ThreadContext;
	LIST_ENTRY       Link;
	CHAR *			 Name; //Name of the Thread
	CHAR * 			 State; //The State of the Thread
	LONG		     Result; // the thread result
	BOOL			 Terminated; // If true, the thread is terminated
								 // and the sole purpose is
								 // to give his result to another
								 // thread
	BOOL			 EnableTermination;  //Determines if a thread can be finished
	BOOL			 ToTerminate;		 //If the RunningThread encounters this variable equal to TRUE, it should Terminate
	UT_FUNCTION      Function;
	UT_ARGUMENT      Argument;
	PUCHAR           Stack;
} UTHREAD, *PUTHREAD;



typedef struct info {
	CHAR *			 Name; //Name of the Thread
	CHAR * 			 State; //The State of the Thread
	DWORD			 StackUsage;
}*UT_INFO_THREAD;

UTHREAD_API
BOOL UtDump(HANDLE tHandle, UT_INFO_THREAD info);

//
//The Minimum Size Of a Stack
#define MINIMUM_STACK_SIZE (16 * 1024)
//
// The fixed stack size of a user thread.
//32KB
#define STACK_SIZE (8 * 4096)

#define BLOCKED "BLOCKED"
#define RUNNING "RUNNING"
#define READY "READY"



