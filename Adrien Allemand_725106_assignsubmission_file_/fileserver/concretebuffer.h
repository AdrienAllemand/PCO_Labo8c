#ifndef REQUESTBUFFER_H
#define REQUESTBUFFER_H

#include "abstractbuffer.h"
#include "request.h"
#include "vector"
#include "QSemaphore"
#include "QDebug"

#define BUFF_SIZE 127
template<typename T>
class ConcreteBuffer: public AbstractBuffer<T> {
private:

    // le tampon
    T buffer[BUFF_SIZE];
    int write_index;    // position d'écriture
    int read_index;     // position de lecture
    int free_space;     // place libre dans le buffer

    int write_waiting;  // nombre de producteurs en attente
    int read_waiting;   // nombre de consomateurs en attente

    QSemaphore* mutex;      // protège les variables
    QSemaphore* put_wait;   // bloque l'insertion
    QSemaphore* get_wait;   // bloque l'extraction

public:
    ConcreteBuffer() {
        free_space = BUFF_SIZE;
        write_index = 0;
        read_index = 0;

        mutex = new QSemaphore(1);
        put_wait = new QSemaphore(0);
        get_wait = new QSemaphore(0);
    }

    ~ConcreteBuffer(){
        delete mutex;
        delete put_wait;
        delete get_wait;
    }

    void put(T item){

        mutex->acquire();

        while(free_space == 0){
            ++write_waiting;

            mutex->release();
            put_wait->acquire();
            mutex->acquire();
        }

        // ajouter la ressource dans le buffer
        buffer[write_index] = item;
        qDebug() << "Buffer : putting item into slot " << write_index;

        // maj des variables d'écriture
        write_index = (write_index + 1) % BUFF_SIZE;
        --free_space;

        // // si il y a un consommateur en attente
        if(read_waiting > 0){
            --read_waiting;
            get_wait->release();    // on le relache
        }

        mutex->release();
    }

    bool tryPut(T item){

        mutex->acquire();

        // si le buffer est plein on retourne false
        if(free_space == 0){
            mutex->release();
            return false;
        }

        // ajouter la ressource dans le buffer
        buffer[write_index] = item;

        // maj des variables d'écriture
        write_index = (write_index + 1) % BUFF_SIZE;
        --free_space;

        // // si il y a un consommateur en attente
        if(read_waiting > 0){
            --read_waiting;
            get_wait->release();    // on le relache
        }

        mutex->release();
        return true;
    }

    T get(){
        mutex->acquire();

        // bloquer si il y a rien a get
        while(free_space == BUFF_SIZE){

            ++read_waiting;
            mutex->release();

            get_wait->acquire();

            mutex->acquire();
        }

        // récupérer la ressource dans le buffer
        T answer = buffer[read_index];

        // maj des variables de lecture
        read_index = (read_index + 1) % BUFF_SIZE;
        ++free_space;

        // Si il y a un producteur en attente
        if(write_waiting > 0){
            --write_waiting;
            put_wait->release();    // on le relache
        }

        mutex->release();
        return answer;
    }

};

#endif // REQUESTBUFFER_H
