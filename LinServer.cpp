#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <vector>
#include "dg_net.h"

void* SocketHandler(void*);
std::string process_message(std::string message);
std::vector<std::string> Splitter(std::string del, std::string str);

int main(int argv, char** argc){
    int host_port= 1101;
    struct sockaddr_in my_addr;
    int hsock;
    int * p_int ;
    int err;
    socklen_t addr_size = 0;
    int* csock;
    sockaddr_in sadr;
    pthread_t thread_id=0;


    hsock = socket(AF_INET, SOCK_STREAM, 0);
    if(hsock == -1){
        printf("Error initializing socket %d\n", errno);
        goto FINISH;
    }

    p_int = (int*)malloc(sizeof(int));
    *p_int = 1;

    if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
        (setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
        printf("Error setting options %d\n", errno);
        free(p_int);
        goto FINISH;
    }
    free(p_int);

    my_addr.sin_family = AF_INET ;
    my_addr.sin_port = htons(host_port);

    memset(&(my_addr.sin_zero), 0, 8);
    my_addr.sin_addr.s_addr = INADDR_ANY ;

    if( bind( hsock, (sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
        fprintf(stderr,"Error binding to socket, make sure nothing else is listening on this port %d\n",errno);
        goto FINISH;
    }
    if(listen( hsock, 10) == -1 ){
        fprintf(stderr, "Error listening %d\n",errno);
        goto FINISH;
    }

    //Now lets do the server stuff

    addr_size = sizeof(sockaddr_in);

    while(true){
        printf("waiting for a connection\n");
        csock = (int*)malloc(sizeof(int));
        if((*csock = accept( hsock, (sockaddr*)&sadr, &addr_size))!= -1){
            printf("---------------------\nReceived connection from %s\n",inet_ntoa(sadr.sin_addr));
            pthread_create(&thread_id,0,&SocketHandler, (void*)csock );
            pthread_detach(thread_id);
        }
        else{
            fprintf(stderr, "Error accepting %d\n", errno);
        }
    }

FINISH:
std::cout << "womp womp womp\n";
;
}

void* SocketHandler(void* lp){
    int *csock = (int*)lp;
    char buffer[1024];
    int buffer_len = 1024;
    int bytecount;
    std::string motd;
    std::string reply_buffer;
    dg_net networkHandler;


    //Sending welcome message of the day
    motd = networkHandler.GetMOTD();
    if((bytecount = send(*csock, motd.c_str(), strlen(motd.c_str()), 0))== -1){
        fprintf(stderr, "Error sending data %d\n", errno);
        goto FINISH;
    }

    while(true) {
        memset(buffer, 0, buffer_len);
        if((bytecount = recv(*csock, buffer, buffer_len, 0)) == -1){
            fprintf(stderr, "Error receiving data %d\n", errno);
            goto FINISH;
        }
        if(bytecount == 0) {
            std::cout << "BOOM\n";
            goto FINISH;
        }

        printf("Received bytes %d\nReceived string \"%s\"\n", bytecount, buffer);

        if(strlen(buffer) > 0) {
            reply_buffer = networkHandler.process_message(buffer);
        }

        if(reply_buffer != "") {
            std::cout << "Reply buffer not blank. Sending return\n";
            if((bytecount = send(*csock, reply_buffer.c_str(), strlen(reply_buffer.c_str()), 0))== -1){
                std::cout << "bork bork bork\n";
                fprintf(stderr, "Error sending data %d\n", errno);
                goto FINISH;
            }
            printf("Sent bytes %d\n", bytecount);
            if(reply_buffer == "FOLD|true") {
                std::cout << "Recieved fold command. Terminating\n";
                goto FINISH;
            }
        } else {
            std::cout << "Reply buffer empty\n";
        }
        //strcat(buffer, " Welcome to the Dark-Gate server.\n");
    }

FINISH:
    std::cout << "Terminating thread\n";
    free(csock);
    return 0;
}

