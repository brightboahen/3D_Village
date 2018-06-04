#pragma once

#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <d3d9.h>
#include <d3dx9math.h>
#include <string>
#include <d3dx9core.h>

#define ARRAY_LENGTH(x) (sizeof(x)/sizeof(x[0]))

class DirectXShell
{
	public:

		static bool Init(HWND hWnd);
		static void Render();		
		static void Shutdown();
		

		static void Print(int x, int y, char* str);
		
		static LPDIRECT3D9             g_pD3D;
		static LPDIRECT3DDEVICE9       g_pd3dDevice;
		static ID3DXFont*			   pFont; 

		/*struct CUSTOMVERTEX
		{
			D3DXVECTOR3 pos;      
			DWORD color;        
		};*/

		struct PosColor
		{
			D3DXVECTOR3 pos;
			DWORD color;

			static IDirect3DVertexDeclaration9* vertexDeclaration;
		};

		struct PosUV0
		{
			D3DXVECTOR3 pos;   
			D3DXVECTOR2 uv0;      

			static IDirect3DVertexDeclaration9* VertexDeclaration;
		};

		struct PosNormal
		{
			D3DXVECTOR3 pos;   
			D3DXVECTOR3 normal;      

			static IDirect3DVertexDeclaration9* VertexDeclaration;
		};

		struct PosNormalUV0
		{
			D3DXVECTOR3 pos;   
			D3DXVECTOR3 normal;     
			D3DXVECTOR2 uv0;      

			static IDirect3DVertexDeclaration9* VertexDeclaration;
		};

		/*static IDirect3DVertexDeclaration9* vertexDecleration;*/
		static RECT	sRect;

		struct TVertex
		{
			float x, y, z;		// 3D world position
			float tu, tv;		// Texture coordinates
		};
};