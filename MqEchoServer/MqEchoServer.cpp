// MqEchoServerTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h>
#include <stdio.h>
#include <stdlib.h> 
#include "EchoServer.h"
#include "../search/search.h"
int counter = 1;

void startChildProcess(STARTUPINFO * si, PROCESS_INFORMATION * pi, char * args);

int main(int argc, char * argv[]) //C:\Users\David\Desktop\so-p2\SO_trab\Debug\MqEchoServer.exe
{
	if (argc < 1) {
		while (true) {
			STARTUPINFOA si;
			PROCESS_INFORMATION pi;
			ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);
			ZeroMemory(&pi, sizeof(pi));
			startChildProcess(&si, &pi, NULL);
			DWORD exitCode = 0;
			while (GetExitCodeProcess(pi.hThread, &exitCode) != FALSE) {}
			printf("Restarting echo server!\n");
		}
	}
	else {
		SYSMQ_HANDLE srv_mq = SysMqCreate("echo", sizeof(ECHO_CMD), 128);
		printf("Starting echo server!\n\n");
		while (true) {
			ECHO_CMD cmd;
			SysMqGet(srv_mq, &cmd);
			if (cmd.clientId == 0) break;
			printf("Cmd received: %s\n", cmd.msg);
			// prepare the response

			CHAR clientMqName[128];
			sprintf_s(clientMqName, "%d", cmd.clientId);
			SYSMQ_HANDLE client_mq = SysMqOpen(clientMqName);

			char * msg = "To search the word: %s in directory: %s from IdClient: %d";//cmd.msg;

			SEARCH_RESULT res;
			PTHREAD_ARGSERVER arg = (PTHREAD_ARGSERVER)malloc(sizeof(THREAD_ARGSERVER));
			arg->id = counter++;
			arg->path = 
			arg->toFind =
			arg->res = &res;

			if(!QueueUserWorkItem(Start,arg,NULL)) return;
			//reset
			cmd.clientId = 0;
			SysMqPut(client_mq, &cmd);

		//	SysMqDestroy(client_mq);

	//	SysMqDestroy(srv_mq);
		}

	}
		return 0;
	}


void startChildProcess(STARTUPINFO* si, PROCESS_INFORMATION * pi,char * args) {
	if (!CreateProcess("C:\\Users\\David\\Desktop\\so-p2\\SO_trab\\x64\\Debug\\MqEchoServer.exe",
		LPSTR("C:\\Users\\David\\Desktop\\so-p2\\SO_trab\\x64\\Debug\\MqEchoServer.exe server"),
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		si,
		pi)
		)
	{
		printf("CreateProcess failed (%d).\n", GetLastError());
		return;
	}
	else {
		printf("CreateProcess was successful.\n");
		return;
	}
}

