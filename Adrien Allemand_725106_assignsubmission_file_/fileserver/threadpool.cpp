#include "threadpool.h"
#include <QDebug>

ThreadPool::ThreadPool(int maxThreadCount): maxThreads(maxThreadCount) {

    poolMonitor = new HoareMonitor();

    slaves = new SlaveThread*[maxThreads];

    // initialisés à nullptr car c'est comme cela que nous savons qu'ils n'ont pas été crées
    for(int i = 0; i < maxThreads; i++){
        slaves[i] = nullptr;
    }
}

ThreadPool::~ThreadPool() {
    // interruption des slaves
    for(int i = 0; i < maxThreads; i ++){

        if(slaves[i] != nullptr){
            slaves[i]->setTimeToDie();  // prévient le slave qu'il doit s'arrêter
            slaves[i]->wait();          // on attend qu'il s'arrête proprement
            delete slaves[i];           // désallocation
        }
    }

    delete slaves;          // désallocation du tableau de slaves*
    delete poolMonitor;     // désallocation du moniteur de hoare
}

void ThreadPool::start(Runnable *runnable) {
    poolMonitor->monitorIn();

    // fonction bloquante tant qu'on n'a pas trouvé de thread pour exécuter le Runnable
    bool threadfound = false;
    while(!threadfound){

        // parcours des slaveThreads pour en trouver un de libre
        for(int i = 0; i < maxThreads; i++){

            // si le thread[i] est null on le crée puis le démarre
            if(slaves[i] == nullptr){
                slaves[i] = new SlaveThread(poolMonitor, &c, i);
                slaves[i]->start();
            }

            // si le thread[i] est libre on lui donne la tache et lui signale de se réveiller
            if (slaves[i]->isfree()){				 // test si le slave est libre
                slaves[i]->setRunnable(runnable);	 // si oui lui set son runnable
                slaves[i]->wakeup();				 // le réveil pour lui dire qu'il a du travail
                threadfound = true;
                break;
            }
        }

        // si on n'a toujours pas trouvé de thread libre on va placer le pool en attente
        if(!threadfound) {
            poolMonitor->wait(c);
        }
    }
    poolMonitor->monitorOut();
}
