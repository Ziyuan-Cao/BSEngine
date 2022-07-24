#include "TMathTool.h"

const float MathHelper::Infinity = FLT_MAX;
const float MathHelper::Pi = 3.1415926535f;

bool Intersection(
	Position_Vector2 P1,
	Position_Vector2 L1,
	Position_Vector2 P2,
	Position_Vector2 L2,
	Position_Vector2& OResposition)
{
	float x1 = P1.X;
	float y1 = P1.Y;
	float x2 = P2.X;
	float y2 = P2.Y;

	//float u1 = L1.X;
	//float v1 = L1.Y;
	//float u2 = L2.X;
	//float v2 = L2.Y;

	float a1 = L1.Y;
	float b1 = -L1.X;
	float a2 = L2.Y;
	float b2 = -L2.X;

	float A1 = a1;
	float B1 = b1;
	float C1 = -a1 * x1 - b1 * y1;
	float A2 = a2;
	float B2 = b2;
	float C2 = -a2 * x2 - b2 * y2;

	if (A2 * B1 - A1 * B2 == 0)
	{
		return false;
	}
	else
	{
		float Rx = (B2 * C1 - B1 * C2) / (A2 * B1 - A1 * B2);
		float Ry = (A1 * C2 - A2 * C2) / (A2 * B1 - A1 * B2);
		OResposition = { Rx,Ry };
		return true;
	}




}

DirectX::XMMATRIX MatrixInterpolation(
	DirectX::XMMATRIX IMatrixA, 
	DirectX::XMMATRIX IMatrixB,
	float IF)
{
	DirectX::XMMATRIX Result;

	Result = IMatrixB - IMatrixA;
	Result = Result * IF;
	Result = IMatrixA + Result;

	return Result;
}
