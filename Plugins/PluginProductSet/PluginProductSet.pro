QT += widgets network serialport opengl xml

TEMPLATE = lib
DEFINES += PLUGINPRODUCTSET_LIBRARY

DESTDIR = $$PWD/../../bin/Plugin
MOC_DIR     = $$PWD/temp/moc
RCC_DIR     = $$PWD/temp/rcc
UI_DIR      = $$PWD/temp/ui
OBJECTS_DIR = $$PWD/temp/obj

CONFIG += c++11
CONFIG += force_debug_info    # 带调试信息
QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO += -Od   #禁用优化

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
    ItemDelegate.cpp \
    PluginProductSet.cpp \
    Tray/TrayCtrl.cpp \
    WidgetMatrixCfg.cpp \
    WidgetProductSet.cpp \
    WidgetRecipeFilmTear.cpp \
    WidgetRecipeGrip.cpp \
    WidgetRecipeMotor.cpp \
    WidgetRecipePlatform.cpp \
    WidgetRecipeTray.cpp \
    WidgetRecipeVision.cpp \
    WidgetRecipeDirty.cpp

HEADERS += \
    ../../interface/coreinterface.h \
    ../../interface/plugininterface.h \
    ItemDelegate.h \
    PluginProductSet.h \
    Tray/TrayCtrl.h \
    WidgetMatrixCfg.h \
    WidgetProductSet.h \
    WidgetRecipeFilmTear.h \
    WidgetRecipeGrip.h \
    WidgetRecipeMotor.h \
    WidgetRecipePlatform.h \
    WidgetRecipeTray.h \
    WidgetRecipeVision.h \
    WidgetRecipeDirty.h

LIBS += -lAdvapi32

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

FORMS += \
    Tray/TrayCtrl.ui \
    WidgetMatrixCfg.ui \
    WidgetProductSet.ui \
    WidgetRecipeFilmTear.ui \
    WidgetRecipeGrip.ui \
    WidgetRecipeMotor.ui \
    WidgetRecipePlatform.ui \
    WidgetRecipeTray.ui \
    WidgetRecipeVision.ui \
    WidgetRecipeDirty.ui

win32: LIBS += -L$$PWD/../../bin/ -lVISFramePluginModel

INCLUDEPATH += $$PWD/../../VISFramePluginModel
DEPENDPATH += $$PWD/../../VISFramePluginModel

win32: LIBS += -L$$PWD/../../bin/Plugin -lPluginParam

INCLUDEPATH += $$PWD/../PluginParam
DEPENDPATH += $$PWD/../PluginParam

win32:CONFIG(release, debug|release): LIBS += -L$$(PATH_VIS)/VisAppTool/ -lVisAppTool
else:win32:CONFIG(debug, debug|release): LIBS += -L$$(PATH_VIS)/VisAppTool/ -lVisAppToold

INCLUDEPATH += $$(PATH_VIS)/VisAppTool
DEPENDPATH += $$(PATH_VIS)/VisAppTool

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../3rd/VisMotorTool/ -lVisMotorTool
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../3rd/VisMotorTool/ -lVisMotorToold

INCLUDEPATH += $$PWD/../../3rd/VisMotorTool
DEPENDPATH += $$PWD/../../3rd/VisMotorTool


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../3rd/VisCameraTool/ -lVisCameraTool
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../3rd/VisCameraTool/ -lVisCameraToold

INCLUDEPATH += $$PWD/../../3rd/VisCameraTool
DEPENDPATH += $$PWD/../../3rd/VisCameraTool

INCLUDEPATH += $$PWD/../../3rd/VisFindEdge
INCLUDEPATH += $$PWD/../../3rd/halcon/include
INCLUDEPATH += $$PWD/../../3rd/halcon/include/halconc
INCLUDEPATH += $$PWD/../../3rd/halcon/include/halconcpp
INCLUDEPATH += $$PWD/../../3rd/halcon/include/hclib
INCLUDEPATH += $$PWD/../../3rd/halcon/include/hdevengine
INCLUDEPATH += $$PWD/../../3rd/halcon/include/hlib
INCLUDEPATH += $$PWD/../../3rd/halconMatchDllPlatform

win32: LIBS += -L$$PWD/../../3rd/AlgorithmModule/ -lAlgorithmModule
INCLUDEPATH += $$PWD/../../3rd/AlgorithmModule
DEPENDPATH += $$PWD/../../3rd/AlgorithmModule

win32: LIBS += -L$$PWD/../../3rd/ImageDraw/ -lImageDraw
INCLUDEPATH += $$PWD/../../3rd/ImageDraw
DEPENDPATH += $$PWD/../../3rd/ImageDraw

win32: LIBS += -L$$PWD/../../3rd/VisFindEdge/ -lVisFindEdge
INCLUDEPATH += $$PWD/../../3rd/VisFindEdge
DEPENDPATH += $$PWD/../../3rd/VisFindEdge

win32:CONFIG(release, debug|release): LIBS += -L$$(PATH_VIS)/opencv/lib/ -lopencv_world411
else:win32:CONFIG(debug, debug|release): LIBS += -L$$(PATH_VIS)/opencv/lib/ -lopencv_world411d

INCLUDEPATH += $$(PATH_VIS)/opencv/include
DEPENDPATH += $$(PATH_VIS)/opencv/include

INCLUDEPATH += $$(PATH_VIS)/VisCommon
DEPENDPATH += $$(PATH_VIS)/VisCommon

win32:CONFIG(release, debug|release): LIBS += -L$$(PATH_VIS)/VisCommon/ -lVisCommon
else:win32:CONFIG(debug, debug|release): LIBS += -L$$(PATH_VIS)/VisCommon/ -lVisCommond

INCLUDEPATH += -L$$PWD/../../3rd/VisAnomalyDetect
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../3rd/VisAnomalyDetect/ -lVisAnomalyDetect

