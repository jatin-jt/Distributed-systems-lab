#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <math.h>
int main(int argc, char const *argv[]){
	// define variables to hold socket descriptor and other parameters
	int server_id, valread, PORT_SELF = atoi(argv[1]), n_processes = atoi(argv[2]);
	// array of processes
	int processes[n_processes];
	// add 8000 to all to make them valid ports
	PORT_SELF += 8000;
	for(int i=0;i<n_processes;i++){
		processes[i] = atoi(argv[3+i]);
		processes[i] += 8000;
	}
	// structure to hold address parameters of self, next and previous process
	struct sockaddr_in address, *from;
	struct sockaddr_storage from_temp, parent;
	socklen_t address_len = sizeof(address), from_temp_len = sizeof(from_temp), parent_len =
	sizeof(parent);
	// create socket
	server_id = socket(PF_INET, SOCK_DGRAM, 0);
	// socketet parameters of address structures for self and next process
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT_SELF);
	// bind socket descriptor to address
	if (bind(server_id, (struct sockaddr*)&address, sizeof(address))<0){
		printf("%s\n", "Bind failed");
		exit(0);
	}
	// store the pre required buffers
	char buffer_request[2048] = {0}, buffer_commit[2048] = {0}, buffer_abort[2048] = {0},
	buffer_receive[2048] = {0};
	char buffer_global_commit[2048] = {0}, buffer_global_abort[2048] = {0};
	snprintf(buffer_request, sizeof("Vote request") + 1, "%s", "Vote request");
	buffer_request[sizeof("Vote request")] = '\0';
	snprintf(buffer_commit, sizeof("Vote commit") + 1, "%s", "Vote commit");
	buffer_commit[sizeof("Vote commit")] = '\0';
	snprintf(buffer_abort, sizeof("Vote abort") + 1, "%s", "Vote abort");
	buffer_abort[sizeof("Vote abort")] = '\0';
	snprintf(buffer_global_commit, sizeof("Global commit") + 1, "%s", "Global commit");
	buffer_global_commit[sizeof("Global commit")] = '\0';
	snprintf(buffer_global_abort, sizeof("Global abort") + 1, "%s", "Global abort");
	buffer_global_abort[sizeof("Global abort")] = '\0';
	// repeat any no. of times
	while(1){
		printf("\nPress enter to initiate commit voting ");
		getchar();
		int commit = 1;
		// send vote message to all processes
		for(int i=0;i<n_processes;i++){
			// get next process' port
			int PORT_NEXT = processes[i];
			struct sockaddr_in next;
			next.sin_family = AF_INET;
			next.sin_addr.s_addr = INADDR_ANY;
			next.sin_port = htons(PORT_NEXT);
			printf("\nSending \"%s\" message to process: %d\n", buffer_request, PORT_NEXT-8000);
			// send the message
			sendto(server_id, buffer_request, strlen(buffer_request) + 1, 0, (struct sockaddr*) &next, sizeof(next));
			// wait for reply
			valread = recvfrom(server_id, buffer_receive, 2048, 0, (struct sockaddr*) &from_temp, &from_temp_len);
			printf("Message received \"%s\" from process: %d\n", buffer_receive, ntohs(((struct sockaddr_in*)&from_temp)->sin_port)-8000);
			// if commit message is received
			if(!strcmp(buffer_receive, buffer_commit)){
				printf("\n");
				continue;
			}
			// if abort message is received
			else if(!strcmp(buffer_receive, buffer_abort)){
				commit = 0;
			}
			// else a wrong message is received
			else{
				printf("Wrong message received\n");
			}
		}
		// send global decision message to all processes
		for(int i=0;i<n_processes;i++){
			// get next process' port no.
			int PORT_NEXT = processes[i];
			struct sockaddr_in next;
			next.sin_family = AF_INET;
			next.sin_addr.s_addr = INADDR_ANY;
			next.sin_port = htons(PORT_NEXT);
			// if global decision is of commiting
			if(commit==1){
				printf("\nSending \"%s\" message to process: %d\n",
					buffer_global_commit, PORT_NEXT-8000);
				sendto(server_id, buffer_global_commit, strlen(buffer_global_commit) +
					1, 0, (struct sockaddr*) &next, sizeof(next));
			}
			// if global decision is of aborting
			else{
				printf("\nSending \"%s\" message to process: %d\n", buffer_global_abort, PORT_NEXT-8000);
				sendto(server_id, buffer_global_abort, strlen(buffer_global_abort) + 1, 0, (struct sockaddr*) &next, sizeof(next));
			}
			// wait for the acknowledgement
			valread = recvfrom(server_id, buffer_receive, 2048, 0, (struct sockaddr*) &from_temp, &from_temp_len);
			printf("Message received \"%s\" from process: %d\n", buffer_receive, ntohs(((struct sockaddr_in*)&from_temp)->sin_port)-8000);
		}
		printf("\n---------------------------------------------------------------\n");
	}
	return 0;
}