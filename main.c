#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <unistd.h>

#define DELIMETERS " "
#define ACK_TAG 0
#define CLIENT 1
#define NEIGHBOR 2


int main(int argc, char *argv[]){
	int rank, world_size, bank_offset,i; 
    char *file;

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
    if(!rank){ //coordinator
        //opens testfile and reads
        char *line = NULL;
        char *command = (char*)malloc(128);
        int opt_arr[3] = {0};
        char *token;
        size_t len = 0;
        ssize_t read = 0;
        FILE *fp = fopen(argv[2],"r");

        if(!fp){
            MPI_Finalize();
            printf("Error opening testfile <%s>\n",argv[2]);
            exit(EXIT_FAILURE); 
        }

        while ((read = getline(&line, &len, fp)) != -1) {
            printf("%s\n", line);

            token = strtok(line, DELIMETERS);
            i=0;

            while( token != NULL ) {
                if(i==0){
                    strncpy( command,token,strlen(token));
                    printf("%s\n",command);
                } 
                else{
                    opt_arr[i-1] = atoi(token);
                }
                printf("%d:%d:%d\n",opt_arr[0],opt_arr[1],opt_arr[2]);
                
                i++;
                token = strtok(NULL, DELIMETERS);
            }

            client_bank[0] = opt_arr[0];
            client_bank[1] = opt_arr[1];

            MPI_Send(client_bank,2,MPI_INT,client_bank[0],CLIENT,MPI_COMM_WORLD);
            MPI_Send(client_bank,2,MPI_INT,client_bank[1],CLIENT,MPI_COMM_WORLD);
        }
    }else
    if(rank <= bank_offset){//banks

		MPI_Recv(client_bank, 2, MPI_INT, MPI_ANY_SOURCE, CLIENT, MPI_COMM_WORLD, &status);
		printf("[rank: %d]  Bank received message: [%d, %d] from %d\n", rank, client_bank[0], client_bank[1], status.MPI_SOURCE);

    }else{//clients

		MPI_Recv(client_bank, 2, MPI_INT, MPI_ANY_SOURCE, CLIENT, MPI_COMM_WORLD, &status);
		printf("[rank: %d]  Client received message: [%d, %d] from %d\n", rank, client_bank[0], client_bank[1], status.MPI_SOURCE);

    }


    MPI_Finalize();

    return 0;
}


