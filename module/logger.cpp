#include "logger.h"
#include <QString>
#include <QDateTime>


Logger::Logger(QObject *parent) : QObject{parent}
{
    mFile = new QFile("logFile.log");
    if(mFile->open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        mStream = new QTextStream(mFile);
    }
    else qDebug("Файл не открыт");

}

Logger::~Logger()
{
    mFile->close();
    delete(mStream);
    delete(mFile);
}

void Logger::WriteMsg(const QString &msg)
{
    auto text = QString("%1:%2").arg(QDateTime::currentDateTime().toString("dd.MM.yy HH:mm:ss"), msg);
    if(mStream) (*mStream) << text << Qt::endl;
}
