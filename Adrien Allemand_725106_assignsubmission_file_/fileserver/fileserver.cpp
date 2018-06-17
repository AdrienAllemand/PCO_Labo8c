/****************************************************************************
**
** Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebSockets module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "fileserver.h"
#include "QtWebSockets/qwebsocketserver.h"
#include "QtWebSockets/qwebsocket.h"
#include <QDebug>
#include "filereader.h"
#include "response.h"
#include "request.h"

FileServer::FileServer(quint16 port, bool debug, QObject *parent) :
    QObject(parent),
    websocketServer(new QWebSocketServer(QStringLiteral("File Server"),
                                            QWebSocketServer::NonSecureMode, this)),
    hasDebugLog(debug)
{
    // requests = new... TODO
    requests = new ConcreteBuffer<Request>();
    // responses = new... TODO
    responses = new ConcreteBuffer<Response>();
    // reqDispatcher = new... TODO
    reqDispatcher = new RequestDispatcherThread(requests, responses);
    reqDispatcher->start();

    respDispatcher = new ResponseDispatcherThread(responses, hasDebugLog);
    respDispatcher->start();

    qDebug() << "Connecting signal to readyResponse";
    connect(respDispatcher, SIGNAL(responseReady(Response)), this, SLOT(handleResponse(Response)));

    if (websocketServer->listen(QHostAddress::Any, port)) {
        //if (hasDebugLog)
            qDebug() << "Fileserver listening on port" << port << "with strategy: 8 threads in pool";
        connect(websocketServer, &QWebSocketServer::newConnection,
                this, &FileServer::onNewConnection);
        connect(websocketServer, &QWebSocketServer::closed, this, &FileServer::closed);
    }
}

FileServer::~FileServer()
{
    websocketServer->close();
    delete reqDispatcher;
    delete respDispatcher;
    qDeleteAll(clients.begin(), clients.end());
}

void FileServer::onNewConnection()
{
    if (hasDebugLog)
        qDebug() << "New client connection";
    QWebSocket *pSocket = websocketServer->nextPendingConnection();
    connect(pSocket, &QWebSocket::textMessageReceived, this, &FileServer::processTextMessage);
    connect(pSocket, &QWebSocket::binaryMessageReceived, this, &FileServer::processBinaryMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &FileServer::socketDisconnected);

    clients[pSocket->origin()] = pSocket;
}

void FileServer::processTextMessage(QString message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (hasDebugLog)
        qDebug() << "Message received:" << message;
    if (pClient) {
        Request req(message, pClient->origin());

        // passe par le tryPut avec possibilité de refus de la requête
        if(!requests->tryPut(req)){
            pClient->sendTextMessage(Response(req, "server overloaded,try later").toJson());
        }

    }
}

void FileServer::processBinaryMessage(QByteArray message)
{
   if (hasDebugLog) {
        qDebug() << "Binary Message received (ignoring):" << message;
    }
}

void FileServer::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (hasDebugLog)
        qDebug() << "Client disconnected:" << pClient;
    if (pClient) {
        clients.remove(pClient->origin());
        pClient->deleteLater();
    }
}

void FileServer::handleResponse(Response response)
{
    if (hasDebugLog) {
        QString json = response.toJson();
        json.truncate(200);
        qDebug() << "Sending response:" << json;
    }
    clients[response.getRequest().getClientId()]->sendTextMessage(response.toJson());
}
