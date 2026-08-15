QT += widgets opengl network

TEMPLATE = lib
DEFINES += PLUGINAUTOPAGE_LIBRARY

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
QMAKE_TARGET_PRODUCT = "PluginAutoPage"
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
include(Product/Product.pri)

SOURCES += \
    ../Common/CurveView/Chart/CustomChart.cpp \
    ../Common/CurveView/Chart/qcustomplot.cpp \
    ../Common/MyQwtPlot.cpp \
    ../Common/NonBlockingMsgBox.cpp \
    ../Common/SwitchButton.cpp \
    ../Common/WidgetLog.cpp \
    PartsSource/WearingParts.cpp \
    PartsSource/WearingPartsList.cpp \
    PartsSource/WearingPartsListUser.cpp \
    PluginAutoPage.cpp \
    ProductForm/ProductDetailForm.cpp \
    ProductForm/WidgetProdutData.cpp \
    WidgetAutoPage.cpp \
    WidgetBtnPanel.cpp \
    WidgetCalibImage.cpp \
    WidgetConsumables.cpp \
    WidgetFlowState.cpp \
    WidgetLogAll.cpp \
    WidgetProductChange.cpp \
    WidgetSingleTest.cpp \
    WidgetStatusBar.cpp \
    WidgetTrayState.cpp \
    WidgetVision.cpp


HEADERS += \
    ../../interface/coreinterface.h \
    ../../interface/plugininterface.h \
    ../Common/CurveView/Chart/CustomChart.h \
    ../Common/CurveView/Chart/qcustomplot.h \
    ../Common/MyQwtPlot.h \
    ../Common/NonBlockingMsgBox.h \
    ../Common/PipeLineDef.h \
    ../Common/SwitchButton.h \
    ../Common/WidgetLog.h \
    PartsSource/WearingParts.h \
    PartsSource/WearingPartsList.h \
    PartsSource/WearingPartsListUser.h \
    PluginAutoPage.h \
    ProductForm/ProductDetailForm.h \
    ProductForm/WidgetProdutData.h \
    WidgetAutoPage.h \
    WidgetBtnPanel.h \
    WidgetCalibImage.h \
    WidgetConsumables.h \
    WidgetFlowState.h \
    WidgetLogAll.h \
    WidgetProductChange.h \
    WidgetSingleTest.h \
    WidgetStatusBar.h \
    WidgetTrayState.h \
    WidgetVision.h


# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

FORMS += \
    ../Common/SwitchButton.ui \
    ../Common/WidgetLog.ui \
    PartsSource/WearingParts.ui \
    PartsSource/WearingPartsList.ui \
    PartsSource/WearingPartsListUser.ui \
    ProductForm/ProductDetailForm.ui \
    ProductForm/WidgetProdutData.ui \
    WidgetAutoPage.ui \
    WidgetBtnPanel.ui \
    WidgetCalibImage.ui \
    WidgetConsumables.ui \
    WidgetFlowState.ui \
    WidgetLogAll.ui \
    WidgetProduce.ui \
    WidgetProductChange.ui \
    WidgetSingleTest.ui \
    WidgetStatusBar.ui \
    WidgetTrayState.ui \
    WidgetVision.ui


RESOURCES += \
    resource/qss.qrc

LIBS += -lAdvapi32

INCLUDEPATH += $$PWD/../Common/CurveView
INCLUDEPATH += $$PWD/PartsSource

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

win32:CONFIG(release, debug|release): LIBS += -L$$(PATH_VIS)/qwt/ -lqwt
else:win32:CONFIG(debug, debug|release): LIBS += -L$$(PATH_VIS)/qwt/ -lqwtd

INCLUDEPATH += $$(PATH_VIS)/qwt/QtQWT
DEPENDPATH += $$(PATH_VIS)/qwt/QtQWT

win32:CONFIG(release, debug|release): LIBS += -L$$(PATH_VIS)/opencv/lib/ -lopencv_world411
else:win32:CONFIG(debug, debug|release): LIBS += -L$$(PATH_VIS)/opencv/lib/ -lopencv_world411d

INCLUDEPATH += $$(PATH_VIS)/opencv/include
DEPENDPATH += $$(PATH_VIS)/opencv/


win32:CONFIG(release, debug|release): LIBS += -L$$(PATH_VIS)/VisCommon/ -lVisCommon
else:win32:CONFIG(debug, debug|release): LIBS += -L$$(PATH_VIS)/VisCommon/ -lVisCommond

INCLUDEPATH += $$(PATH_VIS)/VisCommon
DEPENDPATH += $$(PATH_VIS)/VisCommon


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../3rd/VisMotorTool/ -lVisMotorTool
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../3rd/VisMotorTool/ -lVisMotorToold

INCLUDEPATH += $$PWD/../../3rd/VisMotorTool
DEPENDPATH += $$PWD/../../../3rd/VisMotorTool

INCLUDEPATH += -L$$PWD/../../3rd/VisAnomalyDetect
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../3rd/VisAnomalyDetect/ -lVisAnomalyDetect

win32: LIBS += -L$$PWD/../../3rd/VisCameraTool/ -lVisCameraTool
INCLUDEPATH += $$PWD/../../3rd/VisCameraTool
DEPENDPATH += $$PWD/../../3rd/VisCameraTool

win32: LIBS += -L$$PWD/../../3rd/VisDataMatrixFind/ -lVisDataMatrixFind
INCLUDEPATH += $$PWD/../../3rd/VisDataMatrixFind
DEPENDPATH += $$PWD/../../3rd/VisDataMatrixFind

DISTFILES +=
