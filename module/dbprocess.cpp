#include "dbprocess.h"
#include <qobject.h>
#include <QDebug>
#include <QVariant>
#include <QSqlQuery>
#include <QTimer>
#include <QDateTime>
#include <QSqlError>
#include "RestAPI.h"
#include <thread>

DBprocess::DBprocess()
{
}

DBprocess::~DBprocess()
{
    ra->deleteLater();
    mDB.close();
}

bool DBprocess::openDB(const QString &driver, const QString &name, const QString &hostName,const QString& DatabaseName, const QString &UserName, const QString &password)
{
    ra = new restapi(this, servHost, servPort);
    if(initDB(driver, name, hostName, DatabaseName, UserName, password))
    {
        if(mDB.open())
            if(mDB.isOpen())
            {
                fPause = false;
                connect(&timer, SIGNAL(timeout()), this, SLOT(ReceingNotify()));
                ReceingNotify();
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

void DBprocess::pause()
{
    fPause = true;
}

void DBprocess::resume()
{
    fPause = false;
}

bool DBprocess::initDB(const QString &driver, const QString &connectName, const QString &hostName,const QString& DatabaseName, const QString &UserName, const QString &password)
{
    mDB = QSqlDatabase::addDatabase(driver, connectName);
    mDB.setHostName(hostName);
    mDB.setDatabaseName(DatabaseName);
    mDB.setUserName(UserName);
    mDB.setPassword(password);
    return true;
}

void DBprocess::sendControl(const QString &s)
{
    QSqlQuery q(mDB);
    QString sq = "Select code_of_event, event_date, data_in_json, id"
                 " from for_control where id = '%1'";
    if(!q.exec(sq.arg(s)))
    {
        qCritical() << q.lastError().text();
    }else
        if(q.next())
        {
            QString postData;
            uint64_t key = q.value(3).toLongLong();
            postData = "{\"ModuleType\":" + QString::number(moduleType)
                    + ", \"DataType\":" + q.value(0).toString()
                    //+ ", \"DateofEvent\":" + q.value(1).toString()
                    + ", \"serial_key\":" + QString::number(key)
                    + ", \"Data\":" + q.value(2).toString()+"}";
            connect(ra, &restapi::endReplyProcess, this, &DBprocess::endSend);
            ra->sendRequest(postData, key);

        }else qCritical() << q.lastError().text();
}

void DBprocess::runList()
{
    if(!fPause)
    {
        for(const auto &it : qAsConst(turnNotification))
        {
            QString list;
            list = it;
            sendControl(list);
        }
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
            if(turnNotification.indexOf(q.value(0).toString()) == -1)
                turnNotification.push_back(q.value(0).toString());
        }
        if(!turnNotification.isEmpty())
            runList();
    }
}

void DBprocess::ReceingNotify()
{
    if(!fPause)
    {
        readFirst();
        timer.setInterval(10000); //ms
        timer.start();
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
