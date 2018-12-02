#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
int main(int argc, char const *argv[])
{
	// define variables to hold socket descriptor and get the process nos. from command line
	int server_id, valread, PORT_SELF = atoi(argv[1]), PORT_NEXT = atoi(argv[2]);
	// add 8000 to them to make them valid port nos.
	PORT_SELF += 8000;
	PORT_NEXT += 8000;
	// check if initial token has been given
	char *token = (char*) malloc(strlen(argv[3]));
	strcat(token, argv[3]);
	int token_;
	if(!strcmp(token, "y"))
		token_ = 1;
	else
		token_ = 0;
	// structure to hold address parameters of self, next and previous process
	struct sockaddr_in address, *from, next;
	struct sockaddr_storage from_temp;
	socklen_t address_len = sizeof(address), from_temp_len = sizeof(from_temp);
	// buffer to hold messages
	char buffer_receive[2048] = {0}, buffer_send[2048] = {0}, buffer_exit[2048] = {0};
	// create socket
	server_id = socket(PF_INET, SOCK_DGRAM, 0);
	// set parameters of address structures
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT_SELF);
	next.sin_family = AF_INET;
	next.sin_addr.s_addr = INADDR_ANY;
	next.sin_port = htons(PORT_NEXT);
	// bind socket descriptor to address
	if (bind(server_id, (struct sockaddr*)&address, sizeof(address))<0)
	{
		printf("%s\n", "Bind failed");
		exit(0);
	}
	// store special messages to send
	snprintf(buffer_send, sizeof("Token received") + 1, "%s", "Token received");
	buffer_send[sizeof("Token received") + 1] = '\0';
	snprintf(buffer_exit, sizeof("Exit token received") + 1, "%s", "Exit token received");
	buffer_send[sizeof("Exit token received") + 1] = '\0';// circulate token any no. of times
	while(1)
	{
		// if initial token is not provided, wait for token
		if(token_==0)
		{
			// wait till message from previous node comes
			printf("Waiting for token\n");
			valread = recvfrom(server_id, buffer_receive, 2048, 0, (struct sockaddr*)&from_temp, &from_temp_len);
			// if exit token is received, then exit and send pass exit token to next node
			if(strcmp(buffer_receive, buffer_exit)==0)
			{
				printf("%s\n", buffer_receive);
				printf("Sending exit token to next node and exiting\n");
				sendto(server_id, buffer_exit, sizeof(buffer_exit) + 1, 0, (struct sockaddr*) &next, sizeof(next));
				exit(0);
			}
			// else acknowledgement to previous node
			sendto(server_id, buffer_send, sizeof(buffer_send) + 1, 0, (struct sockaddr*)&from_temp, sizeof(from_temp));
			printf("%s\n", buffer_receive);
		}
		// now token is in possession
		token_ = 1;
		// ask whether to enter critical section
		printf("Enter critical section? ");
		char choice;
		scanf("%s", &choice);
		// if we want to enter critical section
		if(choice=='y')
		{
			// open file and write to file the process no.
			printf("In critical section\n");
			FILE *file;
			file = fopen("a.txt", "a");
			char buffer[100] = {0};
			sprintf(buffer, "Critical section accessed by process no. %d\n", PORT_SELF-8000);
			fprintf(file, buffer);
			fclose(file);
		}
		// if we want to exit system, send exit token to next node
		else if(choice=='e')
		{
			printf("Sending exit token to next node\n");
			sendto(server_id, buffer_exit, sizeof(buffer_exit) + 1, 0, (struct sockaddr*)&next, sizeof(next));
			token_ = 0;
			continue;
		}
		// we do not want to enter critical section, so pass token to next node
		printf("Passing token to next node\n");
		sendto(server_id, buffer_send, sizeof(buffer_send) + 1, 0, (struct sockaddr*) &next,sizeof(next));
		// wait for acknowledgement from next node
		valread = recvfrom(server_id, buffer_receive, 2048, 0, (struct sockaddr*) &from_temp,&from_temp_len);
		printf("%s by next process\n", buffer_receive);
		// leave the token
		token_ = 0;
	}
	return 0;
}