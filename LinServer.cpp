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
#include "config.h"

//#include <stdio.h>
//#include <stdlib.h>
//#include <unistd.h>
//#include <errno.h>
#include <netdb.h>
//#include <sys/types.h>
//#include <netinet/in.h>
//#include <sys/socket.h>

//#include <arpa/inet.h>


std::string game_server_port = "";

void* SocketHandler(void*);
//std::string process_message(std::string message);
//std::vector<std::string> Splitter(std::string del, std::string str);

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

}

int main(int argc, char *argv[]){

    if(argc <= 1) {
        std::cout << "Usage: " << argv[0] << " <filename>" << std::endl;
        exit(1);
    }
    char *pFilename = argv[1];
    Config config = Config(pFilename);
    int host_port = Config::LookupInt("Port");
    game_server_port = Config::LookupString("GameServerPort");
    std::cout << "Listening on port: " << host_port << "\n";
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

    addr_size = sizeof(sockaddr_in);

    while(true){
        printf("waiting for a connection\n");
        csock = (int*)malloc(sizeof(int));
        if((*csock = accept( hsock, (sockaddr*)&sadr, &addr_size))!= -1) {
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
    #define MAXDATASIZE 1024 // max number of bytes we can get at once
    char buffer[MAXDATASIZE];
    int buffer_len = 1024;
    int bytecount;
    std::string motd;
    std::string reply_buffer;
    std::string readpipe;
    fd_set connections;
    fd_set readConnections;

//Code to connect to unity game server.
    int sockfd, numbytes;
    char buf[1024];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    FD_ZERO(&connections); //Clear the connection set
    FD_ZERO(&readConnections); //Clear the connection set

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    dg_net networkHandler;

    if ((rv = getaddrinfo("localhost", game_server_port.c_str(), &hints, &servinfo)) != 0) {
//    if ((rv = getaddrinfo("10.1.10.101", port.c_str(), &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        goto FINISH;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        goto FINISH;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

    networkHandler.SetFD(sockfd);

    motd = networkHandler.GetMOTD();
    if((bytecount = send(*csock, motd.c_str(), strlen(motd.c_str()), 0))== -1){
        fprintf(stderr, "Error sending data %d\n", errno);
        goto FINISH;
    }

/*
* MEAT AND TATOS HERE
*/
//Add the primary connection to the select connections
FD_SET(*csock, &connections);
FD_SET(sockfd, &connections);

    while(true) {
        memset(buffer, 0, buffer_len);
        readConnections = connections;
        if(select(sockfd+1, &readConnections, NULL, NULL, NULL) == -1) {
            perror("select");
        }

        if(FD_ISSET(*csock, &readConnections)) {
            std::cout << "Recieving data from client thread\n";
            if((bytecount = recv(*csock, buffer, buffer_len, 0)) == -1){
                fprintf(stderr, "Error receiving data %d\n", errno);
                goto FINISH;
            }
            printf("Received client bytes %d\nReceived string \"%s\"\n", bytecount, buffer);
            if(bytecount == 0) {
                std::cout << "Client disconected. Log them out.\n";
                networkHandler.Logout();
                goto FINISH;
            }


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
//                std::cout << "Reply buffer empty\n";
            }
            memset(buffer, 0, buffer_len);
        }

        if(FD_ISSET(sockfd, &readConnections)) {
            std::cout << "Data on server sock ready\n";
            if((bytecount = recv(sockfd, buffer, buffer_len, 0)) == -1){
                fprintf(stderr, "Error receiving data %d\n", errno);
                goto FINISH;
            }
            printf("Received from server bytes %d\nReceived string \"%s\"\n", bytecount, buffer);
            if(bytecount == 0) {
                std::cout << "Lost server connection\n";
                goto FINISH;
            }
            if((bytecount = send(*csock, buffer, strlen(buffer), 0))== -1){
                std::cout << "Could not send data back to client\n";
                fprintf(stderr, "Error sending data %d\n", errno);
                goto FINISH;
            }
        }
    }
FINISH:
    std::cout << "Terminating thread\n";
    close(sockfd);
    free(csock);
    std::cout << "Cleared freeing csock and returning 0\n";
    return 0;
}
