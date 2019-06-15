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
    int client_bank[2];
    int any=0;
    if(!rank){ //coordinator
        readnparse_commands(argv[2]);
        // print_list();

        curr = head;

        while(curr!=NULL){
            switch (op2num(curr->op))
            {
            case CLIENT:
                MPI_Send(curr->args,3,MPI_INT,curr->args[0],CLIENT,MPI_COMM_WORLD);
                MPI_Send(curr->args,3,MPI_INT,curr->args[1],CLIENT,MPI_COMM_WORLD);

                MPI_Recv(&any,1,MPI_INT,MPI_ANY_SOURCE,ACK_TAG,MPI_COMM_WORLD,&status);
		        // printf("Coordinator received ack message from %d\n", status.MPI_SOURCE);
                MPI_Recv(&any,1,MPI_INT,MPI_ANY_SOURCE,ACK_TAG,MPI_COMM_WORLD,&status);
		        // printf("Coordinator received ack message from %d\n", status.MPI_SOURCE);
                break;
            case NEIGHBOR:
                MPI_Send(curr->args,3,MPI_INT,curr->args[0],NEIGHBOR,MPI_COMM_WORLD);
                MPI_Send(curr->args,3,MPI_INT,curr->args[1],NEIGHBOR,MPI_COMM_WORLD);
                
                MPI_Recv(&any,1,MPI_INT,MPI_ANY_SOURCE,ACK_TAG,MPI_COMM_WORLD,&status);
		        // printf("Coordinator received ack message from %d\n", status.MPI_SOURCE);
                MPI_Recv(&any,1,MPI_INT,MPI_ANY_SOURCE,ACK_TAG,MPI_COMM_WORLD,&status);
		        // printf("Coordinator received ack message from %d\n", status.MPI_SOURCE);
                break;    
            case DEPOSIT:
                MPI_send(curr->args,3,MPI_INT,curr_args[0],DEPOSIT,MPI_COMM_WORLD);
                break;        
            case DEPOSIT_OK:
                break;        
            case WITHDRAW:
                MPI_send(curr->args,3,MPI_INT,curr_args[0],DEPOSIT,MPI_COMM_WORLD);
                break;        
            case WITHDRAW_OK:break;
            case WITHDRAW_FAILED: break;
            default:
                for(int i = 1;i<=bank_offset;i++)
                    MPI_Send(&any,1,MPI_INT,i,TERMINATE,MPI_COMM_WORLD);
                break;
            }
            
            curr = curr->next;
        }

    }else
    if(rank <= bank_offset){//banks
        int myneighbors[10];
        int tail=0,flag=1,tail2=0,bank_balance=0;
        clients *c_head = NULL;
        clients *c_curr = NULL;
        
        while(flag){
            MPI_Recv(client_bank, 3, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            printf("[rank: %d]  Bank received message <%s>: [%d, %d, %d] from %d\n", rank,op2str(status.MPI_TAG),client_bank[0], client_bank[1], client_bank[2],status.MPI_SOURCE);
            switch(status.MPI_TAG){
                case 0:break;
                case CLIENT:
                    c_curr = c_create();
                    c_curr->id = client_bank[0];
                    c_curr->balance = 0;
                    c_insert(&c_head,c_curr);
                    MPI_Send(&any,1,MPI_INT,status.MPI_SOURCE,ACK_TAG,MPI_COMM_WORLD);

                    break;
                case NEIGHBOR:
                    
                    myneighbors[tail2++] = (rank == client_bank[0]) ? client_bank[1] : client_bank[0];
                    MPI_Send(&any,1,MPI_INT,status.MPI_SOURCE,ACK_TAG,MPI_COMM_WORLD);

                    break;
                case DEPOSIT:
                    
                    if((c_curr = get_client(c_head,client_bank[0])) == NULL) 
                        printf("ERROR : client not found\n");

                    bank_balance +=client_bank[1];

                    client_bank[0] = client_bank[1];
                    client_bank[1] = client_bank[2] = 0;
                    MPI_Send(client_bank,3,MPI_INT,status.MPI_SOURCE,DEPOSIT_OK,MPI_COMM_WORLD);
                    break;
                case WITHDRAW:
                    if((c_curr = get_client(c_head,client_bank[0])) == NULL) 
                        printf("ERROR : client not found\n");

                    if(c_curr && c_curr->balance >= client_bank[1]){
                        bank_balance -= client_bank[1];                    
                        c_curr->balance -= client_bank[1];

                        client_bank[0] = client_bank[1];client_bank[1] = client_bank[2] = 0;
                        MPI_Send(client_bank,3,MPI_INT,status.MPI_SOURCE,WITHDRAW_OK,MPI_COMM_WORLD);
                    }else{
                        client_bank[0] = client_bank[1];client_bank[1] = client_bank[2] = 0;
                        MPI_Send(client_bank,3,MPI_INT,status.MPI_SOURCE,WITHDRAW_FAILED,MPI_COMM_WORLD);

                    }
                    break;
                // case 5:
                case 6:
                default:flag=0;break;
            }
            //break;

       }
        // printf("\nbank <%d> clients:",rank);
        // c_curr = c_head;
        // while(c_curr != NULL){
        //     printf("<%d>",c_curr->id);
        //     c_curr = c_curr->next;
        // }
        // printf("\nbank <%d> neighbors:",rank);
        // for(int i = 0 ; i < tail2;i++){
        //     printf("<%d>,",myneighbors[i]);
        // }
    }else{//clients
        int mybank=0,flag=1,mybalance = 0;
        MPI_Recv(client_bank, 3 , MPI_INT, MPI_ANY_SOURCE, CLIENT, MPI_COMM_WORLD, &status);
        mybank = client_bank[1];
        // printf("[rank: %d]  Client received message <%s>: [%d, %d, %d] from %d\n", rank, op2str(status.MPI_TAG), client_bank[0], client_bank[1], client_bank[2], status.MPI_SOURCE);
        MPI_Send(&any,1,MPI_INT,status.MPI_SOURCE,ACK_TAG,MPI_COMM_WORLD);

        while(flag){
            MPI_Recv(client_bank, 3 , MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            switch(status.MPI_TAG){
                case 0: break;
                case DEPOSIT: 
                    MPI_Send(client_bank,3,MPI_INT,mybank,DEPOSIT,MPI_COMM_WORLD);
                    break;
                case DEPOSIT_OK:
                    mybalance+=client_bank[0];
                    MPI_Send(client_bank,3,MPI_INT,COORDINATOR,DEPOSIT_ACK,MPI_COMM_WORLD);
                    printf("CLIENT <%d> DEPOSITED <%d>\n",rank,client_bank[0]);
                    break;
                case WITHDRAW:
                    MPI_Send(client_bank,3,MPI_INT,mybank,WITHDRAW,MPI_COMM_WORLD);
                    break;
                case WITHDRAW_OK:
                    mybalance-=client_bank[0];
                    MPI_Send(client_bank,3,MPI_INT,COORDINATOR,WITHDRAW_ACK,MPI_COMM_WORLD);
                    printf("CLIENT <%d> WITHDRAW <%d> OK\n",rank,client_bank[0]);
                    break;
                case WITHDRAW_FAILED:
                    MPI_Send(client_bank,3,MPI_INT,COORDINATOR,WITHDRAW_ACK,MPI_COMM_WORLD);
                    printf("CLIENT <%d> WITHDRAW <%d> FAILED\n",rank,client_bank[0]);
                    break;
                default: printf("DEFAULT CASE SWITCH CLIENT\n");break;
                
            }

        }

        printf("\nCLIENT<%d> : mybank <%d>\n",rank,mybank);
    }


    MPI_Finalize();

    return 0;
}


