// macOS stand-in for the Windows header of the same name.
// Outside the renderer the engine uses two D3DX helpers: the size of a flexible
// vertex format and a 4x4 matrix inverse. Both are implemented here.
#pragma once
#include "d3d9.h"

typedef struct D3DXMATRIX
{
	float m[4][4];
} D3DXMATRIX;

// Size in bytes of a vertex in the given flexible vertex format
inline UINT D3DXGetFVFVertexSize(DWORD fvf)
{
	UINT size = 0;
	switch (fvf & D3DFVF_POSITION_MASK)
	{
	case D3DFVF_XYZ: size += 3 * sizeof(float); break;
	case D3DFVF_XYZRHW: size += 4 * sizeof(float); break;
	case 0x006: size += 4 * sizeof(float); break; // XYZB1
	case 0x008: size += 5 * sizeof(float); break; // XYZB2
	case 0x00a: size += 6 * sizeof(float); break; // XYZB3
	case 0x00c: size += 7 * sizeof(float); break; // XYZB4
	case 0x00e: size += 8 * sizeof(float); break; // XYZB5
	case 0x4002: size += 4 * sizeof(float); break; // XYZW
	default: break;
	}
	if (fvf & D3DFVF_NORMAL) size += 3 * sizeof(float);
	if (fvf & D3DFVF_PSIZE) size += sizeof(float);
	if (fvf & D3DFVF_DIFFUSE) size += sizeof(DWORD);
	if (fvf & D3DFVF_SPECULAR) size += sizeof(DWORD);

	// two bits per texture coordinate set from bit 16: 0 = 2D, 1 = 3D, 2 = 4D, 3 = 1D
	UINT tex_count = (fvf & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
	for (UINT i = 0; i < tex_count; ++i)
	{
		static const UINT components[4] = {2, 3, 4, 1};
		size += components[(fvf >> (16 + i * 2)) & 3] * sizeof(float);
	}
	return size;
}

// Inverse of a general 4x4 matrix by cofactor expansion; returns nullptr if singular
inline D3DXMATRIX* D3DXMatrixInverse(D3DXMATRIX* out, float* determinant, const D3DXMATRIX* in)
{
	const float* a = &in->m[0][0];
	float inv[16];

	inv[0] = a[5] * a[10] * a[15] - a[5] * a[11] * a[14] - a[9] * a[6] * a[15] + a[9] * a[7] * a[14] + a[13] * a[6] * a[11] - a[13] * a[7] * a[10];
	inv[4] = -a[4] * a[10] * a[15] + a[4] * a[11] * a[14] + a[8] * a[6] * a[15] - a[8] * a[7] * a[14] - a[12] * a[6] * a[11] + a[12] * a[7] * a[10];
	inv[8] = a[4] * a[9] * a[15] - a[4] * a[11] * a[13] - a[8] * a[5] * a[15] + a[8] * a[7] * a[13] + a[12] * a[5] * a[11] - a[12] * a[7] * a[9];
	inv[12] = -a[4] * a[9] * a[14] + a[4] * a[10] * a[13] + a[8] * a[5] * a[14] - a[8] * a[6] * a[13] - a[12] * a[5] * a[10] + a[12] * a[6] * a[9];
	inv[1] = -a[1] * a[10] * a[15] + a[1] * a[11] * a[14] + a[9] * a[2] * a[15] - a[9] * a[3] * a[14] - a[13] * a[2] * a[11] + a[13] * a[3] * a[10];
	inv[5] = a[0] * a[10] * a[15] - a[0] * a[11] * a[14] - a[8] * a[2] * a[15] + a[8] * a[3] * a[14] + a[12] * a[2] * a[11] - a[12] * a[3] * a[10];
	inv[9] = -a[0] * a[9] * a[15] + a[0] * a[11] * a[13] + a[8] * a[1] * a[15] - a[8] * a[3] * a[13] - a[12] * a[1] * a[11] + a[12] * a[3] * a[9];
	inv[13] = a[0] * a[9] * a[14] - a[0] * a[10] * a[13] - a[8] * a[1] * a[14] + a[8] * a[2] * a[13] + a[12] * a[1] * a[10] - a[12] * a[2] * a[9];
	inv[2] = a[1] * a[6] * a[15] - a[1] * a[7] * a[14] - a[5] * a[2] * a[15] + a[5] * a[3] * a[14] + a[13] * a[2] * a[7] - a[13] * a[3] * a[6];
	inv[6] = -a[0] * a[6] * a[15] + a[0] * a[7] * a[14] + a[4] * a[2] * a[15] - a[4] * a[3] * a[14] - a[12] * a[2] * a[7] + a[12] * a[3] * a[6];
	inv[10] = a[0] * a[5] * a[15] - a[0] * a[7] * a[13] - a[4] * a[1] * a[15] + a[4] * a[3] * a[13] + a[12] * a[1] * a[7] - a[12] * a[3] * a[5];
	inv[14] = -a[0] * a[5] * a[14] + a[0] * a[6] * a[13] + a[4] * a[1] * a[14] - a[4] * a[2] * a[13] - a[12] * a[1] * a[6] + a[12] * a[2] * a[5];
	inv[3] = -a[1] * a[6] * a[11] + a[1] * a[7] * a[10] + a[5] * a[2] * a[11] - a[5] * a[3] * a[10] - a[9] * a[2] * a[7] + a[9] * a[3] * a[6];
	inv[7] = a[0] * a[6] * a[11] - a[0] * a[7] * a[10] - a[4] * a[2] * a[11] + a[4] * a[3] * a[10] + a[8] * a[2] * a[7] - a[8] * a[3] * a[6];
	inv[11] = -a[0] * a[5] * a[11] + a[0] * a[7] * a[9] + a[4] * a[1] * a[11] - a[4] * a[3] * a[9] - a[8] * a[1] * a[7] + a[8] * a[3] * a[5];
	inv[15] = a[0] * a[5] * a[10] - a[0] * a[6] * a[9] - a[4] * a[1] * a[10] + a[4] * a[2] * a[9] + a[8] * a[1] * a[6] - a[8] * a[2] * a[5];

	float det = a[0] * inv[0] + a[1] * inv[4] + a[2] * inv[8] + a[3] * inv[12];
	if (determinant)
		*determinant = det;
	if (det == 0.f)
		return nullptr;

	float* o = &out->m[0][0];
	for (int i = 0; i < 16; ++i)
		o[i] = inv[i] / det;
	return out;
}
