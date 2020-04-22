#include "faneuil.h"

#define TEST


/* 
Semafory:
1. stara se o to, zda je soudce v budove
2. stara se o kazdou zmenu citacu udalosti
3. stara se o cekani soudce na pristehovalce        VOLITELNE
4. stara se o vydani rozhodnuti o naturalizaci
5. stara se o ukonceni procesu soudce


*/
void judge_proc(struct Mem_Struct *mem_block) {

    #ifdef TEST
        double rdm = (double)((rand() % mem_block->borders[1]) / 1000.0);
        printf("Uspavam soudce poprve na dobu: %f\n", rdm);
        sleep(rdm);    //uspani na nahodnou dobu z intervalu 0 az borders[1]
    #endif

    #ifndef TEST
        sleep((double)((rand() % mem_block->borders[1]) / 1000.0)); 
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
    

    //zde je odemknuto, az jsou vsichni pristehovalci zaregistrovani
    
    //overeni podminky zda je pocet migrantu v budove roven poctu registrovanych
    sem_wait(&mem_block->sem_general_process);
    if(mem_block->imm_enter != mem_block->imm_registered) {
        mem_block->acc_ord++;
        printf("%d\t: JUDGE\t: waits fo imm\t: %d\t: %d\t: %d\n", mem_block->acc_ord, 
        mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside);
    }
    sem_post(&mem_block->sem_general_process);


    //tohle se musi udat jeste predtim nez si migranti zacnou vyzvedavat certifikaty
    sem_wait(&mem_block->sem_general_process);
    mem_block->acc_ord++;
    printf("%d\t: JUDGE\t: starts confirmation\t: %d\t: %d\t: %d\n", mem_block->acc_ord,
    mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside);  
    sem_post(&mem_block->sem_general_process);

    #ifdef TEST
        rdm = (double)((rand() % mem_block->borders[3]) / 1000.0);
        printf("Uspavam soudce podruhe na dobu: %f\n", rdm);
        sleep(rdm);
    #endif

    #ifndef TEST
        sleep((double)((rand() % mem_block->borders[3]) / 1000.0)); 
    #endif

    sem_wait(&mem_block->sem_general_process);
    mem_block->acc_ord++;
    mem_block->imm_enter = 0;
    mem_block->imm_registered = 0;
    printf("%d\t: JUDGE\t: ends confirmation\t: %d\t: %d\t: %d\n", mem_block->acc_ord,
    mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside);  
    sem_post(&mem_block->sem_general_process);
    sem_post(&mem_block->sem_confirmation);

    #ifdef TEST
        rdm = (double)((rand() % mem_block->borders[3]) / 1000.0);
        printf("Uspavam soudce potreti na dobu: %f\n", rdm);
        sleep(rdm);
    #endif

    #ifndef TEST
        sleep((double)((rand() % mem_block->borders[3]) / 1000.0)); 
    #endif

    sem_wait(&mem_block->sem_general_process);
    mem_block->acc_ord++;
    printf("%d\t: JUDGE\t: leaves\t: %d\t: %d\t: %d\n", mem_block->acc_ord,
    mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside);
    sem_post(&mem_block->sem_general_process);
    sem_post(&mem_block->sem_judge_inside);    //konec semaforu 1


    //pokud uz neni koho prijmout
    sem_wait(&mem_block->sem_judge_finish);          //semafor typu 5
    sem_wait(&mem_block->sem_general_process);
    mem_block->acc_ord++;
    printf("%d\t: JUDGE\t: finishes\n", mem_block->acc_ord);
    sem_post(&mem_block->sem_general_process);
    
}

void generate_immigrants(struct Mem_Struct *mem_block, int count) {

    int status;
    pid_t imm_pid;

    if(mem_block->borders[0] != 0) {
        #ifdef TEST
            double rdm = (double)((rand() % mem_block->borders[0]) / 1000.0);
            printf("Uspavam generator na dobu: %f\n", rdm);
            sleep(rdm);
        #endif

        #ifndef TEST
            sleep((double)((rand() % mem_block->borders[0]) / 1000.0)); 
        #endif
    }

    imm_pid = fork();    

    if (imm_pid == 0) {
        
        #ifdef TEST
            printf ("Immigrant cislo: %d s PID: %d a PPID: %d\n", count, getpid(), getppid());
        #endif

        immigrant_proc(mem_block);
        exit (EXIT_FAILURE);
    } else if (imm_pid < 0) {
        fprintf( stderr, "Fork failed!\n");
        exit(1);
    } else {

        #ifdef TEST
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

void immigrant_proc (struct Mem_Struct *mem_block) {

    
    //kriticka sekce, pristupuji ke sdilenym promenym
    
    sem_wait(&mem_block->sem_general_process);
    mem_block->acc_ord++; 
    mem_block->imm_ord++;
    printf("%d\t: IMM %d\t: starts\n", mem_block->acc_ord, mem_block->imm_ord);    //hned pri startu procesu
    sem_post(&mem_block->sem_general_process);

    //semafor 1 
    sem_wait(&mem_block->sem_judge_inside);
    //zde necekam, pokud neni soudce v budove
    sem_wait(&mem_block->sem_general_process);
    mem_block->acc_ord++;
    mem_block->imm_enter++;
    mem_block->imm_inside++;
    printf("%d\t: IMM %d\t: enters\t: %d\t: %d\t: %d\n", mem_block->acc_ord, mem_block->imm_ord,
    mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside);
    sem_post(&mem_block->sem_general_process);
    sem_post(&mem_block->sem_judge_inside);

    sem_wait(&mem_block->sem_general_process);
    mem_block->acc_ord++;
    mem_block->imm_registered++;
    printf("%d\t: IMM %d\t: checks\t: %d\t: %d\t: %d\n", mem_block->acc_ord, mem_block->imm_ord,
    mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside);
    sem_post(&mem_block->sem_general_process);

    //tohle se musi udat az potom, co soudce dokonci vydavani potvrzeni
    sem_wait(&mem_block->sem_confirmation);      //semafor typu 4
    //zde cekam na rozhodnuti soudce
    sem_wait(&mem_block->sem_general_process);
    mem_block->acc_ord++;
    printf("%d\t: IMM %d\t: wants certificate\t: %d\t: %d\t: %d\n", mem_block->acc_ord, mem_block->imm_ord,
    mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside);
    sem_post(&mem_block->sem_general_process);

    sleep((double)((rand() % mem_block->borders[2]) / 1000.0));       //vyzvedavani rozhodnuti
    
    sem_wait(&mem_block->sem_general_process);
    mem_block->acc_ord++;
    printf("%d\t: IMM %d\t: got certificate\t: %d\t: %d\t: %d\n", mem_block->acc_ord, mem_block->imm_ord,
    mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside);
    sem_post(&mem_block->sem_general_process);

    sem_post(&mem_block->sem_judge_finish);      //zahlasi soudci, ze uz ma certifikat

    //zde cekam az odejde soudce z budovy    
    sem_wait(&mem_block->sem_judge_inside);      //semafor typu 1
    sem_wait(&mem_block->sem_general_process);   //semafor typu 2
    mem_block->acc_ord++;
    mem_block->imm_enter--;
    printf("%d\t: IMM %d\t: leaves\t: %d\t: %d\t: %d\n", mem_block->acc_ord, mem_block->imm_ord,
    mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside);
    sem_post(&mem_block->sem_general_process);
    sem_post(&mem_block->sem_judge_inside);
    
    
}