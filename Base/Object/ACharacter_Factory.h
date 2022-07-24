#pragma once
#include "Base/ABase_Factory.h"
#include "GCharacter.h"

class ACharacter_Factory : public ABase_Factory<GCharacter>
{
	void NewCharacter(GCharacter* OGCharacter);

	void ReadCharacter(GCharacter* OGCharacter);

	void SaveCharacter(GCharacter* IGCharacter);

};