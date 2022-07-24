#pragma once
#include "Base/ABase_Factory.h"
#include "GWeapon.h"

class AWeapon_Factory : public ABase_Factory<GWeapon>
{
	void NewWeapon(GWeapon* OGWeapon);

	void ReadWeapon(GWeapon* OGWeapon);

	void SaveWeapon(GWeapon* IGWeapon);

};