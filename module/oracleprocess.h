#ifndef ORACLEPROCESS_H
#define ORACLEPROCESS_H

#include <QSqlDatabase>
#include <QObject>

#include "RestAPI.h"

class oracleProcess  : public QObject
{
    Q_OBJECT
private:
    oracleProcess();
    QSqlDatabase mDB;
    bool initDB(const QString &connectName, const QString &hostName,const QString& DatabaseName, const QString &UserName, const QString &password);
    void runList();
    void readFirst();


    restapi* ra;

public:

    QString servHost;
    int servPort;

    bool OpenDB(const QString &name, const QString &hostName,const QString& DatabaseName, const QString &UserName, const QString &password);
};

#endif // ORACLEPROCESS_H
