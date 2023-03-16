#include <QCoreApplication>
#include <QDebug>
#include <QSettings>
#include "moduleservice.h"
#include <QDir>

#include <QString>
#include <QFile>


int main(int argc, char *argv[])
{
    moduleService a(argc, argv);
    return a.exec();
}
