#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>         //funkce fork, exec,...
#include <sys/wait.h>       //funkce waitpid
#include <semaphore.h>      //datovy typ sem

/* Deklarace pametove struktury */
struct Mem_Struct {
    sem_t sem_judge_inside;
    sem_t sem_general_process;
    sem_t sem_judge_finish;
    sem_t sem_confirmation;

    int borders[4];

    int acc_ord;            //volne cislo dalsi akce, inc 1
    int imm_ord;            //volne cislo pro dalsiho migranta, inc 1
    int imm_enter;          //pocet pristehovalcu, o kterych nebylo rozhodnuto, inc 0
    int imm_registered;     //pocet pristehovalcu, kteri se registrovali, inc 0
    int imm_inside;         //celkovy pocet pristehovalcu v budove, inc 0
};


/* Deklarace funkci */
void judge_proc(struct Mem_Struct *mem_block);
void generate_immigrants(struct Mem_Struct *mem_block, int count);
void immigrant_proc(struct Mem_Struct *mem_block);