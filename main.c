#include "list.h"



int main(int argc, char *argv[]){
	int rank, world_size, bank_offset; 
    // char *file;

	if(argc<3){
        printf("Error on number of arguments given\n number of banks and testfile needed\n");
        exit(EXIT_FAILURE);
    }

    /** MPI Initialisation **/
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    bank_offset = atoi(argv[1]);
    // file = strdup(argv[2]);

    if(bank_offset >= world_size){
        MPI_Finalize();
        printf("Error on number of banks given %d:%d\n",bank_offset,world_size);
        exit(EXIT_FAILURE);
    }

    MPI_Status status;
    int client_bank[4];
    
    if(!rank){ //coordinator
        int deposit_cntr=0,withdraw_cntr=0,transfer_cntr=0,src=0;
    
        readnparse_commands(argv[2]);
        // print_list();

        curr = head;

        while(curr!=NULL){
            switch (op2num(curr->op))
            {
            case CLIENT:
                MPI_Send(curr->args,4,MPI_INT,curr->args[0],CLIENT,MPI_COMM_WORLD);
                MPI_Send(curr->args,4,MPI_INT,curr->args[1],CLIENT,MPI_COMM_WORLD);

                MPI_Recv(client_bank,4,MPI_INT,MPI_ANY_SOURCE,ACK_TAG,MPI_COMM_WORLD,&status);
		        // printf("Coordinator received ack message from %d\n", status.MPI_SOURCE);
                MPI_Recv(client_bank,4,MPI_INT,MPI_ANY_SOURCE,ACK_TAG,MPI_COMM_WORLD,&status);
		        // printf("Coordinator received ack message from %d\n", status.MPI_SOURCE);
                break;
            case NEIGHBOR:
                MPI_Send(curr->args,4,MPI_INT,curr->args[0],NEIGHBOR,MPI_COMM_WORLD);
                MPI_Send(curr->args,4,MPI_INT,curr->args[1],NEIGHBOR,MPI_COMM_WORLD);
                
                MPI_Recv(client_bank,4,MPI_INT,MPI_ANY_SOURCE,ACK_TAG,MPI_COMM_WORLD,&status);
		        // printf("Coordinator received ack message from %d\n", status.MPI_SOURCE);
                MPI_Recv(client_bank,4,MPI_INT,MPI_ANY_SOURCE,ACK_TAG,MPI_COMM_WORLD,&status);
		        // printf("Coordinator received ack message from %d\n", status.MPI_SOURCE);
                break;    
            case DEPOSIT:
                deposit_cntr++;
                MPI_Send(curr->args,4,MPI_INT,curr->args[0],DEPOSIT,MPI_COMM_WORLD);
                break;        
            case WITHDRAW:
                withdraw_cntr++;
                MPI_Send(curr->args,4,MPI_INT,curr->args[0],WITHDRAW,MPI_COMM_WORLD);
                break;
            case TRANSFER:
                transfer_cntr++;
                curr->args[3] = transfer_cntr;
                MPI_Send(curr->args,4,MPI_INT,curr->args[0],TRANSFER,MPI_COMM_WORLD);
            default:
                break;
            }
            
            curr = curr->next;
        }
        while(deposit_cntr || withdraw_cntr){
            MPI_Recv(client_bank,4, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            //printf("[rank: %d]  Coordinator received message <%s>: [%d, %d, %d] from %d\n", rank, op2str(status.MPI_TAG), client_bank[0], client_bank[1], client_bank[2], status.MPI_SOURCE);
            switch(status.MPI_TAG){
                case DEPOSIT_ACK:
                    deposit_cntr --;
                    break;    
                case WITHDRAW_ACK:
                    withdraw_cntr--;
                    break;
                case TRANSFER_ACK:
                    transfer_cntr--;
                    break;
            }
        }
        for(int i = 1;i<world_size;i++)
            MPI_Send(client_bank,4,MPI_INT,i,SHUTDOWN,MPI_COMM_WORLD);

    }else
    if(rank <= bank_offset){//banks
        int          flag=1,bank_balance=0,neighbors_cntr=0,next_bank=0;
        clients     *c_head = NULL,*c_curr = NULL;
        neighbors   *n_head = NULL,*n_curr = NULL;
        transfers   *t_head = NULL,*t_curr = NULL;
        dictionary  d_head;

        while(flag){
            MPI_Recv(client_bank,4, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            printf("[rank: %d]  Bank received message <%s>: [%d, %d, %d] from %d\n", rank,op2str(status.MPI_TAG),client_bank[0], client_bank[1], client_bank[2],status.MPI_SOURCE);
            switch(status.MPI_TAG){
                case 0:break;
                case CLIENT:
                    c_curr = c_create();
                    c_curr->id = client_bank[0];
                    c_curr->balance = 0;
                    c_insert(&c_head,c_curr);
                    MPI_Send(client_bank,4,MPI_INT,status.MPI_SOURCE,ACK_TAG,MPI_COMM_WORLD);

                    break;
                case NEIGHBOR:

                    n_curr = n_create();
                    n_curr -> id = (rank == client_bank[0]) ? client_bank[1] : client_bank[0];
                    n_insert(&n_head,n_curr);
                    neighbors_cntr++;
                    MPI_Send(client_bank,4,MPI_INT,status.MPI_SOURCE,ACK_TAG,MPI_COMM_WORLD);

                    break;
                case DEPOSIT:
                    
                    if((c_curr = get_client(c_head,client_bank[0])) == NULL) 
                        printf("ERROR : client not found\n");

                    bank_balance +=client_bank[1];
                    c_curr->balance += client_bank[1];

                    client_bank[0] = client_bank[1];client_bank[1] = client_bank[2] = client_bank[3] = 0;
                    MPI_Send(client_bank,4,MPI_INT,status.MPI_SOURCE,DEPOSIT_OK,MPI_COMM_WORLD);
                    break;
                case WITHDRAW:
                    if((c_curr = get_client(c_head,client_bank[0])) == NULL) 
                        printf("ERROR : client not found\n");

                    if(c_curr && c_curr->balance >= client_bank[1]){
                        bank_balance -= client_bank[1];                    
                        c_curr->balance -= client_bank[1];

                        client_bank[0] = client_bank[1];client_bank[1] = client_bank[2] = client_bank[3] = 0;
                        MPI_Send(client_bank,4,MPI_INT,status.MPI_SOURCE,WITHDRAW_OK,MPI_COMM_WORLD);
                    }else{
                        client_bank[0] = client_bank[1];client_bank[1] = client_bank[2] = client_bank[3] = 0;
                        MPI_Send(client_bank,4,MPI_INT,status.MPI_SOURCE,WITHDRAW_FAILED,MPI_COMM_WORLD);

                    }
                    break;
                case TRANSFER:
                    if((c_curr = get_client(c_head,client_bank[0])) == NULL) 
                        printf("ERROR : client not found\n");
                    
                    if(c_curr && c_curr->balance >= client_bank[2]){
                        t_curr = t_create();
                        t_curr -> src       = client_bank[0];
                        t_curr -> dest      = client_bank[1];
                        t_curr -> amount    = client_bank[2];
                        t_curr -> tid       = client_bank[3];
                        t_curr -> banksend  = 0;
                        t_insert(&t_head,t_curr);

                        c_curr->balance -= client_bank[2];

                        if((c_curr = get_client(c_head,t_curr->dest)) != NULL){
                            client_bank[0] = client_bank[2];client_bank[1] = client_bank[3]; client_bank[2] = client_bank[3] = 0;
                            MPI_Send(client_bank,4,MPI_INT,t_curr->dest,TRANSFER_RECV,MPI_COMM_WORLD);
                        }else{//not banks customer
                            if((next_bank = find_next_neighbor(n_head,neighbors_cntr,d_head)) != -1){
                                MPI_Send(client_bank,4,MPI_INT,next_bank,TRANSFER_FWD,MPI_COMM_WORLD);
                                add_to_dictionary(&d_head,next_bank,t_curr->tid);
                            }else{
                                MPI_Send(client_bank,4,MPI_INT,status.MPI_SOURCE,TRANSFER_FAILED_NO_DEST,MPI_COMM_WORLD);
                            }
                            //add next bank to dictionary
                        }

                    }else{
                        client_bank[0] = client_bank[3];client_bank[1] = client_bank[2] = client_bank[3] = 0;
                        MPI_Send(client_bank,4,MPI_INT,status.MPI_SOURCE,TRANSFER_FAILED_NOT_ENOUGH_MONEY,MPI_COMM_WORLD);

                    }

                    break;
                case TRANSFER_OK:
                    if((c_curr = get_client(c_head,status.MPI_SOURCE)) != NULL){
                        if((t_curr = get_transfer(t_head,client_bank[0])) != NULL){
                        c_curr->balance+= t_curr->amount;
                        if(t_curr->banksend)
                            MPI_Send(client_bank,4,MPI_INT,t_curr->banksend,TRANSFER_FWD_OK,MPI_COMM_WORLD);
                        }else{
                            MPI_Send(client_bank,4,MPI_INT,t_curr->src,TRANSFER_COMPLETED,MPI_COMM_WORLD);
                        }
                    }
                    break;
                case TRANSFER_FWD:
                    if((t_curr = get_transfer(t_head,client_bank[3])) != NULL ){
                        client_bank[0] = client_bank[3];client_bank[1] = client_bank[2] = client_bank[3] = 0;
                        MPI_Send(client_bank,4,MPI_INT,status.MPI_SOURCE,NOT_FOUND,MPI_COMM_WORLD);
                        break;
                    }

                    t_curr = t_create();
                    t_curr -> src       = client_bank[0];
                    t_curr -> dest      = client_bank[1];
                    t_curr -> amount    = client_bank[2];
                    t_curr -> tid       = client_bank[3];
                    t_curr -> banksend  = status.MPI_SOURCE;
                    t_insert(&t_head,t_curr);

                    if((c_curr = get_client(c_head,t_curr->dest)) != NULL){
                        client_bank[0] = client_bank[2];client_bank[1] = client_bank[3]; client_bank[2] = client_bank[3] = 0;
                        bank_balance+=t_curr->amount;
                        MPI_Send(client_bank,4,MPI_INT,t_curr->dest,TRANSFER_RECV,MPI_COMM_WORLD);
                        
                    }else{//not banks customer
                        if((next_bank = find_next_neighbor(n_head,neighbors_cntr,d_head)) != -1){
                            MPI_Send(client_bank,4,MPI_INT,next_bank,TRANSFER_FWD,MPI_COMM_WORLD);
                            add_to_dictionary(&d_head,next_bank,t_curr->tid);
                        }else{
                            client_bank[0] = client_bank[3];client_bank[1] = client_bank[2] = client_bank[3] = 0;
                            MPI_Send(client_bank,4,MPI_INT,status.MPI_SOURCE,NOT_FOUND,MPI_COMM_WORLD);
                        }
                        //add next bank to dictionary
                    }
                    break;
                case TRANSFER_FWD_OK:
                    if((t_curr = get_transfer(t_head,client_bank[0]))){
                        if(t_curr->banksend){
                            MPI_Send(client_bank,4,MPI_INT,t_curr->banksend,TRANSFER_FWD_OK,MPI_COMM_WORLD);
                        }else{
                            bank_balance -= t_curr->amount;
                            MPI_Send(client_bank,4,MPI_INT,t_curr->src,TRANSFER_COMPLETED,MPI_COMM_WORLD);
                        }
                    }
                    break;
                case NOT_FOUND:
                    if((next_bank = find_next_neighbor(n_head,neighbors_cntr,d_head)) != -1){
                        if((t_curr = get_transfer(t_head,client_bank[0]))){
                            client_bank[0] = t_curr->src;
                            client_bank[1] = t_curr->dest;
                            client_bank[2] = t_curr->amount;
                            client_bank[3] = t_curr->tid;
                            MPI_Send(client_bank,4,MPI_INT,next_bank,TRANSFER_FWD,MPI_COMM_WORLD);
                            add_to_dictionary(&d_head,next_bank,t_curr->tid);
                        }
                    }else{
                        client_bank[0] = client_bank[3];client_bank[1] = client_bank[2] = client_bank[3] = 0;
                        MPI_Send(client_bank,4,MPI_INT,status.MPI_SOURCE,NOT_FOUND,MPI_COMM_WORLD);
                    }
                    //add next bank to dictionary
                    break;
                case SHUTDOWN:flag=0;break;
                default: printf("DEFAULT CASE SWITCH BANK\n");break;
            }

       }
        // printf("\nbank <%d> clients:",rank);
        // c_curr = c_head;
        // while(c_curr != NULL){
        //     printf("<%d>",c_curr->id);
        //     c_curr = c_curr->next;
        // }
        // printf("\n");

        // printf("\nbank <%d> neighbors:",rank);
        // n_curr = n_head;
        // while(n_curr != NULL){
        //     printf("<%d>",n_curr->id);
        //     n_curr = n_curr->next;
        // }
        // printf("\n");

        printf("\nbank <%d> clients balance:",rank);
        c_curr = c_head;
        while(c_curr != NULL){
            printf("|%d:<%d,%d>|",rank,c_curr->id,c_curr->balance);
            c_curr = c_curr->next;
        }
        printf("\n");

    }else{//clients
        int mybank=0,flag=1,mybalance = 0;
        transfers *t_head = NULL,*t_curr = NULL;

        MPI_Recv(client_bank,4 , MPI_INT, MPI_ANY_SOURCE, CLIENT, MPI_COMM_WORLD, &status);
        mybank = client_bank[1];
        printf("[rank: %d]  Client received message <%s>: [%d, %d, %d] from %d\n", rank, op2str(status.MPI_TAG), client_bank[0], client_bank[1], client_bank[2], status.MPI_SOURCE);
        MPI_Send(client_bank,4,MPI_INT,status.MPI_SOURCE,ACK_TAG,MPI_COMM_WORLD);

        while(flag){
            MPI_Recv(client_bank,4 , MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            switch(status.MPI_TAG){
                case 0: break;
                case DEPOSIT: 
                    MPI_Send(client_bank,4,MPI_INT,mybank,DEPOSIT,MPI_COMM_WORLD);
                    break;
                case DEPOSIT_OK:
                    mybalance+=client_bank[0];
                    MPI_Send(client_bank,4,MPI_INT,COORDINATOR,DEPOSIT_ACK,MPI_COMM_WORLD);
                    printf("CLIENT <%d> DEPOSITED <%d>\n",rank,client_bank[0]);
                    break;
                case WITHDRAW:
                    MPI_Send(client_bank,4,MPI_INT,mybank,WITHDRAW,MPI_COMM_WORLD);
                    break;
                case WITHDRAW_OK:
                    mybalance-=client_bank[0];
                    MPI_Send(client_bank,4,MPI_INT,COORDINATOR,WITHDRAW_ACK,MPI_COMM_WORLD);
                    printf("CLIENT <%d> WITHDRAW <%d> OK\n",rank,client_bank[0]);
                    break;
                case WITHDRAW_FAILED:
                    MPI_Send(client_bank,4,MPI_INT,COORDINATOR,WITHDRAW_ACK,MPI_COMM_WORLD);
                    printf("CLIENT <%d> WITHDRAW <%d> FAILED\n",rank,client_bank[0]);
                    break;
                case TRANSFER:
                    t_curr = t_create();
                    t_curr -> src       = client_bank[0];
                    t_curr -> dest      = client_bank[1];
                    t_curr -> amount    = client_bank[2];
                    t_curr -> tid       = client_bank[3];
                    t_curr -> banksend  = 0;
                    t_insert(&t_head,t_curr);

                    MPI_Send(client_bank,4,MPI_INT,mybank,TRANSFER,MPI_COMM_WORLD);
                    break;
                case TRANSFER_FAILED_NOT_ENOUGH_MONEY:
                    MPI_Send(client_bank,4,MPI_INT,COORDINATOR,TRANSFER_ACK,MPI_COMM_WORLD);
                    break;
                case TRANSFER_RECV:
                    mybalance+=client_bank[0];
                    client_bank[0] = client_bank[1];client_bank[1] = client_bank[2] = client_bank[3] = 0;
                    MPI_Send(client_bank,4,MPI_INT,status.MPI_SOURCE,TRANSFER_OK,MPI_COMM_WORLD);
                    break;
                case TRANSFER_COMPLETED:
                    if((t_curr = get_transfer(t_head,client_bank[0])) != NULL){
                            mybalance-= t_curr->amount;
                            // MPI_Send(client_bank,4,MPI_INT,COORDINATOR,TRANSFER_ACK,MPI_COMM_WORLD);
                    }
                    break;
                case SHUTDOWN: flag=0;break;
                default: printf("DEFAULT CASE SWITCH CLIENT\n");break;
                
            }

        }
        // printf("CLIENT |%d:<%d><%d>\n",mybank,rank,mybalance);
       // printf("\nCLIENT<%d> : mybank <%d>\n",rank,mybank);
    }


    MPI_Finalize();

    return 0;
}


