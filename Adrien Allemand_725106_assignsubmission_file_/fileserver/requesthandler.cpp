#include <QDebug>
#include "requesthandler.h"
#include "filereader.h"

Response RequestHandler::handle()
{
    if (hasDebugLog)
        qDebug() << "Handling request '" << this->request.getFilePath() << "'...";
    FileReader reader(this->request.getFilePath(), this->hasDebugLog);
    if (reader.fileExists()) {
        return Response(request, reader.readAll());
    } else {
        return Response(request, "File not found!");
    }
}
