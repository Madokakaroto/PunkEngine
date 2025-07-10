#pragma once

#ifndef _MSC_VER
#include "Math/Detail/sal.h"
#endif
#include "DirectXMath.h"
#include "Base/Reflection/StaticReflection.h"

namespace punk
{
    using float2 = DirectX::XMFLOAT2;
    using float3 = DirectX::XMFLOAT3;
    using float4 = DirectX::XMFLOAT4;
    using vector2 = DirectX::XMVECTOR;
    using vector3 = DirectX::XMVECTOR;
    using vector4 = DirectX::XMVECTOR;
    using matrix4x4 = DirectX::XMMATRIX;
}