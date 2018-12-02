//Process:
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
// function to parse the contents of message
int process_message(int length_length, char buffer_receive[2048], int index){
	int temp = length_length - 1, thing = 0;
	for(int i=0;i<length_length;i++){
		thing = thing + pow(10, temp)*(buffer_receive[index+i] - '0');
		temp--;
	}
	return thing;
}
int main(int argc, char const *argv[]){
	// define variables to hold socket descriptor and other parameters
	int server_id, valread, PORT_SELF = atoi(argv[1]), PORT_COORDINATOR = atoi(argv[2]),
	resource = atoi(argv[3]);
	PORT_SELF += 8000;
	PORT_COORDINATOR += 8000;
	// structure to hold address parameters of self, next and previous process
	struct sockaddr_in address, *from, next, coordinator;
	struct sockaddr_storage from_temp, parent;
	socklen_t address_len = sizeof(address), from_temp_len = sizeof(from_temp), parent_len =
	sizeof(parent);
	// create socket
	server_id = socket(PF_INET, SOCK_DGRAM, 0);
	// set parameters of address structures for self and next process
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT_SELF);
	coordinator.sin_family = AF_INET;
	coordinator.sin_addr.s_addr = INADDR_ANY;
	coordinator.sin_port = htons(PORT_COORDINATOR);
	// bind socket descriptor to address
	if (bind(server_id, (struct sockaddr*)&address, sizeof(address))<0){
		printf("%s\n", "Bind failed");
		exit(0);
	}
	// store the pre required buffers
	char buffer_request[2048] = {0}, buffer_allow[2048] = {0}, buffer_receive[2048] = {0};
	snprintf(buffer_request, sizeof("Update request") + 1, "%s", "Update request");
	buffer_request[sizeof("Update request")] = '\0';
	snprintf(buffer_allow, sizeof("Update allowed") + 1, "%s", "Update allowed");
	buffer_allow[sizeof("Update allowed")] = '\0';
	// repeat any no. of times
	while(1){
		// ask user whether to update data
		printf("Update data? ");
		char ch;
		scanf("%c", &ch);
		// if yes, then ask the coordinator for last process who updated the resource
		if(ch=='y'){
			// send and receive the message
			sendto(server_id, buffer_request, strlen(buffer_request) + 1, 0, (struct sockaddr*) &coordinator, sizeof(coordinator));
			valread = recvfrom(server_id, buffer_receive, 2048, 0, (struct sockaddr*) &from_temp, &from_temp_len);
			// if coordinator did not reply with allow update
			if(strcmp(buffer_receive, buffer_allow)){
				// get the process no. of who last updated the resource
				int length_length = buffer_receive[0] - '0';
				int process_no_update = process_message(length_length, buffer_receive, 1);
				// if this is not the process that last updated the resource
				if(process_no_update!=PORT_SELF-8000){
					// get the port and send message to that process to supply with latest resource
					int PORT_NEXT = process_no_update + 8000;
					next.sin_family = AF_INET;
					next.sin_addr.s_addr = INADDR_ANY;
					next.sin_port = htons(PORT_NEXT);
					// send and receive the process
					sendto(server_id, buffer_request, strlen(buffer_request) + 1, 0, (struct sockaddr*) &next, sizeof(next));
					valread = recvfrom(server_id, buffer_receive, 2048, 0, (struct sockaddr*) &from_temp, &from_temp_len);
					length_length = buffer_receive[0] - '0';
					int new_resource = process_message(length_length, buffer_receive, 1);
					// update the resource
					resource = new_resource;
				}
			}
			// ask the user for increment in resource
			printf("Enter increment ");
			int i;
			scanf("%d", &i);
			resource += i;
			printf("New value is %d\n", resource);
		}
		// if this process wants to supply latest value of resource to other process
		else{
			// receive the message
			valread = recvfrom(server_id, buffer_receive, 2048, 0, (struct sockaddr*) &from_temp, &from_temp_len);
			int temp = resource, digits = 0;
			for(int i=0;temp>0;i++){
				digits++;
				temp = temp/10;
			}
			// form the message containing the new resource value
			memset(buffer_receive, 0, sizeof(buffer_receive));
			char digits_char[100], resource_char[100];
			sprintf(digits_char, "%d", digits);
			sprintf(resource_char, "%d", resource);
			strcat(buffer_receive, digits_char);
			strcat(buffer_receive, resource_char);
			// send the message
			sendto(server_id, buffer_receive, strlen(buffer_receive) + 1, 0, (struct sockaddr*) &from_temp, sizeof(from_temp));
		}
		getchar();
		printf("\n");
	}
	return 0;
}