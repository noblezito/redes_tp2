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
    char send_to_server[BUFSZ], receive_from_server[BUFSZ];
    socklen_t len;
    struct sockaddr_storage servaddr[4];
    struct timeval recvtimeout;
    recvtimeout.tv_sec = 0;
    recvtimeout.tv_usec = 10000;

    if (argc < 4 || strcmp("start", argv[3])) {
        printf("%d\n", argc);
		usage(argc, argv);
	}
    
    port_num = htons(atoi(argv[2]));
    for(int server_num=0; server_num<NUM_OF_SERVERS; server_num++){
        memset(&servaddr[server_num], 0, sizeof(servaddr[server_num]));
        
        if (0 != addrparse(argv[1], port_num, &servaddr[server_num])){
            printf("test\n");
            usage(argc, argv);
        }
        port_num ++;
    }

    if ( (sockfd = socket(servaddr[0].ss_family, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &recvtimeout, sizeof(recvtimeout)) < 0) {
        perror("setsockopt error");
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

        recvtimeout.tv_sec = 0;
        recvtimeout.tv_usec = 10000;
        n = sendto(sockfd, (const char *)send_to_server, sizeof(send_to_server), MSG_WAITALL, (const struct sockaddr *) &servaddr[0], sizeof(servaddr[0]));

        if(!strcmp("quit\n", send_to_server)) break;

        if(!strcmp("getdefenders\n", send_to_server) || 
            !strcmp("stats\n", send_to_server) || 
            strstr(send_to_server, "shot") != NULL)
                is_one_message = 1;

        for(int server_num=0; server_num<NUM_OF_SERVERS; server_num++){
            memset(receive_from_server, 0, sizeof(receive_from_server));
            while((n = recvfrom(sockfd, (char *)receive_from_server, sizeof(receive_from_server), 0, (struct sockaddr *) &servaddr[server_num], &len)) < 0){
                    memset(&servaddr[0], 0, sizeof(servaddr[0]));
                    
                    // Filling server information
                    printf("TESTE\n");
                    if(strcmp(receive_from_server, "")) break;
                    if (0 != addrparse(argv[1], atoi(argv[2]), &servaddr[server_num])) usage(argc, argv);
                    sendto(sockfd, (const char *)send_to_server, sizeof(send_to_server), MSG_WAITALL, (const struct sockaddr *) &servaddr[0], sizeof(servaddr[0]));
            }
            
            receive_from_server[n] = '\0';
            printf("%s\n", receive_from_server);
            
            if(strstr(receive_from_server, "gameover") != NULL) break;
            if(is_one_message) break;
        }
    }

    close(sockfd);

	exit(EXIT_SUCCESS);
}