#pragma once

#include <assert.h>
#include <d3d9.h>
#include <d3dx9math.h>
#include <string>
#include <d3dx9core.h>
#include "DirectXShell.h"


class Model
{
	public:
		static Model* Load(const char* Path, const char* Filename)
		{
			Model *pModel = new Model;

			char text[255];

			sprintf(text,"%s%s",Path,Filename);

			HRESULT hr=D3DXLoadMeshFromX(text, D3DXMESH_SYSTEMMEM,
				DirectXShell::g_pd3dDevice, NULL,
				&pModel->materialBuffer,NULL, &pModel->numMaterials,
				&pModel->mesh );

			if(pModel->mesh != NULL)
			{
				pModel->meshMaterials = new D3DMATERIAL9[pModel->numMaterials];
				pModel->meshTextures  = new LPDIRECT3DTEXTURE9[pModel->numMaterials];

				D3DXMATERIAL* d3dxMaterials = (D3DXMATERIAL*)pModel->materialBuffer->GetBufferPointer();

				for (DWORD i=0; i<pModel->numMaterials; i++)
				{
					pModel->meshMaterials[i] = d3dxMaterials[i].MatD3D;
					pModel->meshMaterials[i].Ambient = pModel->meshMaterials[i].Diffuse;

					pModel->meshTextures[i] = NULL;
					if (d3dxMaterials[i].pTextureFilename)
					{
						sprintf(text,"%s%s",Path, d3dxMaterials[i].pTextureFilename);
						D3DXCreateTextureFromFile(DirectXShell::g_pd3dDevice, text,     &pModel->meshTextures[i]) ;
					}
				}
				pModel->CalculateBoundingVolume();
				return pModel;
			}

			delete pModel;
			return NULL;
		}

		void Render(const D3DXMATRIX* world = NULL)
		{
			if(world)
			{
				DirectXShell::g_pd3dDevice->SetTransform(D3DTS_WORLD,world);
			}

			for (DWORD i=0; i<numMaterials; i++)
			{
				DirectXShell::g_pd3dDevice->SetMaterial(&meshMaterials[i]);
				DirectXShell::g_pd3dDevice->SetTexture(0,meshTextures[i]);

				mesh->DrawSubset( i );
			}
		}
		bool IsScreenPointInMe(const D3DXMATRIX& transform,const D3DXVECTOR2& screenPos)
		{
			D3DXVECTOR3 result;

			D3DXVECTOR2 mouseInRenderSpace;

			D3DVIEWPORT9 viewport;

			DirectXShell::g_pd3dDevice->GetViewport(&viewport);

			mouseInRenderSpace.x = (2.0f * screenPos.x / viewport.Width) - 1.0f;
			mouseInRenderSpace.y = (2.0f * (viewport.Height-screenPos.y) / viewport.Height) - 1.0f;


			for(int i=0;i<ARRAY_LENGTH(triIndices);i+=3)
			{
				D3DXVECTOR2 v[3];
				for(int j=0;j<3;j++)
				{
					D3DXVECTOR4 out;

					D3DXVec3Transform(&out,&corners[triIndices[i+j]],&transform);
				
					v[j].x = out.x/out.w;
					v[j].y = out.y/out.w;
				}	

				if(PointInTriangle(&mouseInRenderSpace,&v[0],&v[1],&v[2]) == true)
				{
					return true;
				}
			}

			return false;
		}


		void RenderBoundingVolume(const D3DXMATRIX* world = NULL,D3DXCOLOR col = D3DCOLOR_XRGB(0,255,0))
		{
			if(world)
			{
				DirectXShell::g_pd3dDevice->SetTransform(D3DTS_WORLD,world);
			}

			LineList3D(&corners[0],ARRAY_LENGTH(corners),lineIndices,ARRAY_LENGTH(lineIndices), col); 		
			//TriList3D(&corners[0],ARRAY_LENGTH(corners),triIndices,ARRAY_LENGTH(triIndices), col); 		
		}

		D3DMATERIAL9* GetMaterial(int index)
		{
			assert(index < (int)numMaterials);
			return &meshMaterials[index];
		}

	private:

		void Line3D(const D3DXVECTOR3& v0,const D3DXVECTOR3& v1, DWORD col)
		{
			DirectXShell::PosColor vert[2];

			vert[0].pos = v0; vert[0].color = col;
			vert[1].pos = v1; vert[1].color = col;
				
			DirectXShell::g_pd3dDevice->SetVertexDeclaration(DirectXShell::PosColor::vertexDeclaration);
			DirectXShell::g_pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST,1,vert,sizeof(DirectXShell::PosColor));
		}

		void LineList3D(const D3DXVECTOR3* points,int vertCount,int* indicies, int indexCount,DWORD col)
		{
			DirectXShell::PosColor* vert = new DirectXShell::PosColor[vertCount];

			for(int i=0;i<vertCount;i++)
			{
				vert[i].pos = points[i]; vert[i].color = col;
			}

			DirectXShell::g_pd3dDevice->SetVertexDeclaration(DirectXShell::PosColor::vertexDeclaration);
			DirectXShell::g_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_LINELIST,0,vertCount,indexCount/2,indicies,D3DFMT_INDEX32,vert,sizeof(DirectXShell::PosColor) );

			delete[] vert;
		}

		void TriList3D(const D3DXVECTOR3* points,int vertCount,int* indicies, int indexCount,DWORD col)
		{
			DirectXShell::PosColor* vert = new DirectXShell::PosColor[vertCount];

			for(int i=0;i<vertCount;i++)
			{
				vert[i].pos = points[i]; vert[i].color = col;
			}

			DirectXShell::g_pd3dDevice->SetVertexDeclaration(DirectXShell::PosColor::vertexDeclaration);
			DirectXShell::g_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,vertCount,indexCount/3,indicies,D3DFMT_INDEX32,vert,sizeof(DirectXShell::PosColor) );

			delete[] vert;
		}

		bool PointInTriangle(D3DXVECTOR2* P, D3DXVECTOR2* A, D3DXVECTOR2* B, D3DXVECTOR2* C)
		{
			D3DXVECTOR2 v0 = *C - *A;
			D3DXVECTOR2 v1 = *B - *A;
			D3DXVECTOR2 v2 = *P - *A;

			float dot00 = D3DXVec2Dot(&v0,&v0);
			float dot01 = D3DXVec2Dot(&v0,&v1);
			float dot02 = D3DXVec2Dot(&v0,&v2);
			float dot11 = D3DXVec2Dot(&v1,&v1);
			float dot12 = D3DXVec2Dot(&v1,&v2);

			float invDenom = 1 / (dot00 * dot11 - dot01 * dot01);

			float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
			float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

			return (u >0) && (v>0) && (u + v < 1);
		}


		D3DXVECTOR3 corners[8];
		int			triIndices[6*2*3];
		int			lineIndices[12*2];

		void CalculateBoundingVolume()
		{
			D3DXVECTOR3 minVolume,maxVolume;

			minVolume.x = FLT_MAX;
			minVolume.y = FLT_MAX;
			minVolume.z = FLT_MAX;

			maxVolume.x = -FLT_MAX;
			maxVolume.y = -FLT_MAX;
			maxVolume.z = -FLT_MAX;

			LPDIRECT3DVERTEXBUFFER9 pVB;

			mesh->GetVertexBuffer(&pVB);

			if(pVB)
			{
				DirectXShell::PosNormalUV0 *pVert;
				D3DVERTEXBUFFER_DESC desc;
				pVB->GetDesc(&desc);

				pVB->Lock(0,desc.Size,(void**)&pVert,0);

				for(int iVert = 0;iVert<(int)(desc.Size / sizeof(DirectXShell::PosNormalUV0));iVert++)
				{
					if(pVert[iVert].pos.x > maxVolume.x)	maxVolume.x = pVert[iVert].pos.x;
					if(pVert[iVert].pos.y > maxVolume.y)	maxVolume.y = pVert[iVert].pos.y;
					if(pVert[iVert].pos.z > maxVolume.z)	maxVolume.z = pVert[iVert].pos.z;

					if(pVert[iVert].pos.x < minVolume.x)	minVolume.x = pVert[iVert].pos.x;
					if(pVert[iVert].pos.y < minVolume.y)	minVolume.y = pVert[iVert].pos.y;
					if(pVert[iVert].pos.z < minVolume.z)	minVolume.z = pVert[iVert].pos.z;
				}

				pVB->Unlock();	

				maxVolume = minVolume + (maxVolume-minVolume);
			}


			corners[0] = D3DXVECTOR3(minVolume.x,minVolume.y,minVolume.z);
			corners[1] = D3DXVECTOR3(maxVolume.x,minVolume.y,minVolume.z);
			corners[2] = D3DXVECTOR3(minVolume.x,minVolume.y,maxVolume.z);
			corners[3] = D3DXVECTOR3(maxVolume.x,minVolume.y,maxVolume.z);

			corners[4] = D3DXVECTOR3(minVolume.x,maxVolume.y,minVolume.z);
			corners[5] = D3DXVECTOR3(maxVolume.x,maxVolume.y,minVolume.z);
			corners[6] = D3DXVECTOR3(minVolume.x,maxVolume.y,maxVolume.z);
			corners[7] = D3DXVECTOR3(maxVolume.x,maxVolume.y,maxVolume.z);


			lineIndices[0] = 0;	lineIndices[1] = 1;
			lineIndices[2] = 1;	lineIndices[3] = 3;
			lineIndices[4] = 3;	lineIndices[5] = 2;
			lineIndices[6] = 2;	lineIndices[7] = 0;

			lineIndices[8] = 4;	lineIndices[9] = 5;
			lineIndices[10] = 5;	lineIndices[11] = 7;
			lineIndices[12] = 7;	lineIndices[13] = 6;
			lineIndices[14] = 6;	lineIndices[15] = 4;

			lineIndices[16] = 0;	lineIndices[17] = 4;
			lineIndices[18] = 1;	lineIndices[19] = 5;
			lineIndices[20] = 3;	lineIndices[21] = 7;
			lineIndices[22] = 2;	lineIndices[23] = 6;

			int* pTriIndex = &triIndices[0];

			*pTriIndex++ = 4; *pTriIndex++ = 6; *pTriIndex++ = 0;
			*pTriIndex++ = 6; *pTriIndex++ = 2; *pTriIndex++ = 0;

			*pTriIndex++ = 6; *pTriIndex++ = 7; *pTriIndex++ = 2;
			*pTriIndex++ = 7; *pTriIndex++ = 3; *pTriIndex++ = 2;

			*pTriIndex++ = 7; *pTriIndex++ = 5; *pTriIndex++ = 1;
			*pTriIndex++ = 5; *pTriIndex++ = 1; *pTriIndex++ = 3;

			*pTriIndex++ = 5; *pTriIndex++ = 4; *pTriIndex++ = 0;
			*pTriIndex++ = 5; *pTriIndex++ = 0; *pTriIndex++ = 1;

			*pTriIndex++ = 4; *pTriIndex++ = 5; *pTriIndex++ = 6;
			*pTriIndex++ = 5; *pTriIndex++ = 7; *pTriIndex++ = 6;

			*pTriIndex++ = 1; *pTriIndex++ = 0; *pTriIndex++ = 2;
			*pTriIndex++ = 1; *pTriIndex++ = 2; *pTriIndex++ = 3;
		}
	//private:

		LPD3DXBUFFER materialBuffer;
		DWORD numMaterials;   
		LPD3DXMESH mesh;

		D3DMATERIAL9 *meshMaterials;
		LPDIRECT3DTEXTURE9 *meshTextures;
};
