#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H
#include "request.h"
#include "response.h"

class RequestHandler
{
private:
    Request request;
    bool hasDebugLog;

public:
    RequestHandler(Request request, bool hasDebugLog): request(request), hasDebugLog(hasDebugLog) {}

    Response handle();
};

#endif // REQUESTHANDLER_H
