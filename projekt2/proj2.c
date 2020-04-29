#include "faneuil.h"

#include <sys/types.h>      //special datatypes as sem_t and pid_t
#include <sys/shm.h>        //shared memory management
#include <fcntl.h>

//#define TEST        //uncomment when testing
//#define PROCESS_CREATION

int main (int argc, char *argv[])
{
    /*-------------------------BEGINNING TESTS---------------------*/
    #ifdef TEST
        printf("Number of paramteres: %d\n", argc);

        for(int i = 0; (i < argc) && (argc > 0); i++)
            printf("Parameter %d:  %s\n",i , argv[i]);
    #endif

    int borders[4];             //this does not change the whole time, it does not belong to critical section
    
    if (argc == 6) {          //test the correct nuber of input parameters
       
        // Array elements (command line arguments):
        // (1). PI number of immigrant processes that will be generated
        // 0(2). IG maximum duration after which new immigrant process is generated
        // 1(3). JG maximum duration after which the judge eneters the building again
        // 2(4). IT max duration which inticates how long does the immigrant pick the certificate
        // 3(5). JT maximum amount of time when judge decides about naturalization
        
        int act = 0; 
        int pcess_count = atoi(argv[1]);

        if (pcess_count < 1) {                   //first parameter has different condition than others
            fprintf (stderr, "Parameter 1 out of range, value must be greater than 1 or equal!\n");
            exit(1);
        }

        #ifdef TEST
            printf( "1st parameter written as: %d\n",pcess_count);
        #endif

        for ( int i = 2; i < argc; i++ ) {

            if (sscanf(argv[i], "%d", &act) != 1) {           //tests if input arguments are integers
                fprintf( stderr, "Parameter %d is not an ineger!\n", i + 1);
                exit(EXIT_FAILURE);
            }            

            act = atoi(argv[i]);
            
            if ((act >= 0) && (act <= 2000)) {      //if it is in range then it can be added into array

                borders[i - 2] = act;
                
            } else {
                fprintf( stderr, "Parameter %d out of range, value must be between 0 and 2000!\n", i + 1);
                exit(EXIT_FAILURE);
            }

            #ifdef TEST
                printf( "Parameter %d written as: %d\n", i, borders[i - 2]);
            #endif
        }
        
        #ifdef TEST
            printf("Beginning tests successful...\n");
        #endif

        /*-------------------------------SHARED MEMORY ALLOCATION--------------------------------*/

        int shm_id = shmget(IPC_PRIVATE, sizeof(struct Mem_Struct), 0666 | IPC_CREAT);

        if(shm_id == -1) {           //in case if allocation failed
            fprintf( stderr, "Shared memory allocation failed, using shmget.\n");
            exit(EXIT_FAILURE);
        }

        #ifdef TEST
            printf("Shared memory identifier created.\n");
        #endif


        struct Mem_Struct* mem_block = (struct Mem_Struct*)shmat(shm_id, NULL, 0);

        if(mem_block == (struct Mem_Struct *)-1) {                  //in case mapping fails
            fprintf( stderr, "Shared memory allocation failed, using shmat!\n");
            exit(EXIT_FAILURE);
        }

        #ifdef TEST
            printf("Shared memory attached\n");
        #endif

        /*-------------------------------OPEN FILE FOR OUTPUT------------------------------------*/

        mem_block->fi = fopen("./proj2.out", "w");

        if (mem_block->fi == NULL) {
            fprintf( stderr, "Failed to open file!\n");
            exit(EXIT_FAILURE);
        }
        

        /*-------------------------UNNAMED SEMAPHORES INITIALIZATION--------------------------*/
    
        int ret1 = sem_init(&mem_block->sem_judge_inside, 1, 1);            //MUTEX
        int ret2 = sem_init(&mem_block->sem_imm_starts, 1, 0);            //SIGNAL
        int ret3 = sem_init(&mem_block->sem_general_process, 1, 1);         //MUTEX
        int ret4 = sem_init(&mem_block->sem_confirmation, 1, 0);            //SIGNAL

        if(ret1 == -1 || ret2 == -1 || ret3 == -1 || ret4 == -1) {            //if semaphore creation fails
            fprintf( stderr, "Failed to create semaphore!\n");
            exit(EXIT_FAILURE);
        }

        #ifdef TEST
            printf("Unnamed semaphores initalized...\n");
        #endif

        mem_block->process_count = pcess_count;

        for(int i = 0; i < 4; i++)
            mem_block->borders[i] = borders[i];

        mem_block->acc_ord = 0;
        mem_block->imm_ord = 0;
        mem_block->imm_enter = 0;
        mem_block->imm_registered = 0;
        mem_block->imm_inside = 0;
        mem_block->imm_done = 0;

        #ifdef TEST
            printf("Memory structure variables were successfully initialized\n");
        #endif

        #ifdef TEST
            printf("Allocation using shmget, shmat:\n");
            printf("Variables in \"borders\" array:");
            for (int i = 0; i < 5; i++)
                printf("%d: %d;", i, mem_block->borders[i]);  
            printf("\nVariables: acc_ord = %d, imm_ord = %d, imm_enter = %d, imm_registered = %d, imm_inside = %d, imm_done = %d\n", 
            mem_block->acc_ord, mem_block->imm_ord, mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside, mem_block->imm_done);
        #endif    

        /*----------------------FIRST PROCESS INITIALIZATION-----------------------*/

        int status;         //process exit status
        pid_t judge_pid, gen_pid;
        
        judge_pid = fork();         //following code is duplicated
        
        if (judge_pid == 0) {         //children process = judge

            #ifdef PROCESS_CREATION
                printf("Judge process with PID: %d and PPID: %d\n", getpid(), getppid());
            #endif

            judge_proc(mem_block, 0, 0);
            exit (EXIT_FAILURE);           //exit status must be failure, otherwise the children become parents of itself
       
        } else if (judge_pid < 0) {        //fork failed
            
            fprintf( stderr, "Fork failed!\n");
            exit(EXIT_FAILURE);

        } else {         //parent process = main funciton, return value is the PID of its child

            gen_pid = fork();
            if(gen_pid == 0) {          //here we create the immigrant process generator

                #ifdef PROCESS_CREATION
                    printf("Generator (main function) with PID: %d and PPID: %d\n", getpid(), getppid());
                #endif

                generate_immigrants(mem_block, pcess_count);
                exit (EXIT_FAILURE);
            
            } else if (gen_pid < 0) {

                fprintf( stderr, "Fork failed!\n");
                exit(EXIT_FAILURE);             //this ends parent process
            
            } else {        //parent process = main function
                
                if (waitpid (gen_pid, &status, 0) != gen_pid) {
                    fprintf( stderr, "Child process termination failed!\n");
                    exit(EXIT_FAILURE);
                }

            }            

            #ifdef PROCESS_CREATION
                    printf("Main function with PID: %d and PPID: %d\n", getpid(), getppid());
            #endif

            if (waitpid (judge_pid, &status, 0) != judge_pid) {     //in case waitpid function fails
                fprintf( stderr, "Child process termination failed!\n");
                exit(EXIT_FAILURE);
            }
        }

        /*-------------------------------Final cleaning tasks------------------------------- */

        if(fclose(mem_block->fi) == -1) {
            fprintf( stderr, "Failed to close file!\n");
            exit(EXIT_FAILURE);
        }

        #ifdef TEST
            printf("All processes ended successfully\n");
        #endif

        if(sem_destroy(&mem_block->sem_judge_inside) == -1 || sem_destroy(&mem_block->sem_imm_starts) == -1 || 
        sem_destroy(&mem_block->sem_confirmation) == -1 || sem_destroy(&mem_block->sem_general_process) == -1) {

            fprintf( stderr, "Failed to destroy semaphores!\n");
            exit(EXIT_FAILURE);

        }

        #ifdef TEST
            printf("Semaphores destroyed\n");
        #endif

        if (shmdt(mem_block) == -1) {

            fprintf( stderr, "Failed to detach memory segment!\n");
            exit(EXIT_FAILURE);

        }

        #ifdef TEST
            printf("Shared memory detached\n");
        #endif

        if (shmctl(shm_id, IPC_RMID, NULL) == -1) {

            fprintf( stderr, "Failed to remove memory segment!\n");
            exit(EXIT_FAILURE);

        }

        #ifdef TEST
            printf("Shared memory removed!\n");
        #endif
     

    } else {

        fprintf( stderr, "Wrong number of parameters, 5 expected!\n");     //zde musi byt fprintf protoze tisknu do souboru
        exit(EXIT_FAILURE);

    }
    
    return 0;       //sem se program nesmi dostat
}  