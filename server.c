#include "common.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int last_used_id = 0;
int num_of_pokemons_killed = 0;
int num_of_winner_pokemons = 0;

void usage(int argc, char **argv) {
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

void generate_defense_pokemons(struct pokemon_defense defense_pokemons[NUM_OF_DEFENSE_POKEMONS]){
    time_t t;
    srand((unsigned) time(&t));
    
    for(int i=0; i<NUM_OF_DEFENSE_POKEMONS; i++){
        int x_pos = rand() % 5;
        int y_pos = (rand() % 4) + 1;

        defense_pokemons[i].x_pos = x_pos;
        defense_pokemons[i].y_pos = y_pos;
        defense_pokemons[i].has_attacked = 0;
    }
}

void initialize_battle_fields(int battle_fields[NUM_OF_SERVERS][NUM_OF_STEPS]){
    for(int i=0; i<NUM_OF_SERVERS; i++){
        for(int j=0; j<NUM_OF_STEPS; j++){
            battle_fields[i][j] = 0;
        }
    }
}

void initialize_attack_pokemons(struct pokemon_attack attack_pokemons[MAX_NUM_OF_ATTACK_POKEMONS]){
    for(int i=0; i<MAX_NUM_OF_ATTACK_POKEMONS; i++){
        attack_pokemons[i].id = 0;
        attack_pokemons[i].name = (char *)malloc(sizeof(char)*6);
        attack_pokemons[i].life = 0;
    }
}

void create_new_attack_pokemon(struct pokemon_attack attack_pokemons[MAX_NUM_OF_ATTACK_POKEMONS], char *name){
    for(int i=0; i<MAX_NUM_OF_ATTACK_POKEMONS; i++){
        if(attack_pokemons[i].id == 0){
            attack_pokemons[i].id = ++last_used_id;
            strcpy(attack_pokemons[i].name, name);
            if(!strcmp("Mewtwo", name)) attack_pokemons[i].life = 3;
            if(!strcmp("Lugia", name)) attack_pokemons[i].life = 2;
            if(!strcmp("Zubat", name)) attack_pokemons[i].life = 1;
            return;
        }
    }
}

void generate_random_attack_pokemons(struct pokemon_attack attack_pokemons[MAX_NUM_OF_ATTACK_POKEMONS], 
                                      int battle_field[NUM_OF_SERVERS][NUM_OF_STEPS], int spawn_chance){
    time_t t;
    srand((unsigned) time(&t));

    for(int i=0; i<NUM_OF_SERVERS; i++){
        if(battle_field[i][0] == 0){
            if((rand() % 100) < spawn_chance){
                for(int n=0; n<MAX_NUM_OF_ATTACK_POKEMONS; n++){
                    if(attack_pokemons[n].id == 0){
                        char* name = (char *)malloc(sizeof(char)*6);
                        int random_pokemon = rand() % 3;
                        switch (random_pokemon)
                        {
                        case 0:
                            strcpy(name, "Mewtwo");
                            break;
                        case 1:
                            strcpy(name, "Lugia");
                            break;
                        case 2:
                            strcpy(name, "Zubat");
                            break;
                        default:
                            strcpy(name, "Zubat");
                            break;
                        }
                        create_new_attack_pokemon(attack_pokemons, name);
                        free(name);
                        battle_field[i][0] = last_used_id;
                        break;
                    }
                    
                }
            }
        }
    }
}

void send_status_to_client(struct pokemon_defense defense_pokemons[NUM_OF_DEFENSE_POKEMONS], 
                           struct pokemon_attack attack_pokemons[MAX_NUM_OF_ATTACK_POKEMONS],
                           struct sockaddr_storage cliaddr,
                           int battle_field[NUM_OF_STEPS], int server_num, int turn, int sockfd
                          ) {

    char* status_message = (char *)malloc(BUFSZ/NUM_OF_STEPS);
    char* base_message_line = (char *) malloc(sizeof(char)*8);
    char* turn_message_line = (char *) malloc(sizeof(char)*8);
    char* fixed_location_message_line = (char *) malloc(sizeof(char)*17);
    char* id_to_string = (char *) malloc(sizeof(char)*7);
    char* life_to_string = (char *) malloc(sizeof(char)*5);

    snprintf(base_message_line, 8, "Base %d\n", server_num);
    strcat(status_message, base_message_line);

    for(int i=0; i<NUM_OF_STEPS; i++){
        snprintf(turn_message_line, 15, "turn %d\n", turn);
        strcat(status_message, turn_message_line);

        snprintf(fixed_location_message_line, 17, "fixedLocation %d\n", (i+1));
        strcat(status_message, fixed_location_message_line);

        for(int n=0; n<MAX_NUM_OF_ATTACK_POKEMONS; n++){
            if(attack_pokemons[n].id !=0 && battle_field[i] == attack_pokemons[n].id){
                snprintf(id_to_string, 7, "%d ", attack_pokemons[n].id);
                strcat(status_message, id_to_string);

                strcat(status_message, attack_pokemons[n].name);

                snprintf(life_to_string, 5, " %d", attack_pokemons[n].life);
                strcat(status_message, life_to_string);
                break;
            }
        }
        strcat(status_message, "\n\n");
    }

    socklen_t len = sizeof(cliaddr); 
    sendto(sockfd, (const char *)status_message, BUFSZ, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);

    free(status_message);
    free(base_message_line);
    free(turn_message_line);
    free(fixed_location_message_line);
    free(id_to_string);
    free(life_to_string);
    return;
}

void send_game_started_to_client(int sockfd, struct sockaddr_storage cliaddr, char buf[BUFSZ], int server_num){
    char response_to_client[BUFSZ];
    memset(response_to_client, 0, sizeof(response_to_client));
    socklen_t len = sizeof(cliaddr); 

    snprintf(response_to_client, 21, "game started: path %d", server_num);

    sendto(sockfd, (const char *)response_to_client, BUFSZ, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
    return;
}

void send_shotresp_to_client(int sockfd, struct sockaddr_storage cliaddr, char *received_params[4], char *status){
    char* shotresp_message = (char *)malloc(BUFSZ);
    socklen_t len = sizeof(cliaddr); 

    strcat(shotresp_message, "shotresp ");
    strcat(shotresp_message, received_params[0]);
    strcat(shotresp_message, " ");
    strcat(shotresp_message, received_params[1]);
    strcat(shotresp_message, " ");
    strcat(shotresp_message, received_params[2]);
    strcat(shotresp_message, " ");
    strcat(shotresp_message, status);

    sendto(sockfd, (const char *)shotresp_message, BUFSZ, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
    free(shotresp_message);
}

int defender_exists_and_didnt_attack(struct pokemon_defense defense_pokemons[NUM_OF_DEFENSE_POKEMONS], 
                                     int x_pos, int y_pos){

    printf("Chegou no teste 1\n");
    for(int i=0; i<NUM_OF_DEFENSE_POKEMONS; i++){
        if(defense_pokemons[i].x_pos == x_pos &&
           defense_pokemons[i].y_pos == y_pos &&
           defense_pokemons[i].has_attacked == 0
        )
        return 1;
    }
    return 0;
}

int pokemon_id_exists(struct pokemon_attack attack_pokemons[MAX_NUM_OF_ATTACK_POKEMONS], int attacker_id){
    printf("Chegou no teste 2\n");
    for(int i=0; i<MAX_NUM_OF_ATTACK_POKEMONS; i++){
        if(attack_pokemons[i].id == attacker_id) return 1;
    }
    return 0;
}

int defender_is_next_to_attacker(int defender_x_pos, int defender_y_pos, int battle_field[NUM_OF_STEPS], int attacker_id, int server_num){
    printf("Chegou no teste 3\n");
    int attacker_array_pos = 0;
    for(int i=0; i<NUM_OF_STEPS; i++){
        if(battle_field[i] == attacker_id){
            attacker_array_pos = i;
            break;
        }
    }

    if(defender_y_pos == (attacker_array_pos+1)) return 1;
    else return 0;   
}

void inform_defense_attacked(struct pokemon_defense defense_pokemons[NUM_OF_DEFENSE_POKEMONS], int defender_x_pos, int defender_y_pos){
    for(int i=0; i<NUM_OF_DEFENSE_POKEMONS; i++){
        if(defense_pokemons[i].x_pos == defender_x_pos && defense_pokemons[i].y_pos == defender_y_pos) defense_pokemons[i].has_attacked = 1;
    }
}

void kill_pokemon_and_free_battle_field_space(struct pokemon_attack attack_pokemons[MAX_NUM_OF_ATTACK_POKEMONS], int attacker_id, int battle_field_pos, int battle_field[NUM_OF_STEPS]){
    battle_field[battle_field_pos] = 0;

    for(int j=0; j<MAX_NUM_OF_ATTACK_POKEMONS; j++){
        if(attacker_id == attack_pokemons[j].id){
            attack_pokemons[j].id = 0;
            attack_pokemons[j].name = (char *)malloc(sizeof(char)*6);
            attack_pokemons[j].life = 0;
        }
    }

    num_of_pokemons_killed++;
    return;
}

void attack_pokemon(struct pokemon_attack attack_pokemons[MAX_NUM_OF_ATTACK_POKEMONS], int attacker_id, int battle_field[NUM_OF_STEPS]){
    int battle_field_pos = 0;
    for(int i=0; i<MAX_NUM_OF_ATTACK_POKEMONS; i++){
        if(attack_pokemons[i].id == attacker_id){
            attack_pokemons[i].life--;
            if(attack_pokemons[i].life <= 0){
                for(int h=0; h<NUM_OF_STEPS; h++){
                    if(battle_field[h] == attacker_id) battle_field_pos = h;
                    break;
                }
                printf("POS %d\n", battle_field_pos);
                kill_pokemon_and_free_battle_field_space(attack_pokemons, attack_pokemons[i].id, battle_field_pos, battle_field);
                break;
            }
            else{
                printf("Não matou\n");
            }
        }
    }
    return;
}

void walk_pokemons_on_battle_field(struct pokemon_attack attack_pokemons[MAX_NUM_OF_ATTACK_POKEMONS], int battle_fields[NUM_OF_SERVERS][NUM_OF_STEPS]){
    for(int i=0; i<NUM_OF_SERVERS; i++){
        if(battle_fields[i][NUM_OF_STEPS-1] != 0){
            kill_pokemon_and_free_battle_field_space(attack_pokemons, battle_fields[i][NUM_OF_STEPS-1] ,NUM_OF_STEPS-1, battle_fields[i]);
            num_of_pokemons_killed--;
            num_of_winner_pokemons++;
        }
        for(int j=NUM_OF_STEPS-2; j>=0; j--){
            if(battle_fields[i][j] != 0){
                battle_fields[i][j+1] = battle_fields[i][j];
                battle_fields[i][j] = 0;
            }
        }
    }
    return;
}

int main(int argc, char **argv) {
    int sockfd[NUM_OF_SERVERS];
    char buf[BUFSZ];
    int port_num = atoi(argv[2]);
    int battle_fields[NUM_OF_SERVERS][NUM_OF_STEPS];

    struct sockaddr_storage cliaddr;
    struct sockaddr_storage storage[NUM_OF_SERVERS];
    struct pokemon_defense defense_pokemons[NUM_OF_DEFENSE_POKEMONS];
    struct pokemon_attack attack_pokemons[MAX_NUM_OF_ATTACK_POKEMONS];

    for(int i=0; i<NUM_OF_SERVERS; i++){

        // Filling server information
        if (0 == strcmp(argv[1], "v4")) {
            struct sockaddr_in *addr4 = (struct sockaddr_in *)&storage[i];
            addr4->sin_family = AF_INET;
            addr4->sin_addr.s_addr = INADDR_ANY;
            addr4->sin_port = htons(port_num);
        } else if (0 == strcmp(argv[1], "v6")) {
            struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)&storage[i];
            addr6->sin6_family = AF_INET6;
            addr6->sin6_addr = in6addr_any;
            addr6->sin6_port = htons(port_num);
        }

        // Creating socket file descriptor
        if ( (sockfd[i] = socket(storage[i].ss_family, SOCK_DGRAM, 0)) < 0 ) {
            perror("socket creation failed");
            exit(EXIT_FAILURE);
        }
        
        // Bind the socket with the server address
        if ( bind(sockfd[i], (const struct sockaddr *)&storage[i], sizeof(storage[i])) < 0 )
        {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }

        port_num++;
    }

    initialize_attack_pokemons(attack_pokemons);
    initialize_battle_fields(battle_fields);
    generate_defense_pokemons(defense_pokemons);
    
    
    memset(&cliaddr, 0, sizeof(cliaddr));
    socklen_t len = sizeof(cliaddr);  //len is value/resuslt

    while(1){
        memset(buf, 0, sizeof(buf));

        char* aux = (char *)malloc(BUFSZ);
        int n = recvfrom(sockfd[0], (char *)buf, BUFSZ, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len);
        printf("\nN: %d\n", n);

        if(!strcmp("quit\n", buf)) break;
        snprintf(aux, BUFSZ, "Deaths: %d \nWinners: %d\n", num_of_pokemons_killed, num_of_winner_pokemons);
        if(!strcmp("stats\n", buf)) sendto(sockfd[0], aux, BUFSZ, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
        free(aux);
        if(!strcmp("initialize servers", buf)){
            for(int server_num=1; server_num<=NUM_OF_SERVERS; server_num++){
                send_game_started_to_client(sockfd[server_num-1], cliaddr, buf, server_num);
            }
        }

        else if(!strcmp("getdefenders\n", buf)){
            char *defenders_response = (char *) malloc(sizeof(char)*75);
            snprintf(defenders_response, 75, "defender [[%d, %d], [%d, %d], [%d, %d], [%d, %d], [%d, %d], [%d, %d], [%d, %d], [%d, %d]]",
                     defense_pokemons[0].x_pos,
                     defense_pokemons[0].y_pos,
                     defense_pokemons[1].x_pos,
                     defense_pokemons[1].y_pos,
                     defense_pokemons[2].x_pos,
                     defense_pokemons[2].y_pos,
                     defense_pokemons[3].x_pos,
                     defense_pokemons[3].y_pos,
                     defense_pokemons[4].x_pos,
                     defense_pokemons[4].y_pos,
                     defense_pokemons[5].x_pos,
                     defense_pokemons[5].y_pos,
                     defense_pokemons[6].x_pos,
                     defense_pokemons[6].y_pos,
                     defense_pokemons[7].x_pos,
                     defense_pokemons[7].y_pos
            );
            sendto(sockfd[0], (const char *)defenders_response, BUFSZ, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
            // for(int i=1; i<NUM_OF_SERVERS; i++){
            //     sendto(sockfd[i], "\0", BUFSZ, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
            // }
            free(defenders_response);
        }

        // NEEDS COMMAND TO BE SPLIT
        else{

            char *p = strtok (buf, " ");
            char *command = malloc(sizeof(p));
            strcpy(command, p);

            char *received_params[4];
            for(int i=0; i<4; i++){
                received_params[i] = (char *)malloc(sizeof(char)*1);
            }
            p = strtok (NULL, " ");
            int v = 0;
            while (p != NULL)
            {
                strcpy(received_params[v++], p);
                p = strtok (NULL, " ");
            }

            if(!strcmp("getturn", command)){
                walk_pokemons_on_battle_field(attack_pokemons, battle_fields);

                printf("Ultima posicao %d\n", battle_fields[0][3]);
                printf("Ultima posicao %d\n", battle_fields[1][3]);
                printf("Ultima posicao %d\n", battle_fields[2][3]);
                printf("Ultima posicao %d\n", battle_fields[3][3]);

                int turn = atoi(received_params[0]);
                int spawn_chance = 25;
                if(turn==0) spawn_chance = 100;
                generate_random_attack_pokemons(attack_pokemons, battle_fields, spawn_chance);

                for(int q=0; q<NUM_OF_DEFENSE_POKEMONS; q++){
                    defense_pokemons[q].has_attacked = 0;
                }
                
                for(int server_num=1; server_num<=NUM_OF_SERVERS; server_num++){
                    send_status_to_client(defense_pokemons, attack_pokemons, cliaddr,  battle_fields[server_num-1], server_num, turn, sockfd[server_num-1]);
                }
            }

            else if(!strcmp("shot", command)){
                received_params[2][strcspn(received_params[2], "\n")] = 0;
                int defender_x_pos = atoi(received_params[0]);
                int defender_y_pos = atoi(received_params[1]);
                int attacker_id = atoi(received_params[2]);

                printf("X: %d / Y: %d / ID: %d\n", defender_x_pos, defender_y_pos, attacker_id);

                for(int server_num=0; server_num<NUM_OF_SERVERS; server_num++){

                    // attacker is not in this server's battle field
                    if(defender_x_pos == 0) if(server_num != 0) continue;
                    if(defender_x_pos == 1) if(server_num != 0 && server_num != 1) continue; 
                    if(defender_x_pos == 2) if(server_num != 1 && server_num != 2) continue; 
                    if(defender_x_pos == 3) if(server_num != 2 && server_num != 3) continue; 
                    if(defender_x_pos == 4) if(server_num != 3) continue; 

                    char* status = (char *)malloc(sizeof(char)*1);
                    if(defender_exists_and_didnt_attack(defense_pokemons, defender_x_pos, defender_y_pos) && 
                       pokemon_id_exists(attack_pokemons, attacker_id) &&
                       defender_is_next_to_attacker(defender_x_pos, defender_y_pos, battle_fields[server_num], attacker_id, server_num)){

                        printf("Vai atacar\n");
                        inform_defense_attacked(defense_pokemons, defender_x_pos, defender_y_pos);
                        attack_pokemon(attack_pokemons, attacker_id, battle_fields[server_num]);
                        strcpy(status, "0");
                        
                    }
                    else{
                        printf("Não vai atacar pq tem algum erro\n");
                        strcpy(status, "1");
                    }

                    send_shotresp_to_client(sockfd[server_num], cliaddr, received_params, status);
                    break;
                }
            }
            else{
                sendto(sockfd[0], "Error", BUFSZ, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
            }

            free(p);
            free(command);
            for(int h=0; h<3; h++){
                free(received_params[h]);
            }
        }

        puts(buf);
    }
    exit(EXIT_SUCCESS);
}