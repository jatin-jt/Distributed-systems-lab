#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <arpa/inet.h>
#include <stdio.h>

void convertToString(char buffer[1024], int f)
{
    int j = 0;
    int r = f;
    int digits = 0;
    while(r > 0)
    {
        r = r / 10;
        digits++;
    }
    j = digits - 1;
    buffer[digits] = '\0';

    while(j >= 0)
    {
        buffer[j--] = '0' +  (f % 10);
        f = f / 10;
    }
    return;
}

// converting time from string format to integer format

int convertToInteger(char buffer[1024])
{
    int ans = 0;
    int i = 0;
    while(buffer[i] != '\0')
    {
        ans = (buffer[i] - '0') + (ans * 10);
        ++i;
    }
    return ans;
}

int main(int argc, char * argv[])
{

    // input for port
    int PORT = atoi(argv[1]);
    // input for initial clock
    int clock = atoi(argv[2]);
    // input for increment
    int increment = atoi(argv[3]);

    PORT += 8000;
    // input number of processes
    int n_processes = atoi(argv[4]);
    // printing details:
    printf("CURRENT PORT %d\n", PORT);
    printf("Current clock : %d\n", clock);
    printf("Increment : %d\n", increment);
    printf("Total Number of Processes : %d\n", n_processes);

    // buffer for sending and receiving messages
    char buffer[1024];
    int server_id;

    // storing current address
    struct sockaddr_in current_address;
    current_address.sin_family = AF_INET;
    current_address.sin_addr.s_addr = INADDR_ANY;
    current_address.sin_port = htons(PORT);

    // defining server id
    server_id = socket(PF_INET, SOCK_DGRAM, 0);

    if(server_id < 0)
    {
        printf("Server Connection failed\n");
        exit(1);
    }


    // binding
    if( bind(server_id, (struct sockaddr *) &current_address, sizeof(current_address) ) < 0 )
    {
        printf("Bind Failed\n");
        exit(1);
    }


    // defining structure for receiver
    struct sockaddr_storage receiver_address;

    // defining destination address
    struct sockaddr_in destination_address;
    destination_address.sin_family = AF_INET;
    destination_address.sin_addr.s_addr = INADDR_ANY;

    // address length
    socklen_t address_len = sizeof(current_address
    ), from_temp_len = sizeof(receiver_address);


    while(1)
    {

        // incrementing clock on every iteration
        clock += increment;

        // printing current clock
        printf("Current Clock : %d\n", clock);

        char ch;
        printf("Do you want to send a message ");

        scanf("%c", &ch);

        if(ch == 'y')
        {
            // sending message to a process timestamped at current clock
            printf("Enter the destination process number ");
            int p;
            scanf("%d", &p);
            p += 8000;
            destination_address.sin_port = htons(p);

            // adding clock time to the buffer to be sent.
            convertToString(buffer, clock);

            sendto(server_id, (char * )buffer, 1024,0, (
                    struct sockaddr * )&destination_address, sizeof(destination_address));
            // for ignoring endline inputs
            scanf("%c", &ch);

        }
        else
        {
            printf("Waiting for message\n");

            // receiving message from another process
            int size = recvfrom(server_id, (char * ) buffer, 1024, 0, (struct sock_addr *) &receiver_address, &from_temp_len);


            printf("Received Message : %s from process number %d\n", buffer, ntohs(((struct
                    sockaddr_in*)&receiver_address)->sin_port)-8000);

            int receiver_clock = convertToInteger(buffer);

            if(receiver_clock > clock)
            {

             // updating clock because received message timestamp is greater than the current clock
                printf("Received message timestamp is greater than current clock\n");
                printf("Updating Clock \n");
                clock = receiver_clock + 1;
                printf("Updated Clock : %d\n", clock);
            }
            else
            {
                // no need of updating clock
                // received message timestamp is less than current clock
                printf("Received message timestamp is less than current clock\n");
                printf("No requirement of updating clock \n");
            }

            scanf("%c", &ch);

        }

    }
    return 0;
}   