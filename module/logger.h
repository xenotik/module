#pragma once

#include <QObject>
#include <QString>
#include <QFile>
#include <QTextStream>

class Logger : public QObject
{
    Q_OBJECT
    explicit Logger(QObject *parent = 0);
public:
    static Logger* getLogger();
    ~Logger();


   void WriteMsg(const QString& msg);

private:
    QTextStream *mStream;
    QFile* mFile;
};
