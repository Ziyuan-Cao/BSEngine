#pragma once
#include "Base/ABase_Factory.h"
#include "GMonster.h"

class AMonster_Factory : public ABase_Factory<GMonster>
{
	void NewMonster(GMonster* OGMonster);

	void ReadMonster(GMonster* OGMonster);

	void SaveMonster(GMonster* IGMonster);

};