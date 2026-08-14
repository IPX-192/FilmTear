# Define the export macro
QT +=printsupport
INCLUDEPATH +=$$PWD/Product
HEADERS += \
    $$PWD/../../Common/CurveView/Chart/CustomChart.h \
    $$PWD/../../Common/CurveView/Chart/qcustomplot.h \
    $$PWD/ChartNGSort.h \
    $$PWD/ChartProduceHour.h \
    $$PWD/ProductDef.h \
    $$PWD/TableNGSort.h \
    $$PWD/TableProduct.h \
    $$PWD/WidgetProductTotal.h \
    $$PWD/WidgetStationProduct.h


SOURCES += \
   $$PWD/../../Common/CurveView/Chart/CustomChart.cpp \
   $$PWD/../../Common/CurveView/Chart/qcustomplot.cpp \
   $$PWD/ChartNGSort.cpp \
   $$PWD/ChartProduceHour.cpp \
   $$PWD/TableNGSort.cpp \
   $$PWD/TableProduct.cpp \
   $$PWD/WidgetProductTotal.cpp \
   $$PWD/WidgetStationProduct.cpp

FORMS += \
    $$PWD/ChartNGSort.ui \
    $$PWD/ChartProduceHour.ui \
    $$PWD/TableNGSort.ui \
    $$PWD/TableProduct.ui \
    $$PWD/WidgetProductTotal.ui \
    $$PWD/WidgetStationProduct.ui

