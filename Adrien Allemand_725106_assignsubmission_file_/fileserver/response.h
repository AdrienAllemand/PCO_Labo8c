#ifndef RESPONSE_H
#define RESPONSE_H

#include <QJsonObject>
#include <QJsonDocument>
#include "request.h"

class Response {
private:
    Request request;
    QString response;

public:
    Response(){}
    Response(Request request, QString response):
        request(request), response(response) {}

    QString toJson() {
        QJsonObject obj;
        obj["request"] = request.getFilePath();
        obj["response"] = response;
        QJsonDocument doc(obj);
        QString strJson(doc.toJson(QJsonDocument::Compact));
        return strJson;
    }

    Request getRequest() {
        return request;
    }

    QString getResponse() {
        return response;
    }
};
#endif // RESPONSE_H
