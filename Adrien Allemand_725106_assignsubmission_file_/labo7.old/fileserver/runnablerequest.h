#ifndef SLAVETHREAD_H
#define SLAVETHREAD_H

#include "runnable.h"
#include "qstring.h"

class RunnableRequest: public Runnable {
private:
    Request r;
    AbstractBuffer<Response>* a;
    int i;
public:
    RunnableRequest(Request r, AbstractBuffer<Response>* a, int i):r(r), a(a), i(i){

    }
    ~RunnableRequest(){}

    QString id(){
        return QString::number(i);
    }

    void run(){
        RequestHandler handler(r, false);
        a->put(handler.handle());
    }

};

#endif // SLAVETHREAD_H
