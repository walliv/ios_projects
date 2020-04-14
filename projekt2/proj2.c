#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>         //funkce fork, exec,...
#include <semaphore.h>      //datovy typ sem_t a funkce pro spravu semaforu
#include <sys/types.h>      //datove typy pid_t
#include <sys/wait.h>       //funkce waitpid

#define TEST        //pro testovaci ucely odkomentovat

void judge_proc();
int generate_immigrants(int count);
void immigrant_proc();

int borders[5];     //toto se za celou dobu nezmeni, NEPATRI DO KRITICKE SEKCE
int acc_ord = 1;    //volne cislo dalsi akce
int imm_ord = 1;    //volne cislo pro dalsiho migranta

sem_t semtex;       //vitezoslavny semafor

int main (int argc, char *argv[])
{
    #ifdef TEST// TESTOVACI

        printf("Pocet parametru: %d\n", argc);

        for(int i = 0; (i < argc) && (argc > 0); i++) {
            printf("%d.parametr: %s\n",i , argv[i]);
        }

    #endif//Konec testovacich radku
    
    if (argc == 6) {          //testuje spravny pocet parametru prikaove radky
       
        // Prvky pole:
        // 1. PI pocet procesu karegorie pristehovalcu
        // 2. IG max hodnota doby v milis, po ktere je generovan proces IMM
        // 3. JG max hodnota doby, po ktere JUDGE vstoupi do budovy
        // 4. IT max hodnota doby, kdy IMM vyzvedava certifikat
        // 5. JT max hodnota doby, kdy se JUDGE rozhoduje o vydani
        
        int act = 0;

        for ( int i = 1; i < argc; i++ ) {

            act = atoi(argv[i]);
            
            if ((i == 1) && (act >= 1)) {                   //prvni parametr ma jinou podminku
            
                borders[i - 1] = act;
            
            } else if ((i == 1) && (act < 1)) {
                fprintf (stderr, "%d.parameter out of range, value must be greater than 1 or equal!\n", i);
                exit(1);
            } else {
                if ((act >= 0) && (act <= 2000)) {      //pokud je v rozsahu, zapisu do pole borders
                
                    borders[i - 1] = act;
                
                } else {
                    fprintf( stderr, "%d.parameter out of range, value must be between 0 and 2000!\n", i);
                    exit(1);
               }
            }

            #ifdef TEST
                printf( "%d.parametr zapsan jako: %d\n", i, borders[i - 1]);
            #endif
        }
        
        #ifdef TEST
            printf("Uvodni testy uspely, pokracuji v kodu...\n");
        #endif       

        int status;         //informace o statusu ukonceni daneho potomka
        pid_t judge_pid, gen_pid;
        
        judge_pid = fork();         //za timto kodem se vse duplikuje
        
        if (judge_pid == 0) {         //proces potomka = proces soudce

            #ifdef TEST
                printf("Proces soudce s PID: %d a PPID: %d\n", getpid(), getppid());
            #endif

            judge_proc();
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

                generate_immigrants(borders[0]);
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
            

    } else {
        fprintf( stderr, "Wrong number of parameters, 5 expected!\n");     //zde musi byt fprintf protoze tisknu do souboru
        exit(EXIT_FAILURE);
    }
    
    #ifdef TEST
        getchar();
        getchar();  
        getchar(); 
    #endif

    return 0;       //sem se program nesmi dostat
}

void judge_proc() {

    #ifdef TEST
        printf("Ja jsem proces soudce.\n");
    #endif

    //sleep((double)((rand() % borders[2]) / 1000));    //uspani na nahodnou dobu z intervalu 0 az borders[2]
    //kriticka sekce
    //printf("%d\t: JUDGE\t: wants to enter", acc_ord);
    //acc_ord++;
    //konec kriticke sekce
}

int generate_immigrants(int count) {

    #ifdef TEST
        printf("Ja jsem proces generatoru.\n");
    #endif

    int status;
    pid_t imm_pid;

    imm_pid = fork ();

    if (imm_pid == 0) {
        
        #ifdef TEST
            printf ("Immigrant cislo: %d s PID: %d a PPID: %d\n", count, getpid(), getppid());
        #endif

        immigrant_proc();
        exit (EXIT_FAILURE);
    } else if (imm_pid < 0) {
        fprintf( stderr, "Fork failed!\n");
        status = -1;
    } else {

        #ifdef TEST
            printf("Proces generovani migrantu(ve funkci gen_imm) s PID: %d a PPID: %d\n", getpid(), getppid());
        #endif
        
        if(count > 1)
            status = generate_immigrants(count - 1);

        if (waitpid (imm_pid, &status, 0) != imm_pid) {
            fprintf( stderr, "Child process termination failed!\n");
            status = -1;
        }

    }   
     
    return status;
}

void immigrant_proc () {

    #ifdef TEST
        printf("Ja jsem proces migranta.\n");
    #endif
    
    //kriticka sekce, pristupuji ke sdilenym promenym
    //printf("%d\t: IMM %d\t: starts", acc_ord, imm_ord);    //hned pri startu procesu
    //acc_ord++;
    //imm_ord++;
    //konec kriticke sekce

}