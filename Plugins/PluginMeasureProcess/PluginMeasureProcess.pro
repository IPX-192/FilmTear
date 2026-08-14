QT += widgets  network

TEMPLATE = lib
DEFINES += PLUGINMEASUREPROCESS_LIBRARY

DESTDIR = $$PWD/../../bin/Plugin
MOC_DIR     = $$PWD/temp/moc
RCC_DIR     = $$PWD/temp/rcc
UI_DIR      = $$PWD/temp/ui
OBJECTS_DIR = $$PWD/temp/obj

CONFIG += c++11
CONFIG += force_debug_info    # 带调试信息
QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO += -Od   #禁用优化

CONFIG += skip_target_version_ext  #Windows禁止自动附加在DLL文件名上的版本号
#程序版本
VERSION = 1.0.0.1
#产品名称
QMAKE_TARGET_PRODUCT = "PluginMeasureProcess"
#版权所有
QMAKE_TARGET_COPYRIGHT ="Copyright (C) 2026 ViSensing"

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../Common/NonBlockingMsgBox.cpp \
    CylinderCtrl.cpp \
    Grab/ThreadBox.cpp \
    Grab/ThreadClean.cpp \
    Grab/ThreadFeed.cpp \
    Grab/TurntableGrab.cpp \
    Grab/TurntableProcess.cpp \
    Grab/TurntableWorkStep.cpp \
    Grab/WorkNode/DirtyNode.cpp \
    Grab/WorkNode/FilmTearNode.cpp \
    Grab/WorkNode/PCBCleanNode.cpp \
    PipeLine/PipeLineManager.cpp \
    PipeLine/UdpClient.cpp \
    PipeLine/UdpServer.cpp \
    PluginMeasureProcess.cpp

HEADERS += \
    ../../interface/coreinterface.h \
    ../../interface/plugininterface.h \
    ../Common/NonBlockingMsgBox.h \
    CylinderCtrl.h \
    Grab/ThreadBox.h \
    Grab/ThreadClean.h \
    Grab/ThreadFeed.h \
    Grab/TurntableGrab.h \
    Grab/TurntableProcess.h \
    Grab/TurntableWorkStep.h \
    Grab/WorkNode/DirtyNode.h \
    Grab/WorkNode/FilmTearNode.h \
    Grab/WorkNode/PCBCleanNode.h \
    PipeLine/PipeLineManager.h \
    PipeLine/UdpClient.h \
    PipeLine/UdpServer.h \
    PluginMeasureProcess.h \
    Grab/ThreadGrabDef.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

FORMS += \

win32: LIBS += -L$$PWD/../../bin/ -lVISFramePluginModel

INCLUDEPATH += $$PWD/../../VISFramePluginModel
DEPENDPATH += $$PWD/../../VISFramePluginModel

win32: LIBS += -L$$PWD/../../bin/Plugin -lPluginParam

INCLUDEPATH += $$PWD/../PluginParam
DEPENDPATH += $$PWD/../PluginParam
DEPENDPATH += $$PWD/../Common

INCLUDEPATH += $$(PATH_VIS)/VisAppTool
DEPENDPATH += $$(PATH_VIS)/VisAppTool

win32:CONFIG(release, debug|release): LIBS += -L$$(PATH_VIS)/VisAppTool/ -lVisAppTool
else:win32:CONFIG(debug, debug|release): LIBS += -L$$(PATH_VIS)/VisAppTool/ -lVisAppToold

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../3rd/VisMotorTool/ -lVisMotorTool
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../3rd/VisMotorTool/ -lVisMotorToold

INCLUDEPATH += $$PWD/../../3rd/VisMotorTool
DEPENDPATH += $$PWD/../../3rd/VisMotorTool

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../3rd/VisCameraTool/ -lVisCameraTool
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../3rd/VisCameraTool/ -lVisCameraToold

INCLUDEPATH += $$PWD/../../3rd/VisCameraTool
DEPENDPATH += $$PWD/../../3rd/VisCameraTool


win32:CONFIG(release, debug|release): LIBS += -L$$(PATH_VIS)/opencv/lib/ -lopencv_world411
else:win32:CONFIG(debug, debug|release): LIBS += -L$$(PATH_VIS)/opencv/lib/ -lopencv_world411d

INCLUDEPATH += $$(PATH_VIS)/opencv/include
DEPENDPATH += $$(PATH_VIS)/opencv/include

INCLUDEPATH += $$PWD/../../3rd/VisAnomalyDetect
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../3rd/VisAnomalyDetect/ -lVisAnomalyDetect

INCLUDEPATH += $$PWD/../../3rd/VisDataMatrixFind
DEPENDPATH += $$PWD/../../3rd/VisDataMatrixFind
win32: LIBS += -L$$PWD/../../3rd/VisDataMatrixFind/ -lVisDataMatrixFind

INCLUDEPATH += $$PWD/../../3rd/OpenCV/include
win32: LIBS += -L$$PWD/../../3rd/OpenCV/x64/ -lopencv_world490
