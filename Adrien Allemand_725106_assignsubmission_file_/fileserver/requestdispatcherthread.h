#ifndef REQUESTDISPATCHERTHREAD_H
#define REQUESTDISPATCHERTHREAD_H

#include "concretebuffer.h"
#include <QThread>
#include <QDebug>
#include "request.h"
#include "response.h"
#include "requesthandler.h"
#include "runnablerequest.h"
#include "threadpool.h"

class RequestDispatcherThread : public QThread
{
private:
    AbstractBuffer<Request>* toTreatBuffer;
    AbstractBuffer<Response>* treatedBuffer;
    ThreadPool pool;
    int i = 0;

public:
    RequestDispatcherThread(AbstractBuffer<Request>* toTreatBuffer,
                            AbstractBuffer<Response>* treatedBuffer);
protected:
    void run() Q_DECL_OVERRIDE{
        while(true) {

            qDebug() << "Waiting for requests...";
            Request next = toTreatBuffer->get();
            ++i;
            qDebug() << "Request " << i << "recieved, giving it to ThreadPool";

            // donnes la requête au pool pourqu'il l'assigne à un thread, peut être bloquante
            pool.start(new RunnableRequest(next,treatedBuffer,i));
        }
    }
};

#endif // REQUESTDISPATCHERTHREAD_H
