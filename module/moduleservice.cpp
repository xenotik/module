#include "moduleservice.h"
#include "qdebug.h"

#include <QSettings>

moduleService::moduleService(int argc, char **argv)
    : QtService<QCoreApplication>(argc, argv, "Module Daemon")
{
    setServiceDescription("Module");
    setServiceFlags(QtServiceBase::CanBeSuspended);
}

void moduleService::start()
{
    logger = LogEngine::instance();

    QCoreApplication *app = application();
    daemon = new DBprocess();
    QSettings conf("./conf.ini", QSettings::Format::IniFormat);
    qInfo().quote() << "Файл конфигураций: " <<conf.fileName();

    conf.beginGroup("database");
    QString dbDriver = "", dr = "", dbHost = "";
    if((dr = conf.value("source", "KCS").toString().toUpper()) == "KCS")
    {
        dbDriver = "QOCI";
        daemon->moduleType = 2;
    }
    else if (conf.value("source", "PUS").toString().toUpper() == "PUS")
    {
        dbDriver = "QPSQL";
        daemon->moduleType = 3;
        dbHost = conf.value("host", "localhost").toString();
    }

    QString dbName = conf.value("DatabaseName", "redactor").toString();
    QString dbUser = conf.value("UserName", "postgres").toString();
    QString dbPassword = conf.value("password", "postgres").toString();
    conf.endGroup();

    conf.beginGroup("server");
    daemon->servHost = conf.value("host", "127.0.0.1").toString();
    daemon->servPort = conf.value("port", 8001).toInt();
    conf.endGroup();
    if(!daemon->openDB(dbDriver, dr, dbHost, dbName, dbUser, dbPassword))
    {
        qCritical() << "Нет подключения к базе данных";
        return;
    }else qInfo() << "Подключение к базе данных прошло успешно";
}

void moduleService::pause()
{
    daemon->pause();
}

void moduleService::resume()
{
    daemon->resume();
}

void moduleService::stop()
{
    daemon->deleteLater();
}


