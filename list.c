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

clients *c_create(void){
    clients *ptr = (clients*)malloc(sizeof(struct clients));
    ptr->id = 0;
    ptr->balance = 0;
    ptr->next   = NULL;
    return ptr;
}

int c_insert(clients **c_head,clients* ptr){
    clients* tmp = (*c_head);
    if(!(*c_head)){
        (*c_head) = ptr;
    }else{
        while(tmp->next!=NULL){
            tmp = tmp->next;
        }

        tmp->next = ptr;
    }
    return 1;
}

neighbors *n_create(void){
    neighbors *ptr = (neighbors*)malloc(sizeof(struct neighbors));
    ptr->id = 0;
    ptr->next   = NULL;
    return ptr;
}

int n_insert(neighbors **n_head,neighbors* ptr){
    neighbors* tmp = (*n_head);
    if(!(*n_head)){
        (*n_head) = ptr;
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

    fclose(fp);
}

int op2num(char* op){
         if(!strcmp(op,"CLIENT"))       return 1;
    else if(!strcmp(op,"NEIGHBOR"))     return 2;
    else if(!strcmp(op,"DEPOSIT"))      return 3;
    else if(!strcmp(op,"WITHDRAW"))     return 4;
    else if(!strcmp(op,"TRANSFER"))     return 5;
    else if(!strcmp(op,"TERMINATE"))    return 6;
    else                                return 0;
}

char* op2str(int op){
    switch(op){
        case 0: return  "ACK";
        case 1: return  "CLIENT";
        case 2: return  "NEIGHBOR";
        case 3: return  "DEPOSIT";
        case 4: return  "WITHDRAW";
        case 5: return  "TRANSFER";
        case 6: return  "TERMINATE";
        default: exit(-1);
    }
}
void print_list(){
    command *tmp=head;
    while(tmp!=NULL){
        printf("(%d)%s:%d:%d:%d\n",op2num(tmp->op),tmp->op,tmp->args[0],tmp->args[1],tmp->args[2]);
        tmp=tmp->next;
    }
}

clients *get_client(clients* head,int id){
    clients *c_curr = head;
    while(c_curr!=NULL){
        if(c_curr->id == id){
            return c_curr;
        }
        c_curr = c_curr->next;
    }
    return NULL;
}