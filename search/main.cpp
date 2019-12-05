#include <Windows.h>
#include <stdio.h>
#include <string.h>
#include "search.h"

VOID ShowResults(PSEARCH_RESULT res) {
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

DWORD main(DWORD argc, PCHAR argv[]) {

	if (argc < 3) {
		printf("Use: %s <folder path> <text to find>", argv[0]);
		exit(0);
	}
	SEARCH_RESULT res;


	strcpy_s(res.root, argv[1]);
	res.totalResults = 0;

	init(argv[1], argv[2], &res);
 
	ShowResults(&res);
	return 0;
}
