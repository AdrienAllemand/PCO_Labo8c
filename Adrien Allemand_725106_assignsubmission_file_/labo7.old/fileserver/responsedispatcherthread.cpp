#include "responsedispatcherthread.h"
#include "response.h"
#include <QDebug>

void ResponseDispatcherThread::run()
{
    while(true) {
        if (hasDebugLog)
            qDebug() << "Waiting for responses...";
        Response resp = responses->get();   // block until a response is available
        if (hasDebugLog)
            qDebug() << "Got a response for request '" << resp.getRequest().getFilePath() << "', signalling...";
        responseReady(resp); // send signal
    }
}
