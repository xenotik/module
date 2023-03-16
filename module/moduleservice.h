#ifndef MODULESERVICE_H
#define MODULESERVICE_H


#include <QCoreApplication>
#include "qtservice/src/qtservice.h"
#include "dbprocess.h"

#include "logengine.h"
class moduleService : public QtService<QCoreApplication>
{
public:
    moduleService(int argc, char **argv);
    void start();
    void pause();
    void resume();
    void stop();
private:
    DBprocess *daemon;

    LogEngine::LogEngine_ptr_t logger;
};
#endif // MODULESERVICE_H
