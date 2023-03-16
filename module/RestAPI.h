#ifndef RESTAPI_H
#define RESTAPI_H
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <QObject>

class restapi : public QObject
{
    Q_OBJECT
public:
    restapi(QObject *parent = nullptr, const QString &host = "127.0.0.1", const int port = 0);
    ~restapi();

    void setTimer();
    QNetworkAccessManager *manager;
    QByteArray lastPostData;

    void sendRequest(QString &postData, int64_t key);
    QString m_host;
    uint16_t m_port;


signals:
    void endReplyProcess(bool f, int64_t);

private slots:
    void recvReply();

private:
    QHash<QNetworkReply*, int64_t> replys;
};

#endif // RESTAPI_H
