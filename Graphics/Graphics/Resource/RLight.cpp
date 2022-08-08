#include "RLight.h"


void RLight::GetLightMatrix(DirectX::BoundingSphere const& ISphereBound, LightNFWVPT& OLightNFWVPT)
{
    // Only the first "main" light casts a shadow.
    XMVECTOR lightDir = XMLoadFloat3(&Lightdata.Direction);
    XMVECTOR lightPos = -2.0f * ISphereBound.Radius * lightDir;
    XMVECTOR targetPos = XMLoadFloat3(&ISphereBound.Center);
    XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMMATRIX lightView = XMMatrixLookAtLH(lightPos, targetPos, lightUp);

    XMStoreFloat3(&OLightNFWVPT.mLightPosW, lightPos);

    // Transform bounding sphere to light space.
    XMFLOAT3 sphereCenterLS;
    XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, lightView));

    // Ortho frustum in light space encloses scene.
    float l = sphereCenterLS.x - ISphereBound.Radius;
    float b = sphereCenterLS.y - ISphereBound.Radius;
    float n = sphereCenterLS.z - ISphereBound.Radius;
    float r = sphereCenterLS.x + ISphereBound.Radius;
    float t = sphereCenterLS.y + ISphereBound.Radius;
    float f = sphereCenterLS.z + ISphereBound.Radius;

    OLightNFWVPT.mLightNearZ = n;
    OLightNFWVPT.mLightFarZ = f;
    XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

    // Transform NDC space [-1,+1]^2 to texture space [0,1]^2
    XMMATRIX T(
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f);

    XMMATRIX S = -lightView * lightProj * T;
    XMStoreFloat4x4(&OLightNFWVPT.mLightView, lightView);
    XMStoreFloat4x4(&OLightNFWVPT.mLightProj, lightProj);
    XMStoreFloat4x4(&OLightNFWVPT.mShadowTransform, S);
}