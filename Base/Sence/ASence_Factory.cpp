#include "ASence_Factory.h"

void ASence_Factory::CreateMap(GSence* IOSence)
{
	GMap* Map = new GMap();
	IOSence->InsertMap(Map);
}

void ASence_Factory::SetGrids(GMap* IOMap,
	const Grid_Vector2 IStartposition,
	const Grid_Vector2 IEndPosition,
	const GGrid* IGrid)
{
	IOMap->SetGrids(IStartposition, IEndPosition, IGrid);
}

void ASence_Factory::CheckMap(GSence* IOSence)
{

}

void ASence_Factory::ReadSence(GSence* OSence)
{
	ReadResource(OSence);
	//...

}

void ASence_Factory::NewSence(GSence* OSence)
{
	OSence = new GSence();
	
	CreateMap(OSence);
	
}

void ASence_Factory::SaveSence(GSence* ISence)
{

	SaveResource(ISence);

}