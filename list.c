#include "list.h"

unsigned int buffer_size = 128;
command* head = NULL;
command* curr = NULL;

command *create(void){
    command *ptr = (command*)malloc(sizeof(struct command));
    ptr->op     =  (char*)malloc(sizeof(char)*32);
    memset(ptr->args,0,3*sizeof(int));
    ptr->next   = NULL;
    return ptr;
}

int insert(command* ptr){
    command* tmp = head;
    if(!head){
        head = ptr;
    }else{
        while(tmp->next!=NULL){
            tmp = tmp->next;
        }

        tmp->next = ptr;
    }
    return 1;
}

void free_all(void){  
    command *tmp;
    while(head!=NULL){
        tmp=head;
        head = head->next;
        free(tmp->op);
        free(tmp);
    }
}

void readnparse_commands(char* file){
    char *line = NULL;
    size_t len = 0;
    FILE *fp = fopen(file,"r");
    char *token;
    command *tmp;
    int cnt=0;

    if(!fp){
        MPI_Finalize();
        printf("Error opening testfile <%s>\n",file);
        exit(EXIT_FAILURE); 
    }

    while ((getline(&line, &len, fp)) != -1) {
        
        cnt=0;
        token = strtok(line, DELIMETERS);
        tmp = create();
        while( token != NULL ) {
            if(cnt==0){
                strncpy(tmp->op,token,strlen(token));
            } 
            else{
                // assert(cnt<4);
                tmp->args[cnt-1] = atoi(token);
            }
            
            cnt++;
            token = strtok(NULL, DELIMETERS);
        }
        insert(tmp);
    }
}

int op2num(char* op){
         if(!strcmp(op,"CLIENT"))       return 1;
    else if(!strcmp(op,"NEIGHBOR"))     return 2;
    else if(!strcmp(op,"DEPOSIT"))      return 3;
    else if(!strcmp(op,"WITHDRAW"))     return 4;
    else if(!strcmp(op,"TRANSFER"))     return 5;
    else                                return 0;
}

void print_list(){
    command *tmp=head;
    while(tmp!=NULL){
        printf("(%d)%s:%d:%d:%d\n",op2num(tmp->op),tmp->op,tmp->args[0],tmp->args[1],tmp->args[2]);
        tmp=tmp->next;
    }
}