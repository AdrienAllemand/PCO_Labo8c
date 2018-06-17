#include "requestdispatcherthread.h"

RequestDispatcherThread::RequestDispatcherThread(AbstractBuffer<Request>* toTreatBuffer,
                                                 AbstractBuffer<Response>* treatedBuffer):
                                                 toTreatBuffer(toTreatBuffer),
                                                 treatedBuffer(treatedBuffer), pool(8)
{

}
