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
		        printf("Coordinator received ack message from %d\n", status.MPI_SOURCE);
                MPI_Recv(&any,1,MPI_INT,MPI_ANY_SOURCE,ACK_TAG,MPI_COMM_WORLD,&status);
		        printf("Coordinator received ack message from %d\n", status.MPI_SOURCE);
                break;
            
            default:
                break;
            }
            
            curr = curr->next;
        }

    }else
    if(rank <= bank_offset){//banks
        int myclients[10];
        int tail=0;

		MPI_Recv(client_bank, 3, MPI_INT, MPI_ANY_SOURCE, CLIENT, MPI_COMM_WORLD, &status);
		printf("[rank: %d]  Bank received message: [%d, %d] from %d\n", rank, client_bank[0], client_bank[1], status.MPI_SOURCE);
        myclients[tail++] = client_bank[0];
        MPI_Send(&any,1,MPI_INT,status.MPI_SOURCE,ACK_TAG,MPI_COMM_WORLD);

    }else{//clients
        int mybank=0;
		MPI_Recv(client_bank, 3, MPI_INT, MPI_ANY_SOURCE, CLIENT, MPI_COMM_WORLD, &status);
		printf("[rank: %d]  Client received message: [%d, %d] from %d\n", rank, client_bank[0], client_bank[1], status.MPI_SOURCE);
        mybank = client_bank[1];
        MPI_Send(&any,1,MPI_INT,status.MPI_SOURCE,ACK_TAG,MPI_COMM_WORLD);
    }


    MPI_Finalize();

    return 0;
}


