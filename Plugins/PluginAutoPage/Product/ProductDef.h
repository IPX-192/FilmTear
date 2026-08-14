#ifndef PRODUCTDEF_H
#define PRODUCTDEF_H

#include <QMap>

struct StationInfo {
    int  station = 0;
    int  numTotal = 0;    //投入数
    int  numOK = 0;       //良品数
    int  numNG = 0;       //NG数
    double  timeCT = 0;      //CT时间
    QMap<QString, int> ngInfo;   //NG项信息
    QMap<int, int>numHour;   //时段产出
    StationInfo* ptrTotal = nullptr;  //总工位信息
};

struct ProductTask{
    bool result=false;   //测试结果
    int  station=0;      //工位
    QString errItem;     //错误项
    int  ctTime=0;       //CT时间
};



#endif // PRODUCTDEF_H
