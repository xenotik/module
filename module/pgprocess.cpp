#include "pgprocess.h"
#include <qobject.h>
#include <QDebug>
#include <QVariant>
#include <QSqlQuery>
#include <QTimer>
#include <QDateTime>
#include <QSqlError>
#include "RestAPI.h"

DBprocess::DBprocess()
{
    logger = LogEngine::instance();
}

bool DBprocess::openDB(const QString &name, const QString &hostName,const QString& DatabaseName, const QString &UserName, const QString &password)
{
    ra = new restapi(this, servHost, servPort);
    if(initDB(name, hostName, DatabaseName, UserName, password))
    {
        if(mDB.open())
            if(mDB.isOpen())
            {
                mDB.driver()->subscribeToNotification("ctl_notify");
                connect(mDB.driver(), SIGNAL(notification(const QString&, QSqlDriver::NotificationSource, const QVariant&)),
                        this, SLOT(ReceingNotify(const QString&, QSqlDriver::NotificationSource, const QVariant&)));

                readFirst();
                return true;
            }else
            {
                qCritical() << mDB.lastError().text();
                return false;
            }
        else
        {
            qCritical() << mDB.lastError().text();
            return false;
        }
    }
    else
    {
        qCritical() << mDB.lastError().text();
        return false;
    }
}

QSqlQuery DBprocess::returnQuery(const QString &s)
{
    QSqlQuery q(mDB);
    if(q.exec(s))
        if(q.next())
            return q;
        else { mlastError = q.lastError().text();}
    else { mlastError = q.lastError().text();}
    return QSqlQuery();
}

bool DBprocess::initDB(const QString &connectName, const QString &hostName,const QString& DatabaseName, const QString &UserName, const QString &password)
{
    mDB = QSqlDatabase::addDatabase("QPSQL", connectName);
    mDB.setHostName(hostName);
    mDB.setDatabaseName(DatabaseName);
    mDB.setUserName(UserName);
    mDB.setPassword(password);
    return true;
}

void DBprocess::sendControl(const QString &s)
{
    QSqlQuery q(mDB);
    QString sq = "Select code_of_event, data_in_json, id"
                 " from for_control where id = '%1'";
    if(!q.exec(sq.arg(s)))
    {
        qCritical() << q.lastError().text();
    }else
        if(q.next())
        {
            QString postData;
            uint64_t key = q.value(2).toLongLong();
            postData = "{\"ModuleType\": 3, \"DataType\":" + q.value(0).toString() + ",\"serial_key\":" + QString::number(key) +
                            ", \"Data\":" + q.value(1).toString()+"}";
            connect(ra, &restapi::endReplyProcess, this, &DBprocess::endSend);
            ra->sendRequest(postData, key);

        }else qCritical() << q.lastError().text();
}

void DBprocess::runList()
{
    for(const auto &it : qAsConst(turnNotification))
    {
        QString list;
        list = it;
        sendControl(list);
    }
}

void DBprocess::readFirst()
{

    QSqlQuery q(mDB);
    QString sq = "Select id"
                 " from for_control ";
    if(!q.exec(sq))
    {
        qCritical() << q.lastError().text();
    }
    else
    {    while(q.next())
        {
            turnNotification.push_back(q.value(0).toString());
        }
        if(!turnNotification.isEmpty())
            runList();
    }

}

void DBprocess::ReceingNotify(const QString &name, QSqlDriver::NotificationSource source, const QVariant &payload)
{
    Q_UNUSED(name);
    Q_UNUSED(source);
    QStringList s = payload.toString().split(" ");

    qCritical() << name <<" " << payload;
    if(s[0] == "ctl")
    {
        turnNotification.push_back(s[2]);
        runList();
    }
}

void DBprocess::endSend(bool f, int64_t key)
{
    if(f)
    {
        QSqlQuery q(mDB);
        QString sq = "delete from for_control where id = '%1'";
        if(!q.exec(sq.arg(key)))
        {
            qCritical() << q.lastError().text();
        }
        int i = turnNotification.indexOf(QString::number(key));
        if(i>-1)
            turnNotification.removeAt(i);
    }
}


//void pgProcess::processAbonentState()
//{
//    QSqlQuery q(mDB);
//    QTime time = QTime::currentTime();
//    if(q.exec(QString("Select session_start_date, session_end_date from abonent_state")))
//    {
//        while(q.next())
//        {
//            if(q.value(0).toDate() == QDate::currentDate())
//            {
//                int timefromConnect = q.value(0).toTime().msecsTo(time);
//                int timeToConnect = q.value(1).toTime().msecsTo(time);
//                if(timefromConnect < 0)
//                    continue;
//                else if(timefromConnect > 300000 && timeToConnect != 0 )
//                    qDebug() << "Абонент не вышел на связь";
//                else if(timefromConnect > 300000 && timeToConnect == 0 )
//                    qDebug() << "Абонент вышел на связь";
//            }
//        }
//    }

//}
