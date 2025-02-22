QT       += core gui network printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QMAKE_CXXFLAGS += /utf-8
QMAKE_CFLAGS += /utf-8
CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    connect_page/connect_page.cpp \
    continuous_read_thread/continuous_read_thread.cpp\
    main.cpp \
    mainwindow.cpp \
    once_trigger_thread/once_trigger_thread.cpp \
    scroll_mode_thread/scroll_mode_thread.cpp \
    utils/uint_validator/uint_validator.cpp \
    utils/utils.cpp \
    wave/base_wave_widget.cpp \
    wave/qcustomplot.cpp

HEADERS += \
    connect_page/connect_page.h \
    continuous_read_thread/continuous_read_thread.h\
    mainwindow.h \
    once_trigger_thread/once_trigger_thread.h \
    scroll_mode_thread/scroll_mode_thread.h \
    utils/uint_validator/uint_validator.h \
    utils/utils.h \
    wave/base_wave_widget.h \
    wave/qcustomplot.h

FORMS += \
    connect_page/connect_page.ui \
    mainwindow.ui

TRANSLATIONS += \
    KK_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH +=$$PWD

win32: LIBS += -L$$PWD/DAQ_IPC/lib/windows/ -llibdaq-3.5.6

INCLUDEPATH += $$PWD/DAQ_IPC/include
DEPENDPATH += $$PWD/DAQ_IPC/include
INCLUDEPATH += $$PWD/DAQ_IPC/include/third_party
