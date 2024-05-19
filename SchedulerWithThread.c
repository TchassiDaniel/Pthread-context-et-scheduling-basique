#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>

#define NBRETHREADSINFINI 2 //Nombre de threads scheduler
#define TIMETHREAD 1 //Temps après lequel les thread affiche 
#define TIMESHEDULER 5 //Temps après lequel le trhead sheduler se reveille

//Variable de gestion des threads
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//Condition pour arreter ou demarrer le thread du scheduler
pthread_cond_t conditionScheduler = PTHREAD_COND_INITIALIZER;
//Condition pour arreter ou demarrer les threads infini
pthread_cond_t conditionThreads = PTHREAD_COND_INITIALIZER;

//Fonction qui va gerer la reception d'un signal alarm
//Elle va reveiller le scheduler lorsqu'elle recoit un signal alarm
void handlerAlarm(int signal){
    pthread_mutex_lock(&mutex);
    pthread_cond_signal(&conditionScheduler);
    pthread_mutex_unlock(&mutex);
}

//Fonctionn handler qui va definir le comportement des threads lors de la reception d'un signal
//SIGUSR1(On le suspend) et SIGUSR2(On le remet en marche)
void handlerUSR(int signal){
    if (signal == SIGUSR1){
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&conditionThreads, &mutex);
        pthread_mutex_unlock(&mutex);
    }
}

void *threadInfini(){
    //On recupère le nom du thread(son pid)
    pthread_t monPid = pthread_self();

    //On defini le comportement du thread lors de la reception d'un signal SIGUSR1

    while(1){
        printf("Mon id est: %ld\n",(long)monPid);
        sleep(TIMETHREAD);
    }

    return NULL;
}

void *threadScheduler(void *arg){

    int threadArrete = 0;
    int start = 1;
    pthread_t* thrInf = (pthread_t*)arg;

    //On va suspendre le thread Scheduler pendant 10 secondes et à son reveil il va
    //arreter d'abord un thread infini avant par la suite de commencer son scheduling
    //L'interception du signal alarm reveillera le processus
 
    //Ensuite on commence le scheduling des threads infinis
    while (1)
    {
        //Si onn est au premier tour on arrete d'abord le premier thread infini
        //sinon On va suspendre le thread Scheduler pendant 5 secondes et à son reveil il va effectuer son travail
        if(start){
            alarm(10);
            start = 0;
        }
        else{
            alarm(TIMESHEDULER);
        }
        pthread_mutex_lock(&mutex);
        
        pthread_cond_wait(&conditionScheduler, &mutex);//Le sheduler se met en pause
        
        if(!start)
            pthread_cond_signal(&conditionThreads);//Il reveille le thread infini precedemment arrete
        
        pthread_mutex_unlock(&mutex);
       
        //Par la suite on arrête l'autre thread
        threadArrete = (threadArrete + 1) % NBRETHREADSINFINI;

        pthread_kill(thrInf[threadArrete], SIGUSR1);
        printf("On  arrete le thread %ld\n", (long)thrInf[threadArrete]);
    }
    
    return NULL;
}

int main(){

    pthread_t threads[2], thrScheduler;

    //On change le comportement du signal alarm pour le gerer
    signal(SIGALRM, handlerAlarm);

    //On defini un sigaction pour USR1
    struct sigaction sa;
    sa.sa_handler = handlerUSR;
    sigaction(SIGUSR1, &sa, NULL);

    //On creee les threads infini
    for(int i = 0; i < NBRETHREADSINFINI; i++){
        pthread_create(&threads[i], NULL, threadInfini, NULL);
    }

    //On cree le thread scheduler
    pthread_create(&thrScheduler, NULL, threadScheduler, &threads);
    printf("hello");
    //On attend la terminaison des threads
    for(int i = 0; i < NBRETHREADSINFINI; i++){
        pthread_join(threads[i], NULL);
    }
    pthread_join(thrScheduler, NULL);

    return 0;
}