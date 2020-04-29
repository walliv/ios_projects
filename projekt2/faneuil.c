#include "faneuil.h"

//#define TEST
//#define PROCESS_CREATION

//////////-----------------JUDGE------------------------/////////

void judge_proc(struct Mem_Struct *mem_block,int itt, int prev_imm_done) {

    int imm_waiting = 0;        //specifies the number of current registered immigrants through this function
    int iteration = 0;          //specifies the number of iterations in which judge works without any immigrants inside

    if(itt == 1)
        sem_wait(&mem_block->sem_imm_starts);

    srand(getpid());

    #ifdef TEST
        double rdm = (double)((rand() % (mem_block->borders[1] + 1)) / 1000.0);
        printf("Putting judge into sleep for %f seconds\n", rdm);
        sleep(rdm);
    #endif

    #ifndef TEST
        sleep((double)((rand() % (mem_block->borders[1] + 1)) / 1000.0));
    #endif

    sem_wait(&mem_block->sem_judge_inside);   
    sem_wait(&mem_block->sem_general_process);

        mem_block->acc_ord++;
        fprintf( mem_block->fi, "%d\t: JUDGE\t: wants to enter\n", mem_block->acc_ord);
        fflush(mem_block->fi);

    sem_post(&mem_block->sem_general_process);

    sem_wait(&mem_block->sem_general_process);

        mem_block->acc_ord++;
        fprintf( mem_block->fi, "%d\t: JUDGE\t: enters\t: %d\t: %d\t: %d\n", mem_block->acc_ord,
        mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside);
        fflush(mem_block->fi);

    sem_post(&mem_block->sem_general_process);
        
    //testsif number of immigrants which enterd is same as amount of registered
    sem_wait(&mem_block->sem_general_process);

        if(mem_block->imm_enter != mem_block->imm_registered) {
            mem_block->acc_ord++;
            fprintf( mem_block->fi, "%d\t: JUDGE\t: waits fo imm\t: %d\t: %d\t: %d\n", mem_block->acc_ord, 
            mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside);
            fflush(mem_block->fi);
        }

    sem_post(&mem_block->sem_general_process);

    sem_wait(&mem_block->sem_general_process);

        mem_block->acc_ord++;
        fprintf( mem_block->fi, "%d\t: JUDGE\t: starts confirmation\t: %d\t: %d\t: %d\n", mem_block->acc_ord,
        mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside);
        fflush(mem_block->fi);  

    sem_post(&mem_block->sem_general_process);

    #ifdef TEST
        rdm = (double)((rand() % (mem_block->borders[3] + 1)) / 1000.0);
        printf("Putting judge for second time into sleep for %f seconds\n", rdm);
        sleep(rdm);
    #endif

    #ifndef TEST
        sleep((double)((rand() % (mem_block->borders[3] + 1)) / 1000.0));
    #endif

    sem_wait(&mem_block->sem_general_process);

        mem_block->acc_ord++;
        imm_waiting = mem_block->imm_registered;            //special variable
        mem_block->imm_done += mem_block->imm_registered;
        mem_block->imm_enter = 0;
        mem_block->imm_registered = 0;

        fprintf( mem_block->fi, "%d\t: JUDGE\t: ends confirmation\t: %d\t: %d\t: %d\n", mem_block->acc_ord,
        mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside);

        fflush(mem_block->fi);

    sem_post(&mem_block->sem_general_process);

    #ifdef TEST
        int sem_val = 0;
        sem_getvalue(&mem_block->sem_confirmation, &sem_val);
        printf("Number of immigrants waiting for certificate (semaphore)%d, (variable)%d\n",sem_val, imm_waiting);
    #endif

    for (int i = 0; i < imm_waiting; i++) {

        #ifdef TEST
            sem_getvalue(&mem_block->sem_confirmation, &sem_val);
            printf("Value of sem_confirmation: BEFORE: %d, iteration: %d\n", sem_val, i);
        #endif

        sem_post(&mem_block->sem_confirmation);
        
        #ifdef TEST
            sem_getvalue(&mem_block->sem_confirmation, &sem_val);
            printf("Value of sem_confirmation: AFTER %d, iteration: %d\n", sem_val, i);
        #endif
    }

    #ifdef TEST
        rdm = (double)((rand() % (mem_block->borders[3] + 1)) / 1000.0);
        printf("Putting judge for third time into sleep for %f seconds\n", rdm);
        sleep(rdm);
    #endif

    #ifndef TEST
        sleep((double)((rand() % (mem_block->borders[3] + 1)) / 1000.0));
    #endif

    sem_wait(&mem_block->sem_general_process);
        mem_block->acc_ord++;
        fprintf( mem_block->fi, "%d\t: JUDGE\t: leaves\t: %d\t: %d\t: %d\n", mem_block->acc_ord,
        mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside);
        fflush(mem_block->fi);

        if (mem_block->imm_done == prev_imm_done)       //when judge had accepted nobody
            iteration = 1;
        else
            iteration = 0;

        prev_imm_done = mem_block->imm_done;

    sem_post(&mem_block->sem_general_process);

    sem_post(&mem_block->sem_judge_inside);

    if(mem_block->imm_done < mem_block->process_count) {

        #ifdef TEST
            printf("Calling judge function again\n");
        #endif

        judge_proc(mem_block, iteration, prev_imm_done);
    } else {
        finish_judge(mem_block);
    }
   
}

//////////////////-----------END OF JUDGE---------------///////////////

void finish_judge(struct Mem_Struct *mem_block) {
    //after all immigrants were accepted
    sem_wait(&mem_block->sem_general_process);
        mem_block->acc_ord++;
        fprintf( mem_block->fi, "%d\t: JUDGE\t: finishes\n", mem_block->acc_ord);
        fflush(mem_block->fi);
    sem_post(&mem_block->sem_general_process);
}

///////--------------------IMMIGRANT PROCESS GENERATOR------------------------/////////////

void generate_immigrants(struct Mem_Struct *mem_block, int count) {

    int status;
    pid_t imm_pid;

    srand(getpid());

    if(mem_block->borders[0] != 0) {
        #ifdef TEST
            double rdm = (double)((rand() % (mem_block->borders[0] + 1)) / 1000.0);
            printf("Suspend generator for %f seconds\n", rdm);
            sleep(rdm);
        #endif

        #ifndef TEST
            sleep((double)((rand() % (mem_block->borders[0] + 1)) / 1000.0)); 
        #endif
    }

    imm_pid = fork();    

    if (imm_pid == 0) {
        
        #ifdef PROCESS_CREATION
            printf ("Immigrant %d with PID: %d and PPID: %d\n", count, getpid(), getppid());
        #endif

        immigrant_proc(mem_block);
        exit (EXIT_FAILURE);
    } else if (imm_pid < 0) {
        fprintf( stderr, "Fork failed!\n");
        exit(1);
    } else {
        
        if(count > 1)
            generate_immigrants(mem_block, count - 1);

        if (waitpid (imm_pid, &status, 0) != imm_pid) {
            fprintf( stderr, "Child process termination failed!\n");
            exit(1);
        }
    }   
}

/////////------------------IMMIGRANT---------------------//////////

void immigrant_proc (struct Mem_Struct *mem_block) {

    int imm_num = 0;            //immigrant identifier
    srand(getpid());    
    
    sem_post(&mem_block->sem_imm_starts);

    sem_wait(&mem_block->sem_general_process);

        mem_block->acc_ord++; 
        mem_block->imm_ord++;
        imm_num = mem_block->imm_ord;
        fprintf( mem_block->fi, "%d\t: IMM %d\t: starts\n", mem_block->acc_ord, imm_num);    //immediately after process starts
        fflush(mem_block->fi);

    sem_post(&mem_block->sem_general_process);

    sem_wait(&mem_block->sem_judge_inside);    
        sem_wait(&mem_block->sem_general_process);

            mem_block->acc_ord++;
            mem_block->imm_enter++;
            mem_block->imm_inside++;
            fprintf( mem_block->fi, "%d\t: IMM %d\t: enters\t: %d\t: %d\t: %d\n", mem_block->acc_ord, imm_num,
            mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside);
            fflush(mem_block->fi);

        sem_post(&mem_block->sem_general_process);
    sem_post(&mem_block->sem_judge_inside);

    sem_wait(&mem_block->sem_general_process);

        mem_block->acc_ord++;
        mem_block->imm_registered++;
        fprintf( mem_block->fi, "%d\t: IMM %d\t: checks\t: %d\t: %d\t: %d\n", mem_block->acc_ord, imm_num,
        mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside);
        fflush(mem_block->fi);

    sem_post(&mem_block->sem_general_process);

    //this have to happen after judge finished confirmation
    sem_wait(&mem_block->sem_confirmation);
    //wait for judges decisiom
    sem_wait(&mem_block->sem_general_process);
    
        mem_block->acc_ord++;
        fprintf( mem_block->fi, "%d\t: IMM %d\t: wants certificate\t: %d\t: %d\t: %d\n", mem_block->acc_ord, imm_num,
        mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside);
        fflush(mem_block->fi);

    sem_post(&mem_block->sem_general_process);

    sleep((double)((rand() % (mem_block->borders[2] + 1)) / 1000.0));       //picking up the certificate
    
    sem_wait(&mem_block->sem_general_process);

        mem_block->acc_ord++;
        fprintf( mem_block->fi, "%d\t: IMM %d\t: got certificate\t: %d\t: %d\t: %d\n", mem_block->acc_ord, imm_num,
        mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside);
        fflush(mem_block->fi);

    sem_post(&mem_block->sem_general_process);

    //wait till judge leaves the building   
    sem_wait(&mem_block->sem_judge_inside); 
        sem_wait(&mem_block->sem_general_process);

            mem_block->acc_ord++;
            mem_block->imm_inside--;
            fprintf( mem_block->fi, "%d\t: IMM %d\t: leaves\t: %d\t: %d\t: %d\n", mem_block->acc_ord, imm_num,
            mem_block->imm_enter, mem_block->imm_registered, mem_block->imm_inside);
            fflush(mem_block->fi);

        sem_post(&mem_block->sem_general_process);
    sem_post(&mem_block->sem_judge_inside);
}