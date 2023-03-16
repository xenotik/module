#include "RestAPI.h"
#include "qnetworkrequest.h"
#include <QDebug>
#include <QHttpPart>

restapi::restapi(QObject *parent, const QString &host, const int port )
    : QObject(parent), manager(new QNetworkAccessManager(this)), m_host(host), m_port(port)
{
}

restapi::~restapi()
{

}

//void restapi::setTimer()
//{
//    QDateTime dttm = QDateTime::currentDateTime();
//    int interval = dttm.msecsTo(dttm);
//    if(interval>0)
//        toEvent.setInterval(interval);
//    else
//        qDebug() << "Нет ближайшего события";
//}


void restapi::recvReply()
{
    auto reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply)
        return;
    uint64_t key;
    if(auto it = replys.find(reply); it == replys.end()){
        //error
        return;
    }
    else {
        key = *it;
        replys.erase(it);
    }
    bool f=false;
    if(reply->error() != QNetworkReply::NoError)
    {
        qCritical() << reply->error();
        f = false;
    }
    else
    {
        int attr = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if( attr >=200 && attr <300)
        {
            qInfo() <<"reply: code:"<< attr << ", value:" << reply->readAll();
            f = true;
        }else
            f = false;
    }
    replys.erase(replys.find(reply));
    reply->deleteLater();
    emit endReplyProcess(f, key);
}


void restapi::sendRequest(QString &postData, int64_t key)
{
    QNetworkRequest request;
    QString url = QString("http://%1:%2/%3").arg(m_host).arg(m_port).arg("api/event/");
    request.setUrl(QUrl(url));
    request.setRawHeader("Content-Type","application/json");
    QHttpPart postData1;
    QByteArray a = postData.toUtf8();
    qInfo()<<"post:" << a;

    auto reply = manager->post(request, postData.toUtf8());
    replys[reply] = key;
    connect(reply, &QNetworkReply::finished, this, &restapi::recvReply);
}

