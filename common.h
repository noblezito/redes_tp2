#pragma once

#include <stdlib.h>

#include <arpa/inet.h>

const int NUM_OF_SERVERS = 4;
const int BUFSZ = 1024;
const int NUM_OF_STEPS = 4;
const int NUM_OF_DEFENSE_POKEMONS = 6;
const int MAX_NUM_OF_ATTACK_POKEMONS = 16;

struct pokemon_attack
{
   int id;
   char* name;
   int life;
};

struct pokemon_defense
{
   int x_pos;
   int y_pos;
   int has_attacked;
};

void logexit(const char *msg);

int addrparse(const char *addrstr, int port, struct sockaddr_storage *storage);

int server_sockaddr_init(const char *proto, int port, struct sockaddr_storage *storage);
