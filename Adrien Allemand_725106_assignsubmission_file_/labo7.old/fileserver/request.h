#ifndef REQUEST_H
#define REQUEST_H
#include <QString>

class Request {
private:
    QString filePath;
    QString clientId;

public:
    Request() {}
    Request(QString filePath, QString clientId): filePath(filePath), clientId(clientId) {}

    QString getFilePath() {
        return filePath;
    }

    QString getClientId() {
        return clientId;
    }
};

#endif // REQUEST_H
