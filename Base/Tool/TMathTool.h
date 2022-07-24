#pragma once 
#include <Windows.h>
#include <DirectXMath.h>
#include <cstdint>

struct GVertex
{
	DirectX::XMFLOAT3 Position{0,0,0};
	DirectX::XMFLOAT3 Normal{0,0,0};
	DirectX::XMFLOAT2 TexC{0,0};
	DirectX::XMFLOAT3 TangentU{0,0,0};
	DirectX::XMINT2 MaterialId{0,0};

	GVertex()
	{
		Position = { 0,0,0 };
		Normal = { 0,0,0 };
		TexC = { 0,0 };
		TangentU = { 0,0,0 };
		MaterialId = { 0,0 };
	}

	GVertex(
		const DirectX::XMFLOAT3& p,
		const DirectX::XMFLOAT3& n,
		const DirectX::XMFLOAT3& t,
		const DirectX::XMFLOAT2& uv,
		const DirectX::XMINT2& mid) :
		Position(p),
		Normal(n),
		TangentU(t),
		TexC(uv),
		MaterialId(mid){}
	GVertex(
		float px, float py, float pz,
		float nx, float ny, float nz,
		float tx, float ty, float tz,
		float u, float v,
		int id) :
		Position(px, py, pz),
		Normal(nx, ny, nz),
		TangentU(tx, ty, tz),
		TexC(u, v),
		MaterialId(id,0){}

};


//位置单位
template<typename T>
class AVector2
{
public:
	T X = 0.0f;
	T Y = 0.0f;

	AVector2<T>& operator=(const AVector2<T>& IVector)
	{
		X = IVector.X;
		Y = IVector.Y;
		return *this;
	}

	AVector2<T>& operator+=(const AVector2<T>& IVector)
	{
		X += IVector.X;
		Y += IVector.Y;
		return *this;
	}

	AVector2<T>& operator-(const AVector2<T>& IVector)
	{
		X -= IVector.X;
		Y -= IVector.Y;
		return *this;
	}



	T& operator[](int IIndex)
	{
		switch (IIndex)
		{
		case 0:
			return X;
		case 1:
			return Y;
		default:
			return *(&X + IIndex);
		}
	}


	AVector2(T IX, T IY) :X(IX), Y(IY) {}

	AVector2() :X(0), Y(0) {};

};

class Position_Vector2 : public AVector2<float>
{
public:
	Position_Vector2(float IX, float IY) :AVector2<float>(X, Y) {};
	Position_Vector2(const AVector2<float>& IVector) :AVector2<float>(X, Y) {};
	Position_Vector2() :AVector2<float>() {};

};

class Grid_Vector2 : public AVector2<int>
{
public:
	Grid_Vector2(int IX, int IY) :AVector2<int>(X, Y) {};
	Grid_Vector2() :AVector2<int>() {};

};

//PI
class AAngle
{
public:
	float angle = 0.0f;

	AAngle& operator=(const AAngle& IAngle)
	{
		angle = IAngle.angle;
	}

	AAngle& operator=(const float& IAngle)
	{
		angle = IAngle;
	}

	AAngle& operator=(const Position_Vector2& IVector)
	{
		angle = atan(IVector.Y / IVector.X);
	}

	AAngle(const Position_Vector2& IVector)
	{
		angle = atan(IVector.Y / IVector.X);
	}
};



/// <summary>
/// 求交
/// </summary>
/// <param name="P1">线1点</param>
/// <param name="L1">线1方向</param>
/// <param name="P2">线2点</param>
/// <param name="L2">线2方向</param>
/// <param name="OResposition">交点</param>
/// <returns>是否交</returns>
bool Intersection(
	Position_Vector2 P1,
	Position_Vector2 L1,
	Position_Vector2 P2,
	Position_Vector2 L2,
	Position_Vector2& OResposition);

/// <summary>
/// 矩阵插值
/// </summary>
/// <param name="IMatrixA"></param>
/// <param name="IMatrixB"></param>
/// <param name="IF"></param>
/// <returns></returns>
DirectX::XMMATRIX MatrixInterpolation(
	DirectX::XMMATRIX IMatrixA,
	DirectX::XMMATRIX IMatrixB,
	float IF);


class MathHelper
{
public:
	// Returns random float in [0, 1).
	static float RandF()
	{
		return (float)(rand()) / (float)RAND_MAX;
	}

	// Returns random float in [a, b).
	static float RandF(float a, float b)
	{
		return a + RandF() * (b - a);
	}

	static int Rand(int a, int b)
	{
		return a + rand() % ((b - a) + 1);
	}

	template<typename T>
	static T Min(const T& a, const T& b)
	{
		return a < b ? a : b;
	}

	template<typename T>
	static T Max(const T& a, const T& b)
	{
		return a > b ? a : b;
	}

	template<typename T>
	static T Lerp(const T& a, const T& b, float t)
	{
		return a + (b - a) * t;
	}

	template<typename T>
	static T Clamp(const T& x, const T& low, const T& high)
	{
		return x < low ? low : (x > high ? high : x);
	}

	// Returns the polar angle of the point (x,y) in [0, 2*PI).
	static float AngleFromXY(float x, float y);

	static DirectX::XMVECTOR SphericalToCartesian(float radius, float theta, float phi)
	{
		return DirectX::XMVectorSet(
			radius * sinf(phi) * cosf(theta),
			radius * cosf(phi),
			radius * sinf(phi) * sinf(theta),
			1.0f);
	}

	static DirectX::XMMATRIX InverseTranspose(DirectX::CXMMATRIX M)
	{
		// Inverse-transpose is just applied to normals.  So zero out 
		// translation row so that it doesn't get into our inverse-transpose
		// calculation--we don't want the inverse-transpose of the translation.
		DirectX::XMMATRIX A = M;
		A.r[3] = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

		DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(A);
		return DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&det, A));
	}

	static DirectX::XMFLOAT4X4 Identity4x4()
	{
		static DirectX::XMFLOAT4X4 I(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);

		return I;
	}

	static UINT CalcConstantBufferByteSize(UINT byteSize)
	{
		// Constant buffers must be a multiple of the minimum hardware
		// allocation size (usually 256 bytes).  So round up to nearest
		// multiple of 256.  We do this by adding 255 and then masking off
		// the lower 2 bytes which store all bits < 256.
		// Example: Suppose byteSize = 300.
		// (300 + 255) & ~255
		// 555 & ~255
		// 0x022B & ~0x00ff
		// 0x022B & 0xff00
		// 0x0200
		// 512
		return (byteSize + 255) & ~255;
	}

	static DirectX::XMVECTOR RandUnitVec3();
	static DirectX::XMVECTOR RandHemisphereUnitVec3(DirectX::XMVECTOR n);

	static const float Infinity;
	static const float Pi;


};

