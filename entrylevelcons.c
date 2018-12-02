//Coordinator:
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
// function to process incoming message
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
	int server_id, valread, PORT_SELF = atoi(argv[1]), resource = atoi(argv[2]),
	process_no_update = -1;
	PORT_SELF += 8000;
	// structure to hold address parameters of self, next and previous process
	struct sockaddr_in address, *from, next;
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
	char buffer_request[2048] = {0}, buffer_allow[2048] = {0}, buffer_receive[2048] = {0};
	snprintf(buffer_request, sizeof("Update request") + 1, "%s", "Update request");
	buffer_request[sizeof("Update request")] = '\0';
	snprintf(buffer_allow, sizeof("Update allowed") + 1, "%s", "Update allowed");
	buffer_allow[sizeof("Update allowed")] = '\0';
	// repeat any no. of times
	while(1){
		// ask whether to get new value
		printf("Synchronize with latest value? ");
		char ch;
		scanf("%c", &ch);
		// if we want ot get newest value of resource
		if(ch=='y'){
			// if no update has been made
			if(process_no_update==-1){
				printf("Already latest value\n");
			}
			// ask the last that updated the value to supply newest value
			else{
				// get the port of that process
				int PORT_NEXT = process_no_update + 8000;
				next.sin_family = AF_INET;
				next.sin_addr.s_addr = INADDR_ANY;
				next.sin_port = htons(PORT_NEXT);
				// send the request message
				sendto(server_id, buffer_request, strlen(buffer_request) + 1, 0, (struct sockaddr*) &next, sizeof(next));
				// read the new value and parse it
				valread = recvfrom(server_id, buffer_receive, 2048, 0, (struct sockaddr*) &from_temp, &from_temp_len);
				int length_length = buffer_receive[0] - '0';
				int new_value = process_message(length_length, buffer_receive, 1);
				// update the value
				resource = new_value;
				printf("Resource now is %d\n", resource);
			}
		}
		// if we want to redirect incoming request to last process who updated the value
		else{
			// receive message from process
			valread = recvfrom(server_id, buffer_receive, 2048, 0, (struct sockaddr*) &from_temp, &from_temp_len);
			// if message is request message
			if(!strcmp(buffer_receive, buffer_request)){
				// if no update has been made, allow that process to update
				if(process_no_update==-1){
					// send OK
					sendto(server_id, buffer_allow, strlen(buffer_allow) + 1, 0, (struct sockaddr*) &from_temp, sizeof(from_temp));
				}
				// if some update has been made
				else{
					// send the process no. of the process that last updated the resource
					memset(buffer_receive, 0, sizeof(buffer_receive));
					int temp = process_no_update, digits = 0;
					for(int i=0;temp>0;i++){
						digits++;
						temp = temp/10;
					}
					// form the message
					char digits_char[100], process_no_update_char[100];
					sprintf(digits_char, "%d", digits);
					sprintf(process_no_update_char, "%d", process_no_update);
					strcat(buffer_receive, digits_char);
					strcat(buffer_receive, process_no_update_char);
					// send to destination
					sendto(server_id, buffer_receive, strlen(buffer_receive) + 1, 0, (struct sockaddr*) &from_temp, sizeof(from_temp));
				}
				// update the process no. who last updated the value
				process_no_update = ntohs(((struct sockaddr_in*)&from_temp)->sin_port) - 8000;
			}
		}
		getchar();
		printf("\n");
	}
	return 0;
}
