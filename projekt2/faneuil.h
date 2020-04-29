#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>         //functions fork, exec,...
#include <sys/wait.h>       //waitpid function
#include <semaphore.h>      //sem_t data type

/* Deklarace pametove struktury */
struct Mem_Struct {

    FILE* fi;           //open file pointer

    sem_t sem_judge_inside;         //indicates if judge is inside a building
    sem_t sem_general_process;      //provides restrictions when multiple processes attempt to access a common source
    sem_t sem_imm_starts;           //causes judge to wait till at least one immigrant had started and entered the building
    sem_t sem_confirmation;         //indicates if confiramtion has ended and immigrants can start to pick ther certificates

    int process_count;      //number of immigrant processes which shall be generated
    int borders[4];         //time paramters for sleep functions

    int acc_ord;            //number of last action
    int imm_ord;            //number of the last immigrant
    int imm_enter;          //number if imm. which enterd the building
    int imm_registered;     //number of imm. who have had entered building and registered themselves
    int imm_inside;         //amount of all imm. in the building

    int imm_done;           //amount of imm. which already have picked the certificate
};


/* Deklarace funkci */
void judge_proc(struct Mem_Struct *mem_block,int itt, int prev_imm_done);
void finish_judge(struct Mem_Struct *mem_block);
void generate_immigrants(struct Mem_Struct *mem_block, int count);
void immigrant_proc(struct Mem_Struct *mem_block);