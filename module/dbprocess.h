#pragma once
#include "RestAPI.h"
#include <QTimer>
#include <QSqlDriver>
#include <QObject>
#include <QSqlDatabase>
#include <iostream>

#include <thread>
#include <condition_variable>
#include <mutex>




using namespace std;

class DBprocess  : public QObject
{
    Q_OBJECT
private:
    QSqlDatabase mDB;

    bool initDB(const QString &driver, const QString &connectName, const QString &hostName,const QString& DatabaseName, const QString &UserName, const QString &password);
    void runList();
    void readFirst();
    QStringList turnNotification;
    restapi* ra;
    QTimer timer;
    bool fPause;



public slots:
    void sendControl(const QString &s);

    void ReceingNotify();
    void endSend(bool f, int64_t key);
//    void processAbonentState();


public:

    DBprocess();
    ~DBprocess();
    bool openDB(const QString &driver, const QString &name, const QString &hostName, const QString& DatabaseName, const QString &UserName, const QString &password);
    QSqlQuery returnQuery(const QString &s);
    void pause();
    void resume();

    QString servHost;
    int servPort;
    mutable QString mlastError;
    int moduleType;


    enum ProgramModuleType
    {
        Ats = 1,
        Kcs = 2,
        Puss = 3,
        Sputnik = 4,
        CentralServer = 100
    };

    enum DataType
    {
        Message = 1, //Сообщение
        SessionTime = 2 , //Время выхода на связь
        RoutingTable = 3, //Таблица маршрутизации
        OperatorActions = 4, //Действия оператора
        ChannelAccidents = 5, //Аварийные ситуации на канале
        Call = 6, //Соединение
        CallBreak = 7, //Обрыв соединения
        CallComment = 8, //Замечание по сеансу связи
        FaultyChannels = 9, //Неисправные каналы связи
        RecoveryChannelAccident = 10 //Восстановление аварийной ситуации
    };
};
