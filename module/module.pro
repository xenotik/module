QT -= gui
QT += sql core network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++17 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        RestAPI.cpp \
        dbprocess.cpp \
        main.cpp \
        moduleservice.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    RestAPI.h \
    dbprocess.h \
    moduleservice.h
include(.\logEngine\logEngine.pri)
include(.\qtservice\src\qtservice.pri)

LIBS += -LC:\app\client\Admin\product\18.0.0\client_1\oci\lib\msvc

#win32:static: {
QMAKE_LFLAGS_RELEASE += -static -static-libgcc
#}
