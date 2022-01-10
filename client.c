#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void usage(int argc, char **argv) {
	printf("usage: %s <server IP> <server port> start\n", argv[0]);
	printf("example: %s 127.0.0.1 51511 start\n", argv[0]);
	exit(EXIT_FAILURE);
}

#define BUFSZ 1024

int main(int argc, char **argv) {
	int sockfd, port_num, n;
    int enable = 1;
    char send_to_server[BUFSZ], receive_from_server[BUFSZ];
    socklen_t len;
    struct sockaddr_in servaddr[4];
    struct timeval recvtimeout;
    recvtimeout.tv_sec = 1;
    recvtimeout.tv_usec = 0;

    if (argc < 4 || strcmp("start", argv[3])) {
		usage(argc, argv);
	}

    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &recvtimeout, sizeof(recvtimeout)) < 0) {
        perror("setsockopt error");
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt error");
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0) {
        perror("setsockopt error");
    }
    
    port_num = htons(atoi(argv[2]));
    for(int server_num=0; server_num<NUM_OF_SERVERS; server_num++){
        memset(&servaddr[server_num], 0, sizeof(servaddr[server_num]));
        
        // Filling server information
        servaddr[server_num].sin_family = AF_INET;
        servaddr[server_num].sin_port = port_num;
        servaddr[server_num].sin_addr.s_addr = INADDR_ANY;
        port_num ++;
    }

    int is_start_message = 1;

    while(1){
        int is_one_message = 0;

        memset(send_to_server, 0, sizeof(send_to_server));
        if(is_start_message){
            strcpy(send_to_server, argv[3]);
            is_start_message = 0;
        }
        else fgets(send_to_server, BUFSZ, stdin);

        recvtimeout.tv_sec = 1;
        recvtimeout.tv_usec = 0;
        printf("SEND TO SERVER %s", send_to_server);
        n = sendto(sockfd, (const char *)send_to_server, sizeof(send_to_server), 0, (const struct sockaddr *) &servaddr[0], sizeof(servaddr[0]));
        printf("N: %d\n", n);

        if(!strcmp("getdefenders\n", send_to_server) || 
            !strcmp("stats\n", send_to_server) || 
            strstr(send_to_server, "shot") != NULL)
                is_one_message = 1;

        printf("IS ONE MESSAGE: %d\n", is_one_message);

        for(int server_num=0; server_num<NUM_OF_SERVERS; server_num++){
            memset(receive_from_server, 0, sizeof(receive_from_server));
            while((n = recvfrom(sockfd, (char *)receive_from_server, sizeof(receive_from_server), 0, (struct sockaddr *) &servaddr[server_num], &len)) < 0){
                    memset(&servaddr[0], 0, sizeof(servaddr[0]));
                    
                    // Filling server information
                    servaddr[0].sin_family = AF_INET;
                    servaddr[0].sin_port = htons(atoi(argv[2]));;
                    servaddr[0].sin_addr.s_addr = INADDR_ANY;

                    sendto(sockfd, (const char *)send_to_server, sizeof(send_to_server), 0, (const struct sockaddr *) &servaddr[0], sizeof(servaddr[0]));
                printf("\nMessage was not received\n");
                // break;
            }
            
            receive_from_server[n] = '\0';
            printf("%s\n", receive_from_server);
            
            if(strstr(receive_from_server, "gameover") != NULL) break;
            if(is_one_message) break;
        }

        

        if(!strcmp("quit\n", send_to_server)) break;
    }

    close(sockfd);

	exit(EXIT_SUCCESS);
}