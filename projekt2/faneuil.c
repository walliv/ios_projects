#include "faneuil.h"

//#define TEST
//#define PROCESS_CREATION

/* 
Semafory:
1. stara se o to, zda je soudce v budove
2. stara se o kazdou zmenu citacu udalosti
3. stara se o cekani soudce na pristehovalce        VOLITELNE
4. stara se o vydani rozhodnuti o naturalizaci
5. stara se o ukonceni procesu soudce
*/

//////////-----------------SOUDCE------------------------/////////

void judge_proc(struct Mem_Struct *mem_block,int itt, int prev_imm_done) {

    int imm_waiting = 0;
    int iteration = 0;

    if(itt == 1)
        sem_wait(&mem_block->sem_imm_starts);

    srand(getpid());

    #ifdef TEST
        double rdm = (double)((rand() % (mem_block->borders[1] + 1)) / 1000.0);
        printf("Uspavam soudce poprve na dobu: %f\n", rdm);
        sleep(rdm);    //uspani na nahodnou dobu z intervalu 0 az borders[1]
    #endif

    #ifndef TEST
        sleep((double)((rand() % (mem_block->borders[1] + 1)) / 1000.0));
    #endif

    sem_wait(&mem_block->sem_judge_inside);          //semafor typu 1
    sem_wait(&mem_block->sem_general_process);       //semafor typu 2
    mem_block->acc_ord++;
    printf("%d\t: JUDGE\t: wants to enter\n", mem_block->acc_ord);
    sem_post(&mem_block->sem_general_process);       //konec semaforu 2

    sem_wait(&mem_block->sem_general_process);
    mem_block->acc_ord++;
    printf("%d\t: JUDGE\t: enters\t: %d\t: %d\t: %d\n", mem_block->acc_ord,
    mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside);
    sem_post(&mem_block->sem_general_process);
        
    //overeni podminky zda je pocet migrantu v budove roven poctu registrovanych
    sem_wait(&mem_block->sem_general_process);
    if(mem_block->imm_enter != mem_block->imm_registered) {
        mem_block->acc_ord++;
        printf("%d\t: JUDGE\t: waits fo imm\t: %d\t: %d\t: %d\n", mem_block->acc_ord, 
        mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside);
    }
    sem_post(&mem_block->sem_general_process);

    //[TODO]: zde chybi semafor pro cekani na pristehovalce

    //tohle se musi udat jeste predtim nez si migranti zacnou vyzvedavat certifikaty
    sem_wait(&mem_block->sem_general_process);
    mem_block->acc_ord++;
    printf("%d\t: JUDGE\t: starts confirmation\t: %d\t: %d\t: %d\n", mem_block->acc_ord,
    mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside);  
    sem_post(&mem_block->sem_general_process);

    #ifdef TEST
        rdm = (double)((rand() % (mem_block->borders[3] + 1)) / 1000.0);
        printf("Uspavam soudce podruhe na dobu: %f\n", rdm);
        sleep(rdm);
    #endif

    #ifndef TEST
        sleep((double)((rand() % (mem_block->borders[3] + 1)) / 1000.0));
    #endif

    sem_wait(&mem_block->sem_general_process);
    mem_block->acc_ord++;
    imm_waiting = mem_block->imm_registered;            //zde zapisuji specialne, STATICKA PROMENNA
    mem_block->imm_done += mem_block->imm_registered;
    mem_block->imm_enter = 0;
    mem_block->imm_registered = 0;
    printf("%d\t: JUDGE\t: ends confirmation\t: %d\t: %d\t: %d\n", mem_block->acc_ord,
    mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside);  
    sem_post(&mem_block->sem_general_process);

    #ifdef TEST
        int sem_val = 0;
        sem_getvalue(&mem_block->sem_confirmation, &sem_val);
        printf("Pocet migrantu cekajicich na rozhodnuti (semafor)%d, (promenna)%d\n",sem_val, imm_waiting);
    #endif

    for (int i = 0; i < imm_waiting; i++) {

        #ifdef TEST
            sem_getvalue(&mem_block->sem_confirmation, &sem_val);
            printf("Hodnota semaforu sem_confirmation: hodnota PRED %d, iterace: %d\n", sem_val, i);
        #endif

        sem_post(&mem_block->sem_confirmation);
        
        #ifdef TEST
            sem_getvalue(&mem_block->sem_confirmation, &sem_val);
            printf("Hodnota semaforu sem_confirmation: hodnota PO %d, iterace: %d\n", sem_val, i);
        #endif
    }

    #ifdef TEST
        rdm = (double)((rand() % (mem_block->borders[3] + 1)) / 1000.0);
        printf("Uspavam soudce potreti na dobu: %f\n", rdm);
        sleep(rdm);
    #endif

    #ifndef TEST
        sleep((double)((rand() % (mem_block->borders[3] + 1)) / 1000.0));
    #endif

    sem_wait(&mem_block->sem_general_process);
    mem_block->acc_ord++;
    printf("%d\t: JUDGE\t: leaves\t: %d\t: %d\t: %d\n", mem_block->acc_ord,
    mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside);

    if (mem_block->imm_done == prev_imm_done) //pokud soudce nikoho neprijal
        iteration = 1;
    else
        iteration = 0;

    prev_imm_done = mem_block->imm_done;

    sem_post(&mem_block->sem_general_process);
    sem_post(&mem_block->sem_judge_inside);    //konec semaforu 1

    //hodnota sem_val znaci pocet procesu pristehovalcu, ktere uz byly naturalizovany
    if(mem_block->imm_done < mem_block->process_count) {
        #ifdef TEST
            printf("Podminka pro opetovne zavolani soudce plnena...\n");
        #endif
        /*for (int i = 0; i < imm_waiting; i++) {
            #ifdef TEST
                sem_getvalue(&mem_block->sem_confirmation, &sem_val);
                printf("Uvolneni semaforu sem_confirmation, hodnota PRED %d, iterace %d\n",sem_val, i);
            #endif
            sem_wait(&mem_block->sem_confirmation);

            #ifdef TEST
                sem_getvalue(&mem_block->sem_confirmation, &sem_val);
                printf("Uvolneni semaforu sem_confirmation, hodnota PO %d, iterace %d\n",sem_val, i);
            #endif
        }*/
        #ifdef TEST
            printf("Volam opet funkci soudce...\n");
        #endif
        judge_proc(mem_block, iteration, prev_imm_done);
    } else {
        finish_judge(mem_block);
    }
   
}

//////////////////-----------KONEC SOUDCE---------------///////////////

void finish_judge(struct Mem_Struct *mem_block) {
    //pokud uz neni koho prijmout
    sem_wait(&mem_block->sem_general_process);
    mem_block->acc_ord++;
    printf("%d\t: JUDGE\t: finishes\n", mem_block->acc_ord);
    sem_post(&mem_block->sem_general_process);
}

///////--------------------GENERATOR------------------------/////////////

void generate_immigrants(struct Mem_Struct *mem_block, int count) {

    int status;
    pid_t imm_pid;

    srand(getpid());

    if(mem_block->borders[0] != 0) {
        #ifdef TEST
            double rdm = (double)((rand() % (mem_block->borders[0] + 1)) / 1000.0);
            printf("Uspavam generator na dobu: %f\n", rdm);
            sleep(rdm);
        #endif

        #ifndef TEST
            sleep((double)((rand() % (mem_block->borders[0] + 1)) / 1000.0)); 
        #endif
    }

    imm_pid = fork();    

    if (imm_pid == 0) {
        
        #ifdef PROCESS_CREATION
            printf ("Immigrant cislo: %d s PID: %d a PPID: %d\n", count, getpid(), getppid());
        #endif

        immigrant_proc(mem_block);
        exit (EXIT_FAILURE);
    } else if (imm_pid < 0) {
        fprintf( stderr, "Fork failed!\n");
        exit(1);
    } else {

        #ifdef PROCESS_CREATION
            printf("Proces generovani migrantu(ve funkci gen_imm) s PID: %d a PPID: %d\n", getpid(), getppid());
        #endif
        
        if(count > 1)
            generate_immigrants(mem_block, count - 1);

        if (waitpid (imm_pid, &status, 0) != imm_pid) {
            fprintf( stderr, "Child process termination failed!\n");
            exit(1);
        }

    }   
}

/////////------------------PRISTEHOVALEC---------------------//////////

void immigrant_proc (struct Mem_Struct *mem_block) {

    int imm_num = 0;
    srand(getpid());    
    
    sem_post(&mem_block->sem_imm_starts);

    sem_wait(&mem_block->sem_general_process);
    mem_block->acc_ord++; 
    mem_block->imm_ord++;
    imm_num = mem_block->imm_ord;
    printf("%d\t: IMM %d\t: starts\n", mem_block->acc_ord, imm_num);    //hned pri startu procesu
    sem_post(&mem_block->sem_general_process);

    //semafor 1 
    sem_wait(&mem_block->sem_judge_inside);
    //zde necekam, pokud neni soudce v budove
    sem_wait(&mem_block->sem_general_process);
    mem_block->acc_ord++;
    mem_block->imm_enter++;
    mem_block->imm_inside++;
    printf("%d\t: IMM %d\t: enters\t: %d\t: %d\t: %d\n", mem_block->acc_ord, imm_num,
    mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside);
    sem_post(&mem_block->sem_general_process);
    sem_post(&mem_block->sem_judge_inside);

    sem_wait(&mem_block->sem_general_process);
    mem_block->acc_ord++;
    mem_block->imm_registered++;
    printf("%d\t: IMM %d\t: checks\t: %d\t: %d\t: %d\n", mem_block->acc_ord, imm_num,
    mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside);
    sem_post(&mem_block->sem_general_process);

    //tohle se musi udat az potom, co soudce dokonci vydavani potvrzeni
    sem_wait(&mem_block->sem_confirmation);      //semafor typu 4
    //zde cekam na rozhodnuti soudce
    sem_wait(&mem_block->sem_general_process);
    mem_block->acc_ord++;
    printf("%d\t: IMM %d\t: wants certificate\t: %d\t: %d\t: %d\n", mem_block->acc_ord, imm_num,
    mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside);
    sem_post(&mem_block->sem_general_process);

    sleep((double)((rand() % (mem_block->borders[2] + 1)) / 1000.0));       //vyzvedavani rozhodnuti
    
    sem_wait(&mem_block->sem_general_process);
    mem_block->acc_ord++;
    printf("%d\t: IMM %d\t: got certificate\t: %d\t: %d\t: %d\n", mem_block->acc_ord, imm_num,
    mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside);
    sem_post(&mem_block->sem_general_process);

    //zde cekam az odejde soudce z budovy    
    sem_wait(&mem_block->sem_judge_inside);      //semafor typu 1
    sem_wait(&mem_block->sem_general_process);   //semafor typu 2
    mem_block->acc_ord++;
    mem_block->imm_inside--;
    printf("%d\t: IMM %d\t: leaves\t: %d\t: %d\t: %d\n", mem_block->acc_ord, imm_num,
    mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside);
    sem_post(&mem_block->sem_general_process);
    sem_post(&mem_block->sem_judge_inside);
}