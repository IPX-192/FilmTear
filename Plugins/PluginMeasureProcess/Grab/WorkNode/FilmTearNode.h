#ifndef FILMTEARNODE_H
#define FILMTEARNODE_H

#include <QObject>
#include "ParamDef.h"

class FilmTearNode
{
public:
    FilmTearNode(ModuleInfo* item,int station);

public:
	int Process();

protected:
	int  GrabPCB();
	int  FilmTear();
	int  PlacePCB();
    int  SetGripClose(bool close);
    int  CheckGripModuleExist(bool& exist);
    int  CheckFilmTearResult();
    int  SaveFilmDataToCsv();

protected:
    ModuleInfo* m_item = nullptr;
	int  m_station = 0;
};

#endif // FILMTEARNODE_H
