QT       += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

incPath = inc
srcPath = src
formsPath = forms

include(extern/Qt-Ribbon-Widget/QT-Ribbon-Widget.pri)

# INCLUDEPATH += C:\boost_1_79_0\boost_mingw_530/include/boost-1_79
# LIBS += -LC:\boost_1_79_0\boost_mingw_530\lib -llibboost_filesystem-mgw11-mt-x64-1_79


LIBS += -lstdc++fs \

INCLUDEPATH += $$incPath\
               $$incPath/ui

SOURCES += \
    $$srcPath/file.cpp \
    $$srcPath/folder.cpp \
    $$srcPath/main.cpp \
    $$srcPath/ui/mainwindow.cpp \
    $$srcPath/scanThread.cpp \
    $$srcPath/stringUtility.cpp \
    $$srcPath/ui/fileScanPage.cpp \
    $$srcPath/ui/treeViewPage.cpp \
    $$srcPath/ui/compareScanPage.cpp \
    $$srcPath/ui/duplicateScanPage.cpp \
    src/ui/lineCountPage.cpp \
    src/ui/mainwindowRibbon.cpp \
    src/utility.cpp


HEADERS += \
    $$incPath/file.h \
    $$incPath/folder.h \
    $$incPath/ui/mainwindow.h \
    $$incPath/scanThread.h \
    $$incPath/stringUtility.h \
    $$incPath/ui/fileScanPage.h \
    $$incPath/ui/treeViewPage.h \
    $$incPath/ui/compareScanPage.h \
    $$incPath/ui/duplicateScanPage.h \
    inc/ui/lineCountPage.h \
    inc/ui/mainwindowRibbon.h \
    inc/utility.h

FORMS += \
    $$formsPath/duplicateScanPage.ui \
    $$formsPath/fileScanPage.ui \
    $$formsPath/compareScanPage.ui \
    $$formsPath/mainwindow.ui \
    forms/lineCountPage.ui

RESOURCES += \
        resource.qrc

RC_ICONS = icons/scanArchive.ico

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
