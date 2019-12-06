#pragma once
#define MAX_CONCURRENCY_LEVEL 3
#define MAX_LINE  512
#define MAX_LINES 128
#define MAX_RESULTS 1024

#define TOO_MANY_FILES_TO_PROCESS 1
#define TOO_MANY_LINES_WITH_STRING 2
#define OK 0

typedef struct _searchFileResult {
	CHAR path[MAX_PATH];
	int nLines;
	INT lines[MAX_LINES];
} SEARCH_FILE_RESULT, *PSEARCH_FILE_RESULT;

typedef struct _searchResult {
	INT errorCode;
	CHAR root[MAX_PATH];
	int totalResults;
	long workLeft;
	HANDLE eventt;
	HANDLE DoneAll;
	SEARCH_FILE_RESULT results[MAX_RESULTS];
} SEARCH_RESULT, *PSEARCH_RESULT;

typedef struct pThread_Arg {
	PCSTR toFind;
	int id;
	CHAR path[MAX_PATH];
	PSEARCH_RESULT res;
} THREAD_ARG, *PTHREAD_ARG;

typedef struct pThread_ArgServer {
	PCSTR toFind;
	int id;
	CHAR path[MAX_PATH];
	PSEARCH_RESULT res;
} THREAD_ARGSERVER, *PTHREAD_ARGSERVER;

VOID SearchFileDir(PSTR path, PCSTR toFind, PSEARCH_RESULT res);

BOOL DistributeWork(CHAR filepath[], PCSTR toFind, PSEARCH_RESULT res);

VOID Init(PSTR path, PCSTR toFind, PSEARCH_RESULT res);

DWORD __stdcall Start(LPVOID threadArg);



