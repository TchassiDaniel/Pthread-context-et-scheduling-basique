#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>

int NbreThreadsTermine = 0;
int fin = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER; 


typedef struct infoSearch{
    //Cette variable contient l'adresse de debut du tableau sur lequel le thread va travailler
    int *adrDebutTabDuThread;
    //Ici il s'agit de l'indice du debut de la partie du tableau ou le thread travaille
    long indiceTabDebut;
    //Celle ci l'indice de fin
    long indiceTabFin;
    //Celle ci contient l'adresse vers la variable qui contient le nombre premiers
    //Dont on cherche actuellement les multiples
    long *tampon;
} infoSearch;

typedef struct infoScheduler
{
    //L'adresse de debut du tableau
    int *addrTableau;
    //Taille du tableau
    long tailleTableau;
    //Nbre de thread
    int nbreThread;
    //Le pid desdits threads
    pthread_t *tabThreads;
    //Celle ci contient l'adresse vers la variable qui contient le nombre premiers
    //Dont on cherche actuellement les multiples
    long *tampon;
}infoScheduler;


/*
Cette fonction est celle qui va etre execute par les threads qui vont faire de srecherches
Dans le tableau pour mettre a false les elements du tableau dont la position n'est pas un *
nombre premier.
*/
void *threadSearch(void *arg){
    //Cette variable contient l'adresse de debut du tableau sur lequel le thread va travailler
    infoSearch *info = (infoSearch*)arg;

    while(fin == 0){

        //Si la valeur du tampon est non nul on va barrer les multiples
        if(*(info->tampon) != 0){
                for(int i = info->indiceTabDebut, j= 0; i <= info->indiceTabFin; i++, j++){
                if(*(info->adrDebutTabDuThread + j) == 1){
                    if((i % *(info->tampon)) == 0){
                        *(info->adrDebutTabDuThread + j) = 0;
                    }
                }
            }
        }    
        //On incrémente un variable pour dire qu'on a terminé notre recherche
        //Et on s'arrete
        NbreThreadsTermine++;
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&cond, &mutex);
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

//Cette fonction va schedule les threads et definir les éléments à chercher dans le tableau
void *SchedulerCrible(void *arg){

    infoScheduler *info = (infoScheduler*)arg;
    //Position du nombre premier actuel sur lequel les threads travaillent
    long positionNombrePremierActuel = 0;
    //Element maximal dont on va barrer les multiples
    long elementMax = sqrt(info->tailleTableau);

    while(fin == 0){
        //si tout les threads sont en pause on determine le prochain nombre premier
        //On mets a jour tampon et on relance tout les threads
        if(NbreThreadsTermine == info->nbreThread){
            NbreThreadsTermine = 0;

            *(info->tampon) = 0;
            //On chercher le prochain nombre premier dont on va barrer les multiples
            while(info->tampon == 0){
                if(info->addrTableau[positionNombrePremierActuel] == 1){
                    *(info->tampon) = positionNombrePremierActuel;
                }
                positionNombrePremierActuel++;
            }

            if(positionNombrePremierActuel == elementMax){
                fin = 1;
            }

            //On relance tout les threads
                pthread_mutex_lock(&mutex);
                pthread_cond_broadcast(&cond);
                pthread_mutex_unlock(&mutex);
        }

        //On met le threasheduler en pause
        pthread_mutex_lock(&mutex);
        //On obtiens l'heure actuelle
        struct timespec temps;
        clock_gettime(CLOCK_REALTIME, &temps);
        //gettimeofday(&temps, NULL);
        temps.tv_sec += 2;

        pthread_cond_timedwait(&cond, &mutex, &temps);
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}
int main(){
    int nombreThreads = 2;
    pthread_t threads[2], thrScheduler;
    infoSearch infoThread[2];
    long tailleTableau = 100;
        printf("Hello");
    int *tableau = (int*)malloc(sizeof(int)*tailleTableau);
    long tampon;
    //On intitialise le tableau
    printf("Hello");

    tableau[0] = 0;
    tableau[1] = 0;
    for(int i = 2; i < tailleTableau; i++){
        tableau[i] = 1;
    }
    
    //Taille du tableau par thread
    long tailleTableauThread = tailleTableau / nombreThreads;
    for(int i = 0; i < nombreThreads; i++){
        infoThread[i].adrDebutTabDuThread = tableau+tailleTableauThread*i;
        infoThread[i].indiceTabDebut = tailleTableauThread*i;  
        infoThread[i].indiceTabFin = tailleTableauThread*(i+1); 
        infoThread[i].tampon = &tampon;
        pthread_create(&threads[i], NULL, threadSearch, &infoThread[i]);
    }

    //On lance le scheduler
    infoScheduler info = {tableau, tailleTableau, 2, threads, &tampon};
    pthread_create(&thrScheduler, NULL, SchedulerCrible, &info);
    pthread_join(thrScheduler, NULL);
    for(int i= 0; i < nombreThreads; i++){
        pthread_join(threads[i], NULL);
    }
    //on affiche les indices des nombre premier
    for(int i = 0; i < tailleTableau; i++){
        if(tableau[i] == 1){
            printf("%d ", i);
        }
    }

    return 0; 
}