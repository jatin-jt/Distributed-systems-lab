#include <stdio.h> 
#include <string.h> 
#include <errno.h> 
#include <time.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <arpa/inet.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
 

int main()
{
    time_t clock;
    int udpSocket, nBytes;
    char buffer[1024];
    struct sockaddr_in serverAddr, clientAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size, client_addr_size;
    int i;

    udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(7891);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

    bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    addr_size = sizeof serverStorage;

    while(1)
    {
        printf("\nHi,I am a running server.Some Client hit me\n"); 
        nBytes = recvfrom(udpSocket,buffer,1024,0,(struct sockaddr *)&serverStorage, &addr_size);
        printf("Received a client message: %s Sending date-time\n",buffer); 

        clock = time(NULL);
        snprintf(buffer, sizeof(buffer), "%.24s\r\n", ctime(&clock));

        sendto(udpSocket,buffer,sizeof(buffer),0,(struct sockaddr *)&serverStorage,addr_size);
    }

  return 0;
}