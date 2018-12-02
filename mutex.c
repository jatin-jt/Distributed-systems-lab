#include <stdio.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#include <unistd.h>


typedef struct Node{
    struct sockaddr_in add;
    struct Node* next;
} Node;

Node * createNode(struct sockaddr_in add){
    Node *temp = malloc(sizeof(Node));
    temp->add=add;
    temp->next=NULL;
    return temp;

}
void enqueue(Node *front[],Node *rear[],int index,struct sockaddr_in add){
    Node *n = createNode(add);
    if(!rear[index]){
        front[index] =rear[index] = n;
    }
    else{
        rear[index]->next = n;
        rear[index] = rear[index]->next;
    }

}

void dequeue(Node *front[],Node *rear[],int index){
    if(front[index]==rear[index]){
        free(front[index]);
        front[index]=rear[index]=NULL;
    }
    else{
        Node *temp = front[index];
        front[index]=front[index]->next;
        free(temp);
    }

}

int main(int argc,char *argv[]){
    int udpSocket,n,no_of_clients=0;
    printf("Enter Number of resources: ");
    scanf("%d",&n);
    getchar();
    printf("Server Ready\n");
    bool resource[n];
    for(int i=0;i<n;i++){
        resource[i]=false;
    }

    Node *front[n],*rear[n];
    for (int i = 0; i < n; ++i) {
        front[i]=NULL;
        rear[i]=NULL;
    }
    int PORT = atoi(argv[1]);
    char buffer[1024];
    char message[2];
    struct sockaddr_in serverAddr,*temp,other;
    struct sockaddr_storage peer_addr;
    socklen_t peer_addrlen;

    //Create UDP socket
    udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

    //Configure settings in address struct
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    //memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    //Bind socket with address struct
    bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    //Initialize size variable to be used later on
    peer_addrlen = sizeof peer_addr;


    while(1){
        /* Try to receive any incoming UDP datagram. Address and port of
          requesting client will be stored on peer_addr variable */
        recvfrom(udpSocket,buffer,1024,0,(struct sockaddr *)&peer_addr, &peer_addrlen);
        temp = (struct sockaddr *)&peer_addr;
        printf("Request received from Client: %d\n",temp->sin_port);
        char operation = buffer[0];
        char resource_no  = buffer[1];
        if(operation=='2'){
            message[0]='3';
            no_of_clients++;
        }

        else if(operation=='0'){
            if(!resource[resource_no-'1']){
                message[0]='0';
                resource[resource_no-'1']=true;
            } else{
                message[0]='1';
                enqueue(front,rear,resource_no-'1',*temp);

            }
        }
        else if(operation=='1'){
            message[0]='2';
           // resource[resource_no-'1'] =false;
            if(front[resource_no-'1']) {
                other = front[resource_no - '1']->add;
                dequeue(front, rear, resource_no - '1');
                sendto(udpSocket,"0\n",strlen("0\n"),0,(struct sockaddr* ) &other,peer_addrlen);
            }
            else{
                resource[resource_no-'1']=false;
            }

        }

        size_t message_size = strlen(message);
        message[1]='\0';

        /*Send message to client, using peer_addr as the address*/
        sendto(udpSocket,message,message_size,0,(struct sockaddr *)&peer_addr,peer_addrlen);
        printf("Message Sent to client: %s\n\n",message);
        if (no_of_clients==5){
            break;
        }
    }
    close(udpSocket);

    return 0;
}