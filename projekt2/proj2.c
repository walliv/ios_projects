#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define TEST        //pro testovaci ucely

int main (int argc, char *argv[])
{
    #ifdef TEST// TESTOVACI

    printf("Pocet parametru: %d\n", argc);
    
    for(int i = 0; (i < argc) && (argc > 0); i++) {
        printf("%d.parametr: %s\n",i , argv[i]);
    }

    #endif//Konec testovacich radku
    
    if (argc == 6) {          //testuje spravny pocet parametru prikaove radky
       
        //Prvky pole:
        // 1. PI pocet procesu karegorie pristehovalcu
        // 2. IG max hodnota doby v milis, po ktere je generovan proces IMM
        // 3. JG max hodnota doby, po ktere JUDGE vstoupi do budovy
        // 4. IT max hodnota doby, kdy IMM vyzvedava certifikat
        // 5. JT max hodnota doby, kdy se JUDGE rozhoduje o vydani
        int borders[5];
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
        printf("Testy uspely, pokracuji v kodu...\n");
        #endif
        
        //zde pokracovat
        

    } else {
        fprintf( stderr, "Wrong number of parameters, 5 expected!\n");     //zde musi byt fprintf protoze tisknu do souboru
        exit(1);
    }
    
    return 0;       //sem se program nesmi dostat
}
