#ifndef PCBCLEANNODE_H
#define PCBCLEANNODE_H

#include <QObject>
#include "ParamDef.h"

class PCBCleanNode
{

public:
	PCBCleanNode(ModuleInfo* item, int station);

public:
	int  Process();

protected:
	int        GrabPCB();
	int        CleanPCB();
	int        PlacePCB();
protected:
	int        SetGripClose(bool close);
	int        CheckGripModuleExist(bool& exist);

protected:
	ModuleInfo* m_item = nullptr;
	int  m_station = 0;

};

#endif // PCBCLEANNODE_H
