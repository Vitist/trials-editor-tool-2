#-------------------------------------------------
#
# Project created by QtCreator 2019-01-14T01:08:01
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TrialsEditorTool
TEMPLATE = app
RC_ICONS = app_icon.ico

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        trialseditortool.cpp \
    track.cpp \
    config.cpp \
    configdialog.cpp \
    trackoverwritedialog.cpp \
    fusiontrack.cpp \
    risingtrack.cpp \
    fusiontorisingtrack.cpp

HEADERS += \
        trialseditortool.h \
    track.h \
    config.h \
    configdialog.h \
    trackoverwritedialog.h \
    fusiontrack.h \
    risingtrack.h \
    fusiontorisingtrack.h

FORMS += \
        trialseditortool.ui \
    configdialog.ui \
    trackoverwritedialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32: LIBS += -L$$PWD/../../../FreeImage/Dist/x32/ -lFreeImage

INCLUDEPATH += $$PWD/../../../FreeImage/Dist/x32
DEPENDPATH += $$PWD/../../../FreeImage/Dist/x32
