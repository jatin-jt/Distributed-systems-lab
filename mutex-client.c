#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int PORT = atoi(argv[1]);
    int clientSocket;
    size_t nBytes;
    char op, res, c;
    char response;
    char input[3],output[3];
    char buffer[1024];
    bool finish = false;
    struct sockaddr_in serverAddr;
    socklen_t addr_size;

    //Create UDP socket
    clientSocket = socket(PF_INET, SOCK_DGRAM, 0);

    //Configure settings in address struct
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    // memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    //Initialize size variable to be used later on
    addr_size = sizeof serverAddr;

    while (1) {
        printf("Enter operation: ");
        scanf("%c", &op);
        printf("Enter Resource No: ");
        scanf(" %c", &res);
        getchar();
        input[0] = op;
        input[1] = res;
        nBytes = strlen(input);

        //Send message to server
        sendto(clientSocket, input, nBytes, 0, (struct sockaddr *) &serverAddr, addr_size);


        //Receive message from server
        recvfrom(clientSocket, buffer, 1024, 0, NULL, NULL);
        response = buffer[0];
        
        if(response=='3'){
            printf("Successfully Disconnected");
            break;
        }
        
        if (response == '1') {
            printf("Waiting for resource!\n");
            recvfrom(clientSocket, buffer, 1024, 0, NULL, NULL);
        }

        printf("Resource Acquired!\nEntering Critical Section!!\n");
        printf("Release Resource?(Enter any character): ");
        scanf("%c", &c);
        getchar();
        output[0]='1';
        output[1] =res;
        sendto(clientSocket, output, strlen(output), 0,(struct sockadd *) &serverAddr, addr_size);
        recvfrom(clientSocket, buffer, 1024, 0, NULL, NULL);
        if(buffer[0]=='2'){
            printf("Successfully released resource!\n");
        }

    }
    close(clientSocket);
    return 0;
}