#include "list.h"

int main(int argc, char *argv[]){
	int rank, world_size, bank_offset,i; 

	if(argc<3){
        printf("Error on number of arguments given\n number of banks and testfile needed\n");
        exit(EXIT_FAILURE);
    }

    /** MPI Initialisation **/
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    bank_offset = atoi(argv[1]);

    if(bank_offset >= world_size){
        MPI_Finalize();
        printf("Error on number of banks given %d:%d\n",bank_offset,world_size);
        exit(EXIT_FAILURE);
    }

    MPI_Status status;
    int client_bank[4];
    
    // FILE *f = fopen("balances.txt","a+");


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

        //printf("COORDINATOR DONE\n");

        while(deposit_cntr > 0 || withdraw_cntr > 0 || transfer_cntr > 0){
            // printf("%d | %d | %d \n",deposit_cntr,withdraw_cntr,transfer_cntr);
            MPI_Recv(client_bank,4, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            // printf("[rank: %d]  Coordinator received message <%s>: [%d, %d, %d] from %d\n", rank, op2str(status.MPI_TAG), client_bank[0], client_bank[1], client_bank[2], status.MPI_SOURCE);
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
        for(i = 1;i<world_size;i++)
            MPI_Send(client_bank,4,MPI_INT,i,SHUTDOWN,MPI_COMM_WORLD);

    }else
    if(rank <= bank_offset){//banks
        int          flag=1,bank_balance=0,neighbors_cntr=0,next_bank=0;
        clients     *c_head = NULL,*c_curr = NULL;
        neighbors   *n_head = NULL,*n_curr = NULL;
        transfers   *t_head = NULL,*t_curr = NULL;

        while(flag){
            MPI_Recv(client_bank,4, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            // printf("[rank: %d]  Bank received message <%s>: [%d, %d, %d] from %d\n", rank,op2str(status.MPI_TAG),client_bank[0], client_bank[1], client_bank[2],status.MPI_SOURCE);
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
                    
                    if((c_curr = get_client(c_head,client_bank[0])) == NULL){
                        printf("ERROR : client not found\n");
                    }

                    if(c_curr){
                        bank_balance +=client_bank[1];
                        c_curr->balance += client_bank[1];
                    }

                    client_bank[0] = client_bank[1];client_bank[1] = client_bank[2] = client_bank[3] = 0;
                    MPI_Send(client_bank,4,MPI_INT,status.MPI_SOURCE,DEPOSIT_OK,MPI_COMM_WORLD);
                    break;
                case WITHDRAW:
                    if((c_curr = get_client(c_head,client_bank[0])) == NULL){
                        printf("ERROR : client not found\n");
                    }

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
                    if((c_curr = get_client(c_head,client_bank[0])) == NULL){
                        printf("ERROR : client not found\n");
                    }
                    
                    if(c_curr && c_curr->balance >= client_bank[2]){
                        t_curr = t_create();
                        t_curr -> src       = client_bank[0];
                        t_curr -> dest      = client_bank[1];
                        t_curr -> amount    = client_bank[2];
                        t_curr -> tid       = client_bank[3];
                        t_curr -> banksend  = 0;
                        t_curr -> nghbr_ptr = n_head;
                        t_insert(&t_head,t_curr);

                        c_curr->balance -= client_bank[2];
                        bank_balance-=client_bank[2];

                        if((c_curr = get_client(c_head,t_curr->dest)) != NULL){
                            client_bank[0] = client_bank[2];client_bank[1] = client_bank[3]; client_bank[2] = client_bank[3] = 0;
                            MPI_Send(client_bank,4,MPI_INT,t_curr->dest,TRANSFER_RECV,MPI_COMM_WORLD);
                        }else{//not banks customer
                            if(t_curr -> nghbr_ptr){
                                client_bank[0] = t_curr->src;
                                client_bank[1] = t_curr->dest;
                                client_bank[2] = t_curr->amount;
                                client_bank[3] = t_curr->tid;
                                MPI_Send(client_bank,4,MPI_INT,t_curr -> nghbr_ptr -> id,TRANSFER_FWD,MPI_COMM_WORLD);
                                // printf("\n\nBank %d sent to bank %d for transfer <%d> from client <%d> to client <%d>\n"
                                //  ,rank,t_curr->nghbr_ptr->id,t_curr->tid,t_curr->src,t_curr->dest);
                                t_curr -> nghbr_ptr = t_curr -> nghbr_ptr -> next;
                            }else{
                                client_bank[0] = t_curr->tid;client_bank[1] = client_bank[2] = client_bank[3] = 0;
                                MPI_Send(client_bank,4,MPI_INT,status.MPI_SOURCE,TRANSFER_FAILED_NO_DEST,MPI_COMM_WORLD);
                                if((c_curr = get_client(c_head,t_curr->src))!=NULL){
                                    c_curr->balance += t_curr->amount;
                                    bank_balance+=t_curr->amount;
                                }
                                // printf("\n\nBank %d sent no dest to client %d for transfer <%d> from client <%d> to client <%d>\n"
                                // ,rank,status.MPI_SOURCE,t_curr->tid,t_curr->src,t_curr->dest);
                            }
                        }

                    }else{
                        client_bank[0] = client_bank[3];client_bank[1] = client_bank[2] = client_bank[3] = 0;
                        MPI_Send(client_bank,4,MPI_INT,status.MPI_SOURCE,TRANSFER_FAILED_NOT_ENOUGH_MONEY,MPI_COMM_WORLD);
                        if((c_curr = get_client(c_head,status.MPI_SOURCE))!=NULL){
                            c_curr->balance += client_bank[2];
                            bank_balance+=client_bank[2];
                        }
                    }

                    break;
                case TRANSFER_OK:
                    if((c_curr = get_client(c_head,status.MPI_SOURCE)) != NULL){
                        if((t_curr = get_transfer(t_head,client_bank[0])) != NULL){
                            c_curr->balance+= t_curr->amount;
                            bank_balance+=t_curr->amount;
                            client_bank[0] = t_curr->tid; client_bank[1] = client_bank[2] = client_bank[3] = 0;
                            if(t_curr->banksend){
                                MPI_Send(client_bank,4,MPI_INT,t_curr->banksend,TRANSFER_FWD_OK,MPI_COMM_WORLD);
                            }else{
                                MPI_Send(client_bank,4,MPI_INT,t_curr->src,TRANSFER_COMPLETED,MPI_COMM_WORLD);
                            }
                        }else{
                            printf("ERROR : Transfer Not Found(Transfer_OK\n");
                        }
                    }else{
                        printf("ERROR : Client Not Found(Transfer_OK)\n");
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
                    t_curr -> nghbr_ptr = n_head;
                    t_insert(&t_head,t_curr);

                    if((c_curr = get_client(c_head,t_curr->dest)) != NULL){
                        client_bank[0] = t_curr -> amount; client_bank[1] = t_curr -> tid; client_bank[2] = client_bank[3] = 0;
                        MPI_Send(client_bank,4,MPI_INT,t_curr->dest,TRANSFER_RECV,MPI_COMM_WORLD);
                        
                    }else{//not banks customer

                        if(t_curr -> nghbr_ptr && t_curr -> nghbr_ptr -> id == t_curr -> banksend){
                            t_curr -> nghbr_ptr = t_curr -> nghbr_ptr -> next;
                        }

                        if(t_curr -> nghbr_ptr){
                            client_bank[0] = t_curr->src;
                            client_bank[1] = t_curr->dest;
                            client_bank[2] = t_curr->amount;
                            client_bank[3] = t_curr->tid;
                            MPI_Send(client_bank,4,MPI_INT,t_curr-> nghbr_ptr -> id,TRANSFER_FWD,MPI_COMM_WORLD);
                            
                            // printf("\n\nBank %d sent to bank %d for transfer <%d> from client <%d> to client <%d>\n"
                            // ,rank,t_curr->nghbr_ptr->id,t_curr->tid,t_curr->src,t_curr->dest);
                            
                            t_curr -> nghbr_ptr = t_curr -> nghbr_ptr -> next;
                        }else{
                            client_bank[0] = client_bank[3]; client_bank[1] = client_bank[2] = client_bank[3] = 0;
                            MPI_Send(client_bank,4,MPI_INT,t_curr->banksend,NOT_FOUND,MPI_COMM_WORLD);
                            // printf("\n\nBank %d sent not found to bank %d for transfer <%d> from client <%d> to client <%d>\n"
                            // ,rank,t_curr->banksend,t_curr->tid,t_curr->src,t_curr->dest);
                        }
                    }
                    break;
                case TRANSFER_FWD_OK:
                    if((t_curr = get_transfer(t_head,client_bank[0]))){
                        if(t_curr->banksend){
                            MPI_Send(client_bank,4,MPI_INT,t_curr->banksend,TRANSFER_FWD_OK,MPI_COMM_WORLD);
                        }else{
                            MPI_Send(client_bank,4,MPI_INT,t_curr->src,TRANSFER_COMPLETED,MPI_COMM_WORLD);
                        }
                    }else{
                        printf("ERROR : Bank transfer fwd ok transfer not found\n");
                    }
                    break;
                case NOT_FOUND:
                    if((t_curr = get_transfer(t_head,client_bank[0])) == NULL){
                        printf("TRANSFER NOT FOUND %d\n",client_bank[0]);
                        break;
                    }

                    if(t_curr -> nghbr_ptr && t_curr -> nghbr_ptr -> id == t_curr -> banksend){
                        t_curr -> nghbr_ptr = t_curr -> nghbr_ptr -> next;
                    }

                    if(t_curr -> nghbr_ptr){
                        client_bank[0] = t_curr->src;
                        client_bank[1] = t_curr->dest;
                        client_bank[2] = t_curr->amount;
                        client_bank[3] = t_curr->tid;
                        MPI_Send(client_bank,4,MPI_INT,t_curr-> nghbr_ptr -> id,TRANSFER_FWD,MPI_COMM_WORLD);
                        
                        // printf("\n\nBank %d sent to bank %d for transfer <%d> from client <%d> to client <%d>\n"
                        //     ,rank,t_curr->nghbr_ptr->id,t_curr->tid,t_curr->src,t_curr->dest);
                            
                        t_curr -> nghbr_ptr = t_curr -> nghbr_ptr -> next;
                    }else{
                        if(t_curr->banksend){
                            MPI_Send(client_bank,4,MPI_INT,t_curr->banksend,NOT_FOUND,MPI_COMM_WORLD);
                            // printf("\n\nBank %d sent not found to bank %d for transfer <%d> from client <%d> to client <%d>\n"
                            //     ,rank,t_curr->banksend,t_curr->tid,t_curr->src,t_curr->dest);
                        }else{
                            MPI_Send(client_bank,4,MPI_INT,t_curr->src,TRANSFER_FAILED_NO_DEST,MPI_COMM_WORLD);
                            // printf("\n\nBank %d sent no dest to client %d for transfer <%d> from client <%d> to client <%d>\n"
                            //     ,rank,t_curr->src,t_curr->tid,t_curr->src,t_curr->dest);
                            if((c_curr = get_client(c_head,t_curr->src))!=NULL){
                                c_curr->balance += t_curr->amount;
                                bank_balance+=t_curr->amount;
                            }
                        }

                    }

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

        // fprintf(f,"\nbank <%d> with balance <%d> ",rank,bank_balance);
        // c_curr = c_head;
        // while(c_curr != NULL){
        //     fprintf(f,"|%d:<%d,%d>|",rank,c_curr->id,c_curr->balance);
        //     c_curr = c_curr->next;
        // }
        // fprintf(f,"\n");

    }else{//clients
        int mybank=0,flag=1,mybalance = 0;
        transfers *t_head = NULL,*t_curr = NULL;

        MPI_Recv(client_bank,4 , MPI_INT, MPI_ANY_SOURCE, CLIENT, MPI_COMM_WORLD, &status);
        mybank = client_bank[1];
        MPI_Send(client_bank,4,MPI_INT,status.MPI_SOURCE,ACK_TAG,MPI_COMM_WORLD);

        while(flag){
            MPI_Recv(client_bank,4 , MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            //printf("[rank: %d]  Client received message <%s>: [%d, %d, %d] from %d\n", rank, op2str(status.MPI_TAG), client_bank[0], client_bank[1], client_bank[2], status.MPI_SOURCE);
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
                    if(t_curr && (t_curr = get_transfer(t_head,client_bank[0])) != NULL){
                        MPI_Send(client_bank,4,MPI_INT,COORDINATOR,TRANSFER_ACK,MPI_COMM_WORLD);
                        printf("CLIENT <%d> TRANSFER <%d> TO <%d> NOT ENOUGH MONEY \n",t_curr->src,t_curr->amount,t_curr->dest);
                    }else{
                        printf("ERROR : Client transfer not found\n");
                    }
                    break;
                case TRANSFER_RECV:
                    mybalance+=client_bank[0];
                    client_bank[0] = client_bank[1];client_bank[1] = client_bank[2] = client_bank[3] = 0;
                    MPI_Send(client_bank,4,MPI_INT,status.MPI_SOURCE,TRANSFER_OK,MPI_COMM_WORLD);
                    break;
                case TRANSFER_COMPLETED:
                    if(t_curr && (t_curr = get_transfer(t_head,client_bank[0])) != NULL){
                        mybalance-= t_curr->amount;
                        MPI_Send(client_bank,4,MPI_INT,COORDINATOR,TRANSFER_ACK,MPI_COMM_WORLD);
                        printf("CLIENT <%d> TRANSFER <%d> TO <%d> OK\n",t_curr->src,t_curr->amount,t_curr->dest);
                    }else{
                        printf("ERROR : Client transfer not found\n");
                    }
                    break;
                case TRANSFER_FAILED_NO_DEST:
                    if(t_curr && (t_curr = get_transfer(t_head,client_bank[0])) != NULL){
                        MPI_Send(client_bank,4,MPI_INT,COORDINATOR,TRANSFER_ACK,MPI_COMM_WORLD);
                        printf("CLIENT <%d> TRANSFER <%d> TO <%d> NO DEST\n",t_curr->src,t_curr->amount,t_curr->dest);
                    }else{
                        printf("ERROR : Client transfer not found\n");
                    }
                    break;
                case SHUTDOWN: flag=0;break;
                default: printf("DEFAULT CASE SWITCH CLIENT\n");break;
                
            }
        }
        // fprintf(f,"CLIENT BALANCE |%d:<%d><%d>\n",mybank,rank,mybalance);
        // printf("\nCLIENT<%d> : mybank <%d>\n",rank,mybank);
    }

    // fclose(f);
    MPI_Finalize();

    return 0;
}


