#include "faneuil.h"

#include <sys/types.h>
#include <sys/shm.h>        //funkce spravy sdielene pameti
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define TEST        //pro testovaci ucely odkomentovat

//#define MMAN
//#define MALLOC

int main (int argc, char *argv[])
{
    #ifdef TEST
        printf("Pocet parametru: %d\n", argc);

        for(int i = 0; (i < argc) && (argc > 0); i++)
            printf("%d.parametr: %s\n",i , argv[i]);
    #endif

    int borders[4];             //toto se za celou dobu nezmeni, NEPATRI DO KRITICKE SEKCE
    
    if (argc == 6) {          //testuje spravny pocet parametru prikaove radky
       
        // Prvky pole:
        // 1. PI pocet procesu karegorie pristehovalcu
        // 2. IG max hodnota doby v milis, po ktere je generovan proces IMM
        // 3. JG max hodnota doby, po ktere JUDGE vstoupi do budovy
        // 4. IT max hodnota doby, kdy IMM vyzvedava certifikat
        // 5. JT max hodnota doby, kdy se JUDGE rozhoduje o vydani
        
        int act = 0; 
        int pcess_count = atoi(argv[1]);

        if (pcess_count < 1) {                   //prvni parametr ma jinou podminku
            fprintf (stderr, " 1st parameter out of range, value must be greater than 1 or equal!\n");
            exit(1);
        }

        #ifdef TEST
            printf( "1.parametr zapsan jako: %d\n",pcess_count);
        #endif

        for ( int i = 2; i < argc; i++ ) {

            act = atoi(argv[i]);
            
            if ((act >= 0) && (act <= 2000)) {      //pokud je v rozsahu, zapisu do pole borders

                borders[i - 2] = act;
                
            } else {
                fprintf( stderr, "%d.parameter out of range, value must be between 0 and 2000!\n", i);
                exit(1);
            }

            #ifdef TEST
                printf( "%d.parametr zapsan jako: %d\n", i, borders[i - 2]);
            #endif
        }
        
        #ifdef TEST
            printf("Uvodni testy uspely, pokracuji v kodu...\n");
        #endif

        /* Alokace sdilene pameti */

        int shm_id = shmget(IPC_PRIVATE, sizeof(struct Mem_Struct), 0666 | IPC_CREAT);

        if(shm_id == -1) {           //pro pripad, ze by alokace vratila chybu
            fprintf( stderr, "Shared memory allocation failed, using shmget.\n");
            exit(1);
        }

        #ifdef TEST
            printf("Vytvoreno pole sdilene pameti.\n");
        #endif


        struct Mem_Struct* mem_block = (struct Mem_Struct*)shmat(shm_id, NULL, 0);

        if(mem_block == (struct Mem_Struct *)-1) {                  //v pripade, ze selze namapovani adresoveho bloku
            //fprintf( stderr, "Shared memory allocation failed, using shmat!\n");
            perror("shmat");
            exit(1);
        }

        #ifdef TEST
            printf("Sdilena pamet pripojena\n");
        #endif
        /*
        mem_block->sem_judge_inside = sem_open("/semtex", O_EXCL | O_CREAT, 0666, 1);
        mem_block->sem_general_process = sem_open("/crazy", O_EXCL | O_CREAT, 0666, 1);
        mem_block->sem_judge_finish = sem_open("/monster", O_EXCL | O_CREAT, 0666, ((-pcess_count) + 2));
        mem_block->sem_confirmation = sem_open("/boost", O_EXCL | O_CREAT, 0666, 0);
        */

        int ret1 = sem_init(&mem_block->sem_judge_inside, 1, 1);
        int ret2 = sem_init(&mem_block->sem_judge_finish, 1, ((-pcess_count) + 2));
        int ret3 = sem_init(&mem_block->sem_general_process, 1, 1);
        int ret4 = sem_init(&mem_block->sem_confirmation, 1, 0);

        //if(mem_block->sem_judge_inside == SEM_FAILED || mem_block->sem_general_process == SEM_FAILED ||
        //mem_block->sem_judge_finish == SEM_FAILED || mem_block->sem_confirmation == SEM_FAILED){  
        if(ret1 == -1 || ret2 == -1 || ret3 == -1 || ret4 == -1) {            //v pripade, ze selze vytvoreni semaforu
            //fprintf( stderr, "Failed to create semaphore! Error: %s\n", strerror(errno));
            perror("sem_init");
            exit(1);
        }

        #ifdef TEST
            printf("Semafor inicializovan...\n");
        #endif

        for(int i = 0; i < 4; i++)
            mem_block->borders[i] = borders[i];

        mem_block->acc_ord = 0;
        mem_block->imm_ord = 0;
        mem_block->imm_enter = 0;
        mem_block->imm_registered = 0;
        mem_block->imm_inside = 0;

        #ifdef TEST
            printf("Promenne hlavni struktury inicializovany\n");
        #endif


        #ifdef TEST
            printf("Alokace shmget, shmat:\n");
            printf("Promenne borders:");
            for (int i = 0; i < 5; i++)
                printf("%d: %d;", i, mem_block->borders[i]);  
            printf("\nPromenne: acc_ord = %d, imm_ord = %d, imm_enter = %d, imm_registered = %d, imm_inside = %d\n", 
            mem_block->acc_ord, mem_block->imm_ord, mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside);
        #endif    

        #ifdef MMAN     //tohle nefunguje, chybi funkce ftruncate a shm_open
            struct Mem_Struct *mem_block = (struct Mem_Struct*)mmap(NULL, sizeof(struct Mem_Struct), (PROT_READ | PROT_WRITE), MAP_SHARED, -1, 0);

            memcpy(mem_block, (void *) mem, sizeof(struct Mem_Struct));

            if(mem_block == MAP_FAILED) {           //pro pripad, ze by alokace
                fprintf( stderr, "Shared memory allocation failed, using mmap.\n");
                exit(1);
            }

            for(int i = 0; i < 5; i++)
                mem_block->borders[i] = borders[i];

            mem_block->acc_ord = 1;
            mem_block->imm_ord = 1;
            mem_block->imm_enter = 0;
            mem_block->imm_registered = 0;
            mem_block-> imm_inside = 0;

            #ifdef TEST
                printf("Alokace mmap:\n");
                printf("Promenne borders:");
                for (int i = 0; i < 5; i++)
                    printf("%d: %d;", i, mem_block->borders[i]);  

                printf("\nPromenne: acc_ord = %d, imm_ord = %d, imm_enter = %d, imm_registered = %d, imm_inside = %d\n", 
                mem_block->acc_ord, mem_block->imm_ord, mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside);
            #endif    
        #endif

        int status;         //informace o statusu ukonceni daneho potomka
        pid_t judge_pid, gen_pid;
        
        judge_pid = fork();         //za timto kodem se vse duplikuje
        
        if (judge_pid == 0) {         //proces potomka = proces soudce

            #ifdef TEST
                printf("Proces soudce s PID: %d a PPID: %d\n", getpid(), getppid());
            #endif

            //while(mem_block->imm_ord <= pcess_count)    //pokud nebylo o vsech rozhodnuto, spoustim soudce znovu
                judge_proc(mem_block);

            exit (EXIT_FAILURE);           //zde to musi koncit exit, protoze jinak by se proces stal sam sobe rodicem
        } else if (judge_pid < 0) {        //fork selhal
            fprintf( stderr, "Fork failed!\n");
            exit(EXIT_FAILURE);
        } else {         //proces rodice = hlavni proces tohoto programu, navratova hodnota fork je PID potomka

            gen_pid = fork();
            if(gen_pid == 0) {          //zde vytvorim generator pristehovalcu

                #ifdef TEST
                    printf("Proces generovani migrantu(funkce main) s PID: %d a PPID: %d\n", getpid(), getppid());
                #endif

                generate_immigrants(mem_block, pcess_count);
                exit (EXIT_FAILURE);
            } else if (gen_pid < 0) {
                fprintf( stderr, "Fork failed!\n");
                exit(EXIT_FAILURE);             //tohle ukonci proces a vsechno vycisti
            } else {        //rodicovsky proces = hlavni proces tototo programu
                
                //[TODO]: zde pokracovat, prace rodice pri dokonceni procesu generatoru

                #ifdef TEST
                    printf("Hlavni proces programu s PID: %d a PPID: %d\n", getpid(), getppid());
                #endif

                if (waitpid (gen_pid, &status, 0) != gen_pid) {
                    fprintf( stderr, "Child process termination failed!\n");
                    exit(EXIT_FAILURE);
                }

            }            
                
            //[TODO]: zde pokracovat, prace rodice pri dokonceni procesu soudce

            #ifdef TEST
                printf("Hlavni proces programu s PID: %d a PPID: %d\n", getpid(), getppid());
            #endif

            if (waitpid (judge_pid, &status, 0) != judge_pid) {     //v pripade chyby funkce waitpid
                fprintf( stderr, "Child process termination failed!\n");
                exit(EXIT_FAILURE);
            }
        }

    /* Zaverecne uklizeci prace */

    if(sem_destroy(&mem_block->sem_judge_inside) == -1 || sem_destroy(&mem_block->sem_judge_finish) == -1 || 
    sem_destroy(&mem_block->sem_confirmation) == -1 || sem_destroy(&mem_block->sem_general_process) == -1) {
        perror("sem_destroy");
        exit(1);
    }

    /*
    if(sem_close(mem_block->sem_judge_inside) == -1 || sem_close(mem_block->sem_general_process) == -1 || 
    sem_close(mem_block->sem_judge_finish) == -1 || sem_close(mem_block->sem_confirmation) == -1) {
        fprintf( stderr, "Failed to close semaphore!\n");
        exit(EXIT_FAILURE);
    }

     if(sem_unlink("midori") == -1 || sem_unlink("falkon") == -1 || sem_unlink("chrome") == -1 || 
     sem_unlink("firefox") == -1) {
        fprintf( stderr, "Failed to remove semaphore!\n");
        exit(EXIT_FAILURE);
    }
    */

    if (shmdt(mem_block) == -1) {
        fprintf( stderr, "Failed to detach memory segment!\n");
        exit(EXIT_FAILURE);
    }

    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        fprintf( stderr, "Failed to remove memory segment!\n");
        exit(EXIT_FAILURE);
    }
    

    //[TODO]: zde jeste chybi volani funkce shmctl pro smazani bloku sdilene pameti
     

    } else {
        fprintf( stderr, "Wrong number of parameters, 5 expected!\n");     //zde musi byt fprintf protoze tisknu do souboru
        exit(EXIT_FAILURE);
    }
    
    return 0;       //sem se program nesmi dostat
}