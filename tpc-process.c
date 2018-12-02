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
	int server_id, valread, PORT_SELF = atoi(argv[1]);
	PORT_SELF += 8000;
	// structure to hold address parameters of self, next and previous process
	struct sockaddr_in address, *from;
	struct sockaddr_storage from_temp, parent;
	socklen_t address_len = sizeof(address), from_temp_len = sizeof(from_temp), parent_len =
	sizeof(parent);
	// create socket
	server_id = socket(PF_INET, SOCK_DGRAM, 0);
	// set parameters of address structures for self and next process
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
	char buffer_global_commit[2048] = {0}, buffer_global_abort[2048] = {0},
	buffer_commit_ack[2048] = {0}, buffer_abort_ack[2048] = {0};
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
	snprintf(buffer_commit_ack, sizeof("Commit acknowledgement") + 1, "%s", "Commit acknowledgement");
	buffer_commit_ack[sizeof("Commit acknowledgement")] = '\0';
	snprintf(buffer_abort_ack, sizeof("Abort acknowledgement") + 1, "%s", "Abort acknowledgement");
	buffer_abort_ack[sizeof("Abort acknowledgement")] = '\0';
	// repeat any no. of times
	while(1){
		// wait for the message
		printf("\nWaiting for message\n\n");
		valread = recvfrom(server_id, buffer_receive, 2048, 0, (struct sockaddr*) &from_temp, &from_temp_len);
		printf("Message received \"%s\" from process: %d\n", buffer_receive, ntohs(((struct sockaddr_in*)&from_temp)->sin_port)-8000);
		// decide whether to commit or abort
		printf("Enter commit or abort? ");
		char response[100];
		scanf("%s", response);
		// if decision is of committing, then send that message to coordinator
		if(!strcmp(response, "commit")){
			sendto(server_id, buffer_commit, strlen(buffer_commit) + 1, 0, (struct sockaddr*) &from_temp, sizeof(from_temp));
		}
		// if decision is of aborting, then send that message to coordinator
		else if(!strcmp(response, "abort")){
			sendto(server_id, buffer_abort, strlen(buffer_abort) + 1, 0, (struct sockaddr*) &from_temp, sizeof(from_temp));
		}
		// send wrong message to coordinator
		else{
			printf("Wrong\n");
			continue;
		}
		// wait for global decision
		printf("Waiting for reply\n");
		valread = recvfrom(server_id, buffer_receive, 2048, 0, (struct sockaddr*) &from_temp, &from_temp_len);
		printf("\nMessage received \"%s\" from process: %d\n", buffer_receive, ntohs(((struct sockaddr_in*)&from_temp)->sin_port)-8000);
		// send the commit acknowledgement
		if(!strcmp(buffer_receive, buffer_global_commit)){
			sendto(server_id, buffer_commit_ack, strlen(buffer_commit_ack) + 1, 0, (struct sockaddr*) &from_temp, sizeof(from_temp));
		}
		// send the abort acknowledgement
		else if(!strcmp(buffer_receive, buffer_global_abort)){
			sendto(server_id, buffer_abort_ack, strlen(buffer_abort_ack) + 1, 0, (struct sockaddr*) &from_temp, sizeof(from_temp));
		}
		// send wrong message to coordinator
		else{
			printf("Wrong message received\n");
			sendto(server_id, "Wrong", strlen("Wrong") + 1, 0, (struct sockaddr*) &from_temp, sizeof(from_temp));
		}
	}
	return 0;
}