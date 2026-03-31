QT += core gui sql charts network
QT += core gui widgets sql charts webenginewidgets network


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

win32: LIBS += -lws2_32

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    arduino.cpp \
    client.cpp \
    connection.cpp \
    employe.cpp \
    examen.cpp \
    main.cpp \
    autoecole.cpp \
    qrcodegen.cpp \
    seance.cpp \
    vehicule.cpp

HEADERS += \
    arduino.h \
    autoecole.h \
    client.h \
    connection.h \
    employe.h \
    examen.h \
    qrcodegen.hpp \
    seance.h \
    vehicule.h

FORMS += \
    autoecole.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    ressources.qrc

QT += sql

DISTFILES += \
    .gitignore

QT += charts
QT += charts widgets sql

# AJOUTEZ ICI POUR L'EMAIL :
QT += network
QT += core gui widgets charts webchannel positioning
QT += core gui widgets serialport
