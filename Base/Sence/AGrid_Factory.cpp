#include "AGrid_Factory.h"

void AGrid_Factory::NewGrid(GGrid* OGrid)
{
	//�H�H�H㞖R���n���͌^
	OGrid->Staticmodel = new AStatic_Model();
	
}

void AGrid_Factory::ReadGrid(GGrid* OGrid)
{
	ReadResource(OGrid);
}

void AGrid_Factory::SaveGrid(GGrid* IGrid)
{
	SaveResource(IGrid);
}

void AGrid_Factory::SetAttribe(GGrid* IOGrid)
{

}