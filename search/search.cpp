#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <stdio.h>
#include <assert.h>

#include "search.h"

// Mutex to handle multiple threads increasing value

static int counter = 1;
// utilitary functions

static INT readLine(FILE *f, CHAR line[], int capacity) {
	int i = 0, c;
	while (i < capacity - 1 && (c = fgetc(f)) != EOF && c != '\n')
		line[i++] = c;
	line[i] = 0;
	while (c != EOF && c != '\n') c = fgetc(f);
 	return (i != 0 || c != EOF) ? i : -1;
}

static BOOL strEndsWith(LPCSTR s, LPCSTR pattern) {
	int is = strlen(s) - 1, ip = strlen(pattern) - 1;
	
	while (is >= 0 && ip >= 0 && s[is] == pattern[ip]) { is--; ip--; }

	return ip == -1;
}

static BOOL isCodeFile(LPCSTR file) {
	return strEndsWith(file, ".c") || strEndsWith(file, ".cpp") ||
		strEndsWith(file, ".h") || strEndsWith(file, ".hpp");
}

/*
   if *fes is NULL force it to point to the next free FILEResult in res
   return FALSE if no free File Result exists on res
*/
static BOOL enforceFileResult(PSEARCH_RESULT res, PSEARCH_FILE_RESULT *fres) {
	if (*fres != NULL) return TRUE;
	if (res->totalResults == MAX_RESULTS) return FALSE;
	*fres = res->results + res->totalResults;
	res->totalResults++;
	(*fres)->nLines = 0;
	return TRUE;
}



// search a specific file
DWORD __stdcall searchText(LPVOID threadArg) {
	PTHREAD_ARG thread = (PTHREAD_ARG)threadArg;
	LPCSTR path = thread->path;
	LPCSTR toFind = thread->toFind;
	printf("Thread %d working, tDirectory of files to search for reproductions = \"%s\"\n", thread->id,thread->path);

	PSEARCH_RESULT res = thread->res;
	FILE *f = fopen(path, "r");
	if (f == NULL) return FALSE;
	CHAR line[MAX_LINE];
	int sz, line_number =1;

	PSEARCH_FILE_RESULT fileRes = NULL;
	int errorCode = OK;
	
	while ((sz = readLine(f, line, MAX_LINE)) != -1) {
		if (strstr(line, toFind) != NULL) {
			if (!enforceFileResult(res, &fileRes) ) {
				errorCode = TOO_MANY_FILES_TO_PROCESS;
				break;
			}
			else if (fileRes->nLines == MAX_LINES) {
				errorCode = TOO_MANY_LINES_WITH_STRING;
				break;
			}
			fileRes->lines[fileRes->nLines++] = line_number;
		}
		line_number++;
	}
	if (fileRes != NULL) strcpy_s(fileRes->path, MAX_PATH, path);
	res->errorCode = errorCode;
	InterlockedDecrement(&res->workLeft);
	SetEvent(res->eventt);
	if (res->workLeft == 0) SetEvent(res->DoneAll);
	return errorCode == OK;
}



/*
	Search the folder and corresponding sub-folders for 
	c/c++ source code where to find the string "toFind".

	fills a result with the names of the files that contains "toFind"
	and foreach one all the lines where the string was found.

*/
VOID SearchFileDir(PSTR path, PCSTR toFind,  PSEARCH_RESULT res) {

	CHAR buffer[MAX_PATH];		// auxiliary buffer

	//Remove eventual last slash on "path"
	int path_len = strlen(path);
	if (path_len == 0) return;

	CHAR c = path[path_len-1];
	if (c == '/' || c == '\\') path[path_len - 1] = 0;
	// the buffer is needed to define a match string that guarantees 
	// a priori selection for all files
	sprintf_s(buffer, "%s/%s", path, "*.*");

	WIN32_FIND_DATA fileData;
	HANDLE fileIt = FindFirstFile(buffer, &fileData);
	if (fileIt == NULL) return;

	// Process directory entries
	do {
		if ((fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
			// Not processing "." and ".." directories!
			if (fileData.cFileName[0] != '.') {
				sprintf_s(buffer, "%s/%s", path, fileData.cFileName);
				// Recursively process child directory
				SearchFileDir(buffer, toFind, res);
			}
		}
		else if (isCodeFile(fileData.cFileName)) {
			// Process file 
			CHAR filepath[MAX_PATH];
			sprintf_s(filepath, "%s/%s", path, fileData.cFileName);
			DistributeWork(filepath, toFind, res);
		}
	} while ( FindNextFile(fileIt, &fileData) == TRUE );

	FindClose(fileIt);

}


BOOL DistributeWork(CHAR filepath[], PCSTR toFind, PSEARCH_RESULT res)
{
	if (res->workLeft == MAX_CONCURRENCY_LEVEL) {
		printf("Waiting for one thread to finish\n");
		WaitForSingleObject(res->eventt, INFINITE);
	}
	InterlockedIncrement(&res->workLeft);

	PTHREAD_ARG arg = (PTHREAD_ARG)malloc(sizeof(THREAD_ARG));
	arg->toFind = toFind;
	arg->id = counter++;
	strcpy(arg->path, filepath);
	arg->res = res;

	// Add work to the thread pool
	if (!QueueUserWorkItem((LPTHREAD_START_ROUTINE)searchText, arg, NULL))
	{
		return FALSE;
	}
	return TRUE;
}

VOID Init(PSTR path, PCSTR toFind, PSEARCH_RESULT res) {
	res->workLeft = 0;
	res->eventt = CreateEvent(NULL, TRUE, FALSE, NULL);
	SearchFileDir(path, toFind, res);
	WaitForSingleObject(res->DoneAll, INFINITE);
}

DWORD __stdcall Start(LPVOID threadArg) {
	PTHREAD_ARG arg = (PTHREAD_ARG)malloc(sizeof(THREAD_ARG));
	arg->toFind = toFind;
	arg->id = counter++;
	strcpy(arg->path, filepath);

	long 
	res->workLeft = 0;
	res->eventt = CreateEvent(NULL, TRUE, FALSE, NULL);
	SearchFileDir(path, toFind, res);
	WaitForSingleObject(res->DoneAll, INFINITE);

	int startIndex = strlen(res->root);
	for (int i = 0; i < res->totalResults; ++i) {
		PSEARCH_FILE_RESULT fres = res->results + i;
		printf("~%s: ", fres->path + startIndex);
		for (int j = 0; j < fres->nLines; ++j) {
			printf("%d ", fres->lines[j]);
		}
		printf("\n");
	}
}
