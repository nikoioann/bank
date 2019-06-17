#include "list.h"

unsigned int buffer_size = 128;
command* head = NULL;
command* curr = NULL;

command *create(void){
    command *ptr = (command*)malloc(sizeof(struct command));
    ptr->op     =  (char*)malloc(sizeof(char)*32);
    memset(ptr->args,0,4*sizeof(int));
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

transfers *t_create(void){
    transfers *ptr = (transfers*)malloc(sizeof(struct transfers));
    ptr->tid = ptr->src = ptr->dest = ptr->amount = ptr->banksend = 0;
    ptr->next = NULL;
    ptr->nghbr_ptr = NULL;

    return ptr;
}

int t_insert(transfers **n_head,transfers* ptr){
    transfers* tmp = (*n_head);
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
         if(!strcmp(op,"CLIENT"))           return 1;
    else if(!strcmp(op,"NEIGHBOR"))         return 2;
    else if(!strcmp(op,"DEPOSIT"))          return 3;
    else if(!strcmp(op,"WITHDRAW"))         return 4;
    else if(!strcmp(op,"TRANSFER"))         return 5;
    else if(!strcmp(op,"SHUTDOWN"))         return 6;
    else if(!strcmp(op,"DEPOSIT_OK"))       return 7;
    else if(!strcmp(op,"DEPOSIT_ACK"))      return 8;
    else if(!strcmp(op,"ACK_TAG"))          return 9;
    else if(!strcmp(op,"WITHDRAW_OK"))      return 10;
    else if(!strcmp(op,"WITHDRAW_FAILED"))  return 11;
    else if(!strcmp(op,"WITHDRAW_ACK"))     return 12;
    else if(!strcmp(op,"TRANSFER_ACK"))     return 13;
    else if(!strcmp(op,"TRANSFER_FAILED_NOT_ENOUGH_MONEY"))     return 14;
    else if(!strcmp(op,"TRANSFER_RECV"))    return 15;
    else if(!strcmp(op,"TRANSFER_OK"))      return 16;
    else if(!strcmp(op,"TRANSFER_COMPLETED"))       return 17;
    else if(!strcmp(op,"TRANSFER_FWD"))     return 18;
    else if(!strcmp(op,"TRANSFER_FWD_OK"))  return 19;
    else if(!strcmp(op,"TRANSFER_FAILED_NO_DEST"))  return 20;
    else if(!strcmp(op,"NOT_FOUND"))     return 21;
    else {
        printf("op2num else ERRROR\n");
        return 0;
    }
}

char* op2str(int op){
    switch(op){
        case 0: return  "ACK";
        case 1: return  "CLIENT";
        case 2: return  "NEIGHBOR";
        case 3: return  "DEPOSIT";
        case 4: return  "WITHDRAW";
        case 5: return  "TRANSFER";
        case 6: return  "SHUTDOWN";
        case 7: return  "DEPOSIT_OK";
        case 8: return  "DEPOSIT_ACK";
        case 9: return  "ACK_TAG";
        case 10: return  "WITHDRAW_OK";
        case 11: return  "WITHDRAW_FAILED";
        case 12: return  "WITHDRAW_ACK";
        case 13: return  "TRANSFER_ACK";
        case 14: return  "TRANSFER_FAILED_NOT_ENOUGH_MONEY";
        case 15: return  "TRANSFER_RECV";
        case 16: return  "TRANSFER_OK";
        case 17: return  "TRANSFER_COMPLETED";
        case 18: return  "TRANSFER_FWD";
        case 19: return  "TRANSFER_FWD_OK";
        case 20: return  "TRANSFER_FAILED_NO_DEST";
        case 21: return  "NOT_FOUND";
        default:     printf("op2str else ERRROR\n");return "empty";
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

transfers *get_transfer(transfers* head,int tid){
    transfers *curr = head;
    while(curr!=NULL){
        if(curr->tid == tid){
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

