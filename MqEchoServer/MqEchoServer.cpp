// MqEchoServerTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h>
#include <stdio.h>
#include <stdlib.h> 
#include "EchoServer.h"

void startChildProcess(STARTUPINFO si, PROCESS_INFORMATION pi, char * arg);

int main(int argc, char * argv[])
{
	while (true) {
		SYSMQ_HANDLE srv_mq = SysMqCreate("echo", sizeof(ECHO_CMD), 128);
		printf("Starting echo server!\n\n");
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));
		startChildProcess(si, pi, argv[1]);
		DWORD id = pi.dwProcessId;
		HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, false, id);
		DWORD exitCode = 0;
		while (GetExitCodeProcess(handle, &exitCode) != FALSE) {}
	}
	return 0;
}	
		



		ECHO_CMD cmd;
		SysMqGet(srv_mq, &cmd);

		if (cmd.clientId == 0) 
			break;

		printf("Cmd received: %s\n", cmd.msg);
		// prepare the response
		 
		CHAR clientMqName[128];
		sprintf_s(clientMqName, "%d", cmd.clientId);
		SYSMQ_HANDLE client_mq = SysMqOpen(clientMqName);

		cmd.clientId = 0;
		SysMqPut(client_mq, &cmd);

		SysMqDestroy(client_mq);
	}

	SysMqDestroy(srv_mq);
	return 0;
}

void startChildProcess(STARTUPINFO si, PROCESS_INFORMATION pi,char * arg) {
	if (!CreateProcessA(NULL,
		arg,
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&si,
		&pi)
		)
	{
		printf("CreateProcess failed (%d).\n", GetLastError());
		return;
	}
}

