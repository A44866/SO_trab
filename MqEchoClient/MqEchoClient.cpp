// MqClientTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>

#include <stdio.h>
#include <stdlib.h> 
#include "../MqEchoServer/EchoServer.h"


void fillMessage(PECHO_CMD cmd,const char * argv[]) {
	cmd->clientId = GetCurrentThreadId();
	sprintf_s(cmd->msg, "To search the word: %s in directory: %s from IdClient: %d", argv[1], argv[0], cmd->clientId);
}

int main()
{
	CHAR clientName[128];
	sprintf_s(clientName, "%d", GetCurrentThreadId());

	SYSMQ_HANDLE client_mq = SysMqCreate(clientName, sizeof(ECHO_CMD), 1);
	SYSMQ_HANDLE srv_mq = SysMqOpen("echo");

	printf("Starting echo client!\n\n");
	
		ECHO_CMD cmd;
		
		const char * argv[] = {"C:\\Users\\David\\Desktop\\testSO","test"};
		fillMessage(&cmd, argv);
		SysMqPut(srv_mq, &cmd);
	 
		// get the response

		SysMqGet(client_mq, &cmd);

		printf("Echo received: %s\n", cmd.msg);



	printf("End echo client, press return to continue...");
	getchar();

	SysMqDestroy(srv_mq);
	SysMqDestroy(client_mq);
	return 0;
}


 