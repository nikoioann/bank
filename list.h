#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <mpi.h>

#define DELIMETERS " \n\r\t"
#define ACK_TAG 0
#define CLIENT 1
#define NEIGHBOR 2
#define DEPOSIT 3
#define WITHDRAW 4
#define TRANSFER 5

typedef struct command{
    char* op;
    int args[3];
    struct command *next;
}command;

extern command *head;
extern command *curr;
extern unsigned int buffer_size;

command *create(void);
int insert(command*);
void free_all(void);
void readnparse_commands(char*);
void print_list(); 
int op2num(char*);