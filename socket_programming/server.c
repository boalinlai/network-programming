/*
 

Author Boa-Lin Lai. 
this file only functional under  Linux environment
REF:
http://www.geeksforgeeks.org/
http://www.tldp.org/LDP/LGNET/91/misc/tranter/server.c.txt


*/

//Handle multiple socket connections with select and fd_set on Linux 
#include <stdio.h> 
#include <string.h>   //strlen 
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h>   //close 
#include <arpa/inet.h>    //close 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros 
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <fcntl.h>
#define TRUE   1 
#define FALSE  0 
//#define PORT 9876
    
int send_file_content(char* file_name, int sd);
int cat_command_handler(char* recv_buffer, int sock_fd);

int main(int argc , char *argv[])  
{  
    int opt = TRUE;  
    int master_socket , addrlen , new_socket , client_socket[30] , 
          max_clients = 30 , activity, i , valread , sd;  
    int max_sd;  
    int PORT;
    struct sockaddr_in address;  
        
    char buffer[1025];  //data buffer of 1K 
        
    //set of socket descriptors 
    fd_set readfds;  

    if (argc < 2) {
       printf("Please use the format of ./server \"PORT\"\n");
       return -1;
    }

        
    PORT = atoi(argv[1]);
    
    //initialise all client_socket[] to 0 so not checked 
    for (i = 0; i < max_clients; i++)  
    {  
        client_socket[i] = 0;  
    }  
        
    //create a master socket 
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)  
    {  
        perror("socket failed");  
        exit(EXIT_FAILURE);  
    }  
    
    //set master socket to allow multiple connections , 
    //this is just a good habit, it will work without this 
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, 
          sizeof(opt)) < 0 )  
    {  
        perror("setsockopt");  
        exit(EXIT_FAILURE);  
    }  
    
    //type of socket created 
    address.sin_family = AF_INET;  
    address.sin_addr.s_addr = INADDR_ANY;  
    address.sin_port = htons( PORT );  
        
    //bind the socket to localhost port 8888 
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)  
    {  
        perror("bind failed");  
        exit(EXIT_FAILURE);  
    }  
    printf("Listener on port %d \n", PORT);  
        
    //try to specify maximum of 3 pending connections for the master socket 
    if (listen(master_socket, 3) < 0)  
    {  
        perror("listen");  
        exit(EXIT_FAILURE);  
    }  
        
    //accept the incoming connection 
    addrlen = sizeof(address);  
    puts("Waiting for connections ...");  

    while(TRUE)  
    {  
        //clear the socket set 
        FD_ZERO(&readfds);  
    
        //add master socket to set 
        FD_SET(master_socket, &readfds);  
        max_sd = master_socket;  
            
        //add child sockets to set 
        for ( i = 0 ; i < max_clients ; i++)  
        {  
            //socket descriptor 
            sd = client_socket[i];  
                
            //if valid socket descriptor then add to read list 
            if(sd > 0)  
                FD_SET( sd , &readfds);  
                
            //highest file descriptor number, need it for the select function 
            if(sd > max_sd)  
                max_sd = sd;  
        }  
    
        //wait for an activity on one of the sockets , timeout is NULL , 
        //so wait indefinitely 
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);  
      
        if ((activity < 0) && (errno!=EINTR))  
        {  
            printf("select error");  
        }  
            
        //If something happened on the master socket , 
        //then its an incoming connection 
        if (FD_ISSET(master_socket, &readfds))  
        {  
            if ((new_socket = accept(master_socket, 
                    (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)  
            {  
                perror("accept");  
                exit(EXIT_FAILURE);  
            }  
            
            //inform user of socket number - used in send and receive commands 
          
            //add new socket to array of sockets 
            for (i = 0; i < max_clients; i++)  
            {  
                //if position is empty 
                if( client_socket[i] == 0 )  
                {  
                    client_socket[i] = new_socket;  
                    printf("Adding to list of sockets as %d\n" , i);  
                        
                    break;  
                }  
            }  
        }  
            
        //else its some IO operation on some other socket
        for (i = 0; i < max_clients; i++) {  
            sd = client_socket[i];  
                
            if (FD_ISSET( sd , &readfds)) {  
                //Check if it was for closing , and also read the 
                //incoming message 
                memset(buffer, 0, 1025 * sizeof(char));
                if ((valread = read( sd , buffer, 1024)) == 0)  
                {  
                    //Somebody disconnected , get his details and print 
                    getpeername(sd , (struct sockaddr*)&address , \
                        (socklen_t*)&addrlen);  
                    printf("Host disconnected , ip %s , port %d \n" , 
                          inet_ntoa(address.sin_addr) , ntohs(address.sin_port));  
                        
                    //Close the socket and mark as 0 in list for reuse 
                    close( sd );  
                    client_socket[i] = 0;  
                }  
                    
                //Echo back the message that came in 
                else {  
                    //set the string terminating NULL byte on the end 
                    //of the data read 
                    char number_of_char[5];
                    memset(number_of_char, 0, 5 * sizeof(char));

                    buffer[valread] = '\0';  

                    printf("Message received:\"%s\"\n",buffer);


                    if (cat_command_handler(buffer, sd) == -1) {
                        snprintf(number_of_char, 5, "%d", (int)(strlen(buffer)));
                        number_of_char[strlen(buffer)] = '\0';
                        send(sd , number_of_char, strlen(number_of_char) , 0 );  
                    }
                }  
            }  
        }  
    }  
        
    return 0;  
}  
/*

cat funciton handler for the format  sanity check.

*/
int cat_command_handler(char* recv_buffer, int sock_fd) {

    char* cmd_buffer = calloc(1025, sizeof(char));   
    char* tokenPtr;
    char* cmd = calloc(4, sizeof(char));   // include the NULL char
    char* file_name = calloc((256), sizeof(char));   
    int ret = snprintf(cmd_buffer, 1025, "%s", recv_buffer);
    

    tokenPtr = strtok(cmd_buffer, " ");
    snprintf(cmd, 4, "%s", tokenPtr);
    if (strcmp(cmd, "GET") == 0) {
        tokenPtr = strtok(NULL, " ");
        snprintf(file_name, 256, "%s", tokenPtr);
        printf("cat:%s\n",file_name); // should print file name
        if (send_file_content(file_name, sock_fd) == 1) {
            printf("content sent!\n");
        } else {
            printf("format error!\n");
            send(sock_fd , "\0", 1 , 0);  // for client not to listen 
        }
    } else {
        ret = -1;
    }
    free(cmd_buffer);
    free(cmd);
    free(file_name);
    return ret;
}


/*
Function that send the file to sock descriptor
*/

int send_file_content(char* filename, int sock_fd) {

    off_t offset = 0; 
    int rc;
    struct stat stat_buf; 
    int fd = open(filename, O_RDONLY);
    

    if (fd == -1) {
      fprintf(stderr, "unable to open '%s': %s\n", filename, strerror(errno));
      return -1;
      //exit(1);
    }

    /* get the size of the file to be sent */
    fstat(fd, &stat_buf);
    /* copy file using sendfile */
    

    for (size_t size_to_send = stat_buf.st_size; size_to_send > 0;) {
      ssize_t sent = sendfile(sock_fd, fd, &offset, size_to_send);
      if (sent <= 0) break;
      
      offset += sent;
      size_to_send -= sent;  


    }
/*
    rc = sendfile (sock_fd, fd, &offset, stat_buf.st_size);
    if (rc == -1) {
      fprintf(stderr, "error from sendfile: %s\n", strerror(errno));
      exit(1);
    }
    if (rc != stat_buf.st_size) {
      fprintf(stderr, "incomplete transfer from sendfile: %d of %d bytes\n",
              rc,
              (int)stat_buf.st_size);
      exit(1);
    }

*/
    printf("\nEOF\n");
    close(fd);
    return 1;
}
