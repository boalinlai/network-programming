/*


Author: Boa-Lin Lai

client can only receive full file on local server.
might be the issue with recv/send implentation and buffer size 

*/

// Client side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
  
int main(int argc, char const *argv[])
{
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    char cmd[4]; // include the NULL char
    int PORT;
    char IP[40];
    int MAXBUF = 1 << 25; 
    char* recv_buf = calloc((MAXBUF), sizeof(char));          
    char input_str[2000];
    
    if (argc < 3) {
       printf("Please use the format ./client \"\"IP\"\" \"PORT\"\n"); 
       return -1;
    }
    
    PORT = atoi(argv[2]);
    snprintf(IP, 40,"%s", argv[1]);


    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
  
    memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
      
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, IP, &serv_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
  
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    while (fgets(input_str, 2000 -1 , stdin)) {
    // trim the last char
        input_str[strlen(input_str)-1] = '\0';

        if (strlen(input_str) > 4) {
            memset(cmd, 0, 4 * sizeof(char));
            snprintf(cmd, 4,"%s", input_str);
        }

        if (strcmp(cmd, "GET") == 0) {
            send(sock , input_str , strlen(input_str) , 0);
            int byte_read = 1;
            memset(recv_buf, 0, MAXBUF * sizeof(char));
            byte_read = recv(sock, recv_buf, MAXBUF, 0);
            printf("%s", recv_buf);
            printf("\nEOF\n");
        } else {
            if (strlen(input_str) == 0) continue;
            send(sock , input_str , strlen(input_str) , 0 );
            printf("s_size:%d\n", (int) strlen(input_str));
            memset(buffer, 0, 1024 * sizeof(char));
            valread = recv(sock , buffer, 1024, 0);
            printf("r_size:%s\n", buffer);
        }
        memset(input_str, 0, 2000 * sizeof(char));
    }
    free(recv_buf);
    return 0;
}
