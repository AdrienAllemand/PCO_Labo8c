#ifndef SLAVETHREAD_H
#define SLAVETHREAD_H

#include "QThread"
#include "runnable.h"
#include <QMutex>
#include "QWaitCondition"
#include "hoaremonitor.h"
#include "QDebug"

class SlaveThread : public QThread {

private:
    Runnable* r;

    int id;

    bool _isfree;
    bool stop = false;

    QMutex* mutex;
    QWaitCondition* slaveWait;

    HoareMonitor* poolMonitor;
    HoareMonitor::Condition* poolWait;

    bool isTimeToDie = false;

public:
    SlaveThread(HoareMonitor* poolMonitor, HoareMonitor::Condition* poolWait, int id): poolMonitor(poolMonitor), poolWait(poolWait), id(id) {

        mutex = new QMutex();
        slaveWait= new QWaitCondition();

        _isfree = true;
    }

    ~SlaveThread(){
        delete mutex;
        delete slaveWait;
    }


    /**
     * @brief isfree retourne l'état du slavethread, si ce dernier est libre il passe directement a occupé
     *               et retourne true sinon il retourne false. Commme on est protégé par le mutex cela
     *               garantie que un seul appelant aura "true" comme réponse à la fois
     * @return
     */
    bool isfree(){
        mutex->lock();
        bool temp = _isfree;
        if(_isfree){
            _isfree = !_isfree;
        }
        mutex->unlock();
        return temp;
    }


    /**
     * @brief setTimeToDie dit à un thread de s'arrêter proprement
     */
    void setTimeToDie(){
        mutex->lock();
        isTimeToDie = true;
        mutex->unlock();
        slaveWait->wakeOne();
    }
    /**
     * @brief wakeup lance un signal de réveil sur le slave
     */
    void wakeup(){
        slaveWait->wakeOne();
    }

    /**
     * @brief setRunnable setter qui définit un nouveau Runnable
     * @param r
     */
    void setRunnable(Runnable* r){
        mutex->lock();
        this->r = r;
        mutex->unlock();
    }

    /**
     * @brief run boucle infinie d'exécution
     */
    void run(){

        mutex->lock();
        while (!isTimeToDie){
            // tant que le slave n'est pas occupé il va se mettre en attente
            while(_isfree && !isTimeToDie){
                slaveWait->wait(mutex);     // attente que on lui signale qu'il a du travail
            }

            if(isTimeToDie){
                break;  // interruption si réveillé pour mourir
            }

            mutex->unlock();    // super important que le mutex soit unlock pour le r->run()
            r->run();           // exécution de la tâche

            mutex->lock();
            _isfree = true;     // passe son état a libre
            mutex->unlock();    // pour que le pool puisse faire le isfree() sans être bloqué

            poolMonitor->monitorIn();
            poolMonitor->signal(*poolWait);  // signal au pool que on est libre
            poolMonitor->monitorOut();

            mutex->lock();
        }
        mutex->unlock();
    }
};

#endif // SLAVETHREAD_H
