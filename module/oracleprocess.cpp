#include "oracleprocess.h"
#include "qsqldriver.h"
#include "qsqlerror.h"

oracleProcess::oracleProcess()
{

}

bool oracleProcess::initDB(const QString &connectName, const QString &hostName, const QString &DatabaseName, const QString &UserName, const QString &password)
{
    mDB = QSqlDatabase::addDatabase("QOCI", connectName);
    mDB.setHostName(hostName);
    mDB.setDatabaseName(DatabaseName);
    mDB.setUserName(UserName);
    mDB.setPassword(password);
    return true;
}

bool oracleProcess::OpenDB(const QString &name, const QString &hostName, const QString &DatabaseName, const QString &UserName, const QString &password)
{
    ra = new restapi(this, servHost, servPort);
    if(initDB(name, hostName, DatabaseName, UserName, password))
    {
        if(mDB.open())
            if(mDB.isOpen())
            {
//                mDB.driver()->subscribeToNotification("ctl_notify");
//                connect(mDB.driver(), SIGNAL(notification(const QString&, QSqlDriver::NotificationSource, const QVariant&)),
//                        this, SLOT(ReceingNotify(const QString&, QSqlDriver::NotificationSource, const QVariant&)));
                //readFirst();
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
