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
	printf("usage: %s <server IP> <server port>\n", argv[0]);
	printf("example: %s 127.0.0.1 51511\n", argv[0]);
	exit(EXIT_FAILURE);
}

#define BUFSZ 1024

int main(int argc, char **argv) {
	int sockfd, port_num, n;
    char send_to_server[BUFSZ], receive_from_server[BUFSZ];
    socklen_t len;
    struct sockaddr_in servaddr[4];
    struct timeval recvtimeout;
    recvtimeout.tv_sec = 0;
    recvtimeout.tv_usec = 100000;


    if (argc < 3) {
		usage(argc, argv);
	}

    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &recvtimeout, sizeof(recvtimeout)) < 0) {
        perror("setsockopt error");
    }
    
    port_num = htons(atoi(argv[2]));
    for(int server_num=0; server_num<NUM_OF_SERVERS; server_num++){
        memset(&servaddr[server_num], 0, sizeof(servaddr));
        
        // Filling server information
        servaddr[server_num].sin_family = AF_INET;
        servaddr[server_num].sin_port = port_num;
        servaddr[server_num].sin_addr.s_addr = INADDR_ANY;
        port_num ++;
    }

	memset(send_to_server, 0, sizeof(send_to_server));
    sendto(sockfd, "initialize servers", sizeof("initialize servers"), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));

    while(1){
        int is_one_message = 0;
        int message_was_not_received_on_server = 1;

        do{
            memset(receive_from_server, 0, sizeof(receive_from_server));

            if(!strcmp("getdefenders\n", send_to_server) || !strcmp("stats\n", send_to_server) || strstr(send_to_server, "shot") != NULL) is_one_message = 1;

            for(int server_num=0; server_num<NUM_OF_SERVERS; server_num++){
                n = recvfrom(sockfd, (char *)receive_from_server, BUFSZ, MSG_WAITALL, (struct sockaddr *) &servaddr[server_num], &len);
                if(n<0){
                    printf("Message was not received\n");
                    message_was_not_received_on_server = 1;
                    break;
                }
                else{
                    receive_from_server[n] = '\0';
                    printf("%s\n", receive_from_server);
                    message_was_not_received_on_server = 0;
                }

                if(is_one_message) break;
            }
        }while(message_was_not_received_on_server);

        memset(send_to_server, 0, sizeof(send_to_server));
        fgets(send_to_server, BUFSZ-1, stdin);
        n = sendto(sockfd, (const char *)send_to_server, sizeof(send_to_server), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
        printf("N: %d\n", n);

        if(!strcmp("quit\n", send_to_server)) break;
    }

    close(sockfd);

	exit(EXIT_SUCCESS);
}