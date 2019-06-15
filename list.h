#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <mpi/mpi.h>

#define DELIMETERS " \n\r\t"
#define COORDINATOR         0
#define CLIENT              1
#define NEIGHBOR            2
#define DEPOSIT             3
#define WITHDRAW            4
#define TRANSFER            5
#define TERMINATE           6
#define DEPOSIT_OK          7
#define DEPOSIT_ACK         8
#define ACK_TAG             9
#define WITHDRAW_OK         10
#define WITHDRAW_FAILED     11

typedef struct command{
    char* op;
    int args[3];
    struct command *next;
}command;

typedef struct clients{
    int id;
    int balance;
    struct clients *next;
}clients;

typedef struct neighbors{
    int id;
    struct neighbors *next;
}neighbors;

extern command *head;
extern command *curr;
extern unsigned int buffer_size;

command *create(void);
int insert(command*);
clients *c_create(void);
int c_insert(clients**,clients*);
neighbors *n_create(void);
int n_insert(neighbors**,neighbors*);
void free_all(void);
void readnparse_commands(char*);
void print_list(); 
int op2num(char*);
char* op2str(int);
clients *get_client(clients*,int);