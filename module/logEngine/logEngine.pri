INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

SOURCES += \
        $$PWD/logengine.cpp

HEADERS += \
    $$PWD/logengine.h

DEFINES += QT_MESSAGELOGCONTEXT
CONFIG(debug, release|debug) {
!contains(DEFINES, _DEBUG) DEFINES += _DEBUG
}
