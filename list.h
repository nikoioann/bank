#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <mpi/mpi.h>

#define DELIMETERS          " \n\r\t"
#define COORDINATOR         0
#define CLIENT              1
#define NEIGHBOR            2
#define DEPOSIT             3
#define WITHDRAW            4
#define TRANSFER            5
#define SHUTDOWN            6
#define DEPOSIT_OK          7
#define DEPOSIT_ACK         8
#define ACK_TAG             9
#define WITHDRAW_OK         10
#define WITHDRAW_FAILED     11
#define WITHDRAW_ACK        12
#define TRANSFER_ACK        13
#define TRANSFER_FAILED_NOT_ENOUGH_MONEY     14
#define TRANSFER_RECV       15
#define TRANSFER_OK         16
#define TRANSFER_COMPLETED  17
#define TRANSFER_FWD        18
#define TRANSFER_FWD_OK     19
#define TRANSFER_FAILED_NO_DEST 20
#define NOT_FOUND           21

#define dictionary          int**


typedef struct command{
    char* op;
    int args[4];
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

typedef struct transfers{
    int tid;
    int src;
    int dest;
    int amount;
    int banksend;
    struct transfers *next;
}transfers;

extern command *head;
extern command *curr;
extern unsigned int buffer_size;

command*    create(void);
clients*    c_create(void);
neighbors*  n_create(void);
transfers*  t_create(void);
int         insert(command*);
int         c_insert(clients**,clients*);
int         n_insert(neighbors**,neighbors*);
int         t_insert(transfers**,transfers*);
clients*    get_client(clients*,int);
transfers*  get_transfer(transfers*,int);
void        free_all(void);
void        readnparse_commands(char*);
void        print_list(); 
int         op2num(char*);
char*       op2str(int);

int find_next_neighbor(neighbors*,int,dictionary);
int add_to_dictionary(dictionary*,int,int);