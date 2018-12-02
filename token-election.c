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
#define print(x) printf("%s\n", x);


int process_message(int length_length, char buffer_receive[2048], int index){
	int temp = length_length - 1, thing = 0;
	for(int i=0;i<length_length;i++){
		thing = thing + pow(10, temp)*(buffer_receive[index+i] - '0');
		temp--;
	}
	return thing;
}

// function to print contents of buffer
void unspool(char buffer_receive[2048], int PORT_SELF){
	printf("\nBuffer received: %s\n\n", buffer_receive);
	int type = buffer_receive[0] - '0', length_length = buffer_receive[1] - '0', self_present = 0;
	int n_process = process_message(length_length, buffer_receive, 2), max_process_no = 0;
	if(type==1){
		printf("Type: %d, Process: %d\n", type, n_process);
		return;
	}
	printf("Type: %d, No. of processes: %d\nProcesses:\n", type, n_process);
	int processes[2048] = {0}, d = 0, index = 2 + length_length;
	for(int i=0;i<n_process;i++){
		int length_process_no = buffer_receive[index] - '0';
		int process_no = process_message(length_process_no, buffer_receive, index+1);
		processes[d] = process_no;
		d++;
		index = index + length_process_no+1;
		if(process_no==(PORT_SELF-8000))
			self_present = 1;
		if(process_no>max_process_no)
			max_process_no = process_no;
	}
	for(int i=0;i<n_process;i++){
		printf("%d\n", processes[i]);
	}
	printf("\n");
}

int main(int argc, char const *argv[]){

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
	char buffer_receive[20480] = {0};//, buffer_send[2048] = {0}, buffer_exit[2048] = {0};

	// create socket
	server_id = socket(PF_INET, SOCK_DGRAM, 0);

	// set parameters of address structures for self and next process
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT_SELF);
	next.sin_family = AF_INET;
	next.sin_addr.s_addr = INADDR_ANY;
	next.sin_port = htons(PORT_NEXT);

	// bind socket descriptor to address
	if (bind(server_id, (struct sockaddr*)&address, sizeof(address))<0){
		printf("%s\n", "Bind failed");
		exit(0);
	}

	
	

	// repeat any no. of times until coordinator is found
	while(1){

		// if this process is not the election message initiator
		if(token_==0){

			// wait for election or coordinator message
			printf("Waiting for election message\n");
			valread = recvfrom(server_id, buffer_receive, 2048, 0, (struct sockaddr*) &from_temp, &from_temp_len);

            // print the buffer and its contents parsed
			unspool(buffer_receive, PORT_SELF);

			// determine the type of message, whether election or coordinator
			int type = buffer_receive[0] - '0', length_length = buffer_receive[1] - '0', self_present = 0;

			// if it is coordinator message
			if(type==1){

				// print coordinator found and determine it by parsing the message
				printf("Coordinator found: ");
				int process_no_coordinator = process_message(length_length, buffer_receive, 2);
				printf("%d\n", process_no_coordinator);
				printf("Press enter to send message ");
				char c;
				c = getchar();

				// send the coordinator message and exit
				sendto(server_id, buffer_receive, sizeof(buffer_receive) + 1, 0, (struct sockaddr*) &next, sizeof(next));
				exit(0);
			}

			// if it is election message
			else{

				// parse the message to retreive the processes contained in the message
				int n_process = process_message(length_length, buffer_receive, 2), max_process_no = 0;
				int processes[2048] = {0}, d = 0, index = 2 + length_length;

				// store the process id's in an array and find maximum process id
				for(int i=0;i<n_process;i++){

					// determine the process id
					int length_process_no = buffer_receive[index] - '0';
					int process_no = process_message(length_process_no, buffer_receive, index+1);
					processes[d] = process_no;
					d++;
					index = index + length_process_no + 1;

					// determine if message contains self id, and find maximum id
					if(process_no==(PORT_SELF-8000))
						self_present = 1;
					if(process_no>max_process_no)
						max_process_no = process_no;
				}

				// if message does not contain self id
				if(self_present==0){

					// append the self id in message, and change message contents to increase no. of processes it contains
					int temp = n_process, digits1 = 0;
					for(int i=0;temp>0;i++){
						digits1++;
						temp = temp/10;
					}
					n_process++;
					temp = n_process;
					int digits2 = 0;
					for(int i=0;temp>0;i++){
						digits2++;
						temp = temp/10;
					}
					int digits = 0;
					temp = PORT_SELF-8000;
					for(int i=0;temp>0;i++){
						digits++;
						temp = temp/10;
					}

					// form the message to send
					char digits_char[2], process_no_char[100];
					digits_char[0] = digits + '0';
					digits_char[1] = '\0';
					sprintf(process_no_char, "%d", PORT_SELF-8000);
					if(digits1!=digits2)
						length_length++;
					char length_length_char = length_length + '0', n_process_char[digits2];
					sprintf(n_process_char, "%d", n_process);
					char buffer_temp[20480] = {0};
					snprintf(buffer_temp, sizeof(buffer_temp), "%s", buffer_receive+2+length_length);
					memset(buffer_receive, 0, sizeof(buffer_receive));
					buffer_receive[0] = '0';
					buffer_receive[1] = length_length_char;
					strcat(buffer_receive, n_process_char);
					strcat(buffer_receive, buffer_temp);
					strcat(buffer_receive, digits_char);
					strcat(buffer_receive, process_no_char);
					printf("Press enter to send message ");
					char c;
					c = getchar();

					// send the newly formd message to next process
					sendto(server_id, buffer_receive, sizeof(buffer_receive) + 1, 0, (struct sockaddr*) &next, sizeof(next));
				}

				// if message contains self id, that is it has been circulated once
				else{

					// clear the buffer, as coordinator will be process with highest id
					memset(buffer_receive, 0, sizeof(buffer_receive));
					buffer_receive[0] = '1';
					int digits = 0, temp = max_process_no;
					for(int i=0;temp>0;i++){
						digits++;
						temp = temp/10;
					}


					// write the highest process id into buffer

					buffer_receive[1] = digits + '0';
					char process_no_char[2000];
					sprintf(process_no_char, "%d", max_process_no);
					snprintf(buffer_receive+2, sizeof(process_no_char) + 1, "%s", process_no_char);
					buffer_receive[sizeof(process_no_char)] = '\0';
					printf("Coordinator found: %d\n", max_process_no);
					printf("Press enter to send message ");
					char c;
					c = getchar();

					// send the coordinator message to next process and exit
					sendto(server_id, buffer_receive, sizeof(buffer_receive) + 1, 0, (struct sockaddr*) &next, sizeof(next));
					exit(0);
				}
			}
		}
		
		
		
		
		// if this process is election message initiator
		else{

			// clear any message contents, and put the self id in message
			memset(buffer_receive, 0, sizeof(buffer_receive));
			buffer_receive[0] = '0';
			buffer_receive[1] = '1';
			buffer_receive[2] = '1';
			int temp = PORT_SELF - 8000, digits = 0;
			for(int i=0;temp>0;i++){
				temp = temp/10;
				digits++;
			}

			// fomr the message to send
			buffer_receive[3] = digits + '0';
			char process_no_char[100];
			sprintf(process_no_char, "%d", PORT_SELF-8000);
			strcat(buffer_receive, process_no_char);
			printf("Press enter to send message ");
			char c;
			c = getchar();

			// send the message to next process, and set message initiator to false
			sendto(server_id, buffer_receive, sizeof(buffer_receive) + 1, 0, (struct sockaddr*) &next, sizeof(next));
			token_ = 0;
		}

	}

	return 0;
}