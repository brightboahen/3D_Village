
//Bright Boahen w1278020
//************************
#include "stdafx.h"
#include "DirectXShell.h" // direct x class 
#include "Model.h" // model class
#include "Timer.h" // timer class

// pointers to directx
LPDIRECT3D9             DirectXShell::g_pD3D = NULL;
LPDIRECT3DDEVICE9       DirectXShell::g_pd3dDevice=NULL;

ID3DXFont*  DirectXShell::pFont=NULL; 
RECT DirectXShell::sRect;

IDirect3DVertexDeclaration9* DirectXShell::PosColor::vertexDeclaration = NULL;


IDirect3DVertexDeclaration9* DirectXShell::PosUV0::VertexDeclaration = NULL;
IDirect3DVertexDeclaration9* DirectXShell::PosNormal::VertexDeclaration = NULL;
IDirect3DVertexDeclaration9* DirectXShell::PosNormalUV0::VertexDeclaration = NULL;

IDirect3DTexture9* myTexture = NULL;
LPDIRECT3DVERTEXBUFFER9		g_pVertexBuffer	= NULL; // vertex buffer to store vertices
LPDIRECT3DTEXTURE9			g_SkyTextures[6];// an array for skybox textures


D3DXMATRIX  g_matView; // a global variable for the viewing matrix

//screen width and height
const int					SCREEN_WIDTH		= 1024; 
const int					SCREEN_HEIGHT		= 768;

//timer variable
Timer frameTime;

char str[255];

HRESULT hRet;

D3DXIMAGE_INFO info;

#define   FVF_FLAGS			D3DFVF_XYZ | D3DFVF_TEX1 // flags to stream resources



#include "BinaryReader.h"

void SetColour(D3DCOLORVALUE& v, float r, float g, float b, float a=1.0f)
{
	v.r = r;
	v.g = g;
	v.b = b;
	v.a = a;
}

// teapot and box mesh variables
LPD3DXMESH pTeapot;
LPD3DXMESH pBox;

// variables to import models
Model *house = NULL;
Model *table = NULL;
Model *church = NULL;
Model *barrel = NULL;
Model *chair = NULL;
Model *tree = NULL;


IDirect3DTexture9* LoadTextureResource(const char* Filename)
{
	BinaryReader reader(Filename);

	if(reader.IsOpen() == false)
	{
		return NULL;
	}

	int width = reader.ReadInt32();
	int height = reader.ReadInt32();
	int size = reader.ReadInt32();

	char* pData = reader.ReadBytes(size);

	IDirect3DTexture9* pTexture = NULL;
	DirectXShell::g_pd3dDevice->CreateTexture(width
												,height
												,1
												,0
												,D3DFMT_X8R8G8B8
												,D3DPOOL_MANAGED
												,&pTexture
												,NULL
												);

	if(pTexture)
	{
		D3DLOCKED_RECT out;

		if(pTexture->LockRect(0,&out,NULL,0) == D3D_OK)
		{
			memcpy(out.pBits,pData,size);
			pTexture->UnlockRect(0);
					
			delete[] pData;
			return pTexture;
		}
	}

	return NULL;
}

// function to render meshes(teapot and box)
void RenderD3DXMESH(const LPD3DXMESH pMesh)
{
	LPDIRECT3DVERTEXBUFFER9 pVB;
	LPDIRECT3DINDEXBUFFER9  pIB;

	int FVF;
	int FaceCount;

	pMesh->GetVertexBuffer(&pVB);
	pMesh->GetIndexBuffer(&pIB);
	FVF = pMesh->GetFVF();

	FaceCount = pMesh->GetNumFaces();

	DirectXShell::g_pd3dDevice->SetFVF(FVF);
	DirectXShell::g_pd3dDevice->SetStreamSource( 0, pVB, 0, 6*4 );
	DirectXShell::g_pd3dDevice->SetIndices(pIB);

	DirectXShell::g_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST
													, 0,0
													, pMesh->GetNumVertices()
													, 0
													, pMesh->GetNumFaces());
}

// initialising the direct x devices
bool DirectXShell::Init(HWND hWnd)
{
	g_pD3D = Direct3DCreate9( D3D_SDK_VERSION );

	if(g_pD3D == NULL)
	{
		return false;
	}

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory( &d3dpp, sizeof( d3dpp ) );
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;



	g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice );

	if(g_pd3dDevice == NULL)
	{
		return false;
	}

	D3DVERTEXELEMENT9 PosColorVertexElements[] = 
	{
		{0,   0, D3DDECLTYPE_FLOAT3,     D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,  0},
		{0, 3*4, D3DDECLTYPE_D3DCOLOR,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,     0},
		D3DDECL_END()
	};

	g_pd3dDevice->CreateVertexDeclaration(PosColorVertexElements,&PosColor::vertexDeclaration);

	D3DVERTEXELEMENT9 PosUV0VertexElements[] = 
	{
		{0,   0, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,  0},
		{0, 3*4, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  0},
		D3DDECL_END()
	};

	g_pd3dDevice->CreateVertexDeclaration(PosUV0VertexElements,&PosUV0::VertexDeclaration);

	D3DVERTEXELEMENT9 PosNormalVertexElements[] = 
	{
		{0,   0, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,  0},
		{0, 3*4, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,  0},
		D3DDECL_END()
	};

	g_pd3dDevice->CreateVertexDeclaration(PosNormalVertexElements,&PosNormal::VertexDeclaration);

	D3DVERTEXELEMENT9 PosNormalUV0VertexElements[] = 
	{
		{0,   0, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,  0},
		{0, 3*4, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,  0},
		{0, (3+3)*4, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  0},
		D3DDECL_END()
	};

	g_pd3dDevice->CreateVertexDeclaration(PosNormalUV0VertexElements,&PosNormalUV0::VertexDeclaration);


	D3DXCreateFont(g_pd3dDevice, 18, 0, FW_REGULAR, 0, FALSE, DEFAULT_CHARSET
				  ,OUT_TT_ONLY_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE
				  ,TEXT("Segoe UI Mono"), &pFont );

	D3DXCreateTeapot	(g_pd3dDevice,                     &pTeapot,  NULL);
	/*D3DXCreateTorus		(g_pd3dDevice,0.5f,0.75f,40,40,    &pTorus,   NULL);*/
	D3DXCreateBox		(g_pd3dDevice,1.5f,1.5f,1.5f,      &pBox,     NULL);

	myTexture = LoadTextureResource("../textures/texture01.res");

	// loading models
	house = Model::Load("../models/house/","house.x");
	table = Model::Load("../models/table/","table_lowpoly.x");
	church = Model::Load("../models/housex/","fw43_lowpoly_n1.x");
	barrel = Model::Load("../models/barrels/","barrels.x");
	chair = Model::Load("../models/Wooden Kitchen Chair X/","Wooden Kitchen Chair.x");
	tree = Model::Load("../models/tree/","Tree2.x");
	
	if(table)
	{
		table->GetMaterial(0)->Power = 20.0f;
	}

		hRet = D3DXCreateTextureFromFileEx(g_pd3dDevice,"../Textures/SkyBox_Front.jpg",D3DX_DEFAULT_NONPOW2,D3DX_DEFAULT_NONPOW2,1,0,D3DFMT_UNKNOWN,D3DPOOL_MANAGED
		,D3DX_DEFAULT,D3DX_DEFAULT,0,&info,NULL,&g_SkyTextures[0]);
		hRet = D3DXCreateTextureFromFileEx(g_pd3dDevice,"../Textures/SkyBox_Back.jpg",D3DX_DEFAULT_NONPOW2,D3DX_DEFAULT_NONPOW2,1,0,D3DFMT_UNKNOWN,D3DPOOL_MANAGED
		,D3DX_DEFAULT,D3DX_DEFAULT,0,&info,NULL,&g_SkyTextures[1]);
		hRet = D3DXCreateTextureFromFileEx(g_pd3dDevice,"../Textures/SkyBox_Left.jpg",D3DX_DEFAULT_NONPOW2,D3DX_DEFAULT_NONPOW2,1,0,D3DFMT_UNKNOWN,D3DPOOL_MANAGED
		,D3DX_DEFAULT,D3DX_DEFAULT,0,&info,NULL,&g_SkyTextures[2]);
		hRet = D3DXCreateTextureFromFileEx(g_pd3dDevice,"../Textures/SkyBox_Right.jpg",D3DX_DEFAULT_NONPOW2,D3DX_DEFAULT_NONPOW2,1,0,D3DFMT_UNKNOWN,D3DPOOL_MANAGED
		,D3DX_DEFAULT,D3DX_DEFAULT,0,&info,NULL,&g_SkyTextures[3]);
		hRet = D3DXCreateTextureFromFileEx(g_pd3dDevice,"../Textures/SkyBox_Top.jpg",D3DX_DEFAULT_NONPOW2,D3DX_DEFAULT_NONPOW2,1,0,D3DFMT_UNKNOWN,D3DPOOL_MANAGED
		,D3DX_DEFAULT,D3DX_DEFAULT,0,&info,NULL,&g_SkyTextures[4]);
		hRet = D3DXCreateTextureFromFileEx(g_pd3dDevice,"../sky12/grass01.jpg",D3DX_DEFAULT_NONPOW2,D3DX_DEFAULT_NONPOW2,1,0,D3DFMT_UNKNOWN,D3DPOOL_MANAGED
		,D3DX_DEFAULT,D3DX_DEFAULT,0,&info,NULL,&g_SkyTextures[5]);
	 if ( FAILED(hRet) )
		{
		MessageBox(NULL, "Failed to open 1 or more images files!",  "Error Opening Texture Files", MB_OK | MB_ICONSTOP);
		
		}


	return true;
}

//Camera variables
float cameraHeading = -1.0f;
D3DXVECTOR3 cameraPosition(0,-14,-18);
float distance = 10;


// function to process camera movement
D3DXMATRIX ProcessCamera()
{
	unsigned char keys[256];
	GetKeyboardState( keys );

	D3DXMATRIX g_matView;
	D3DXVECTOR3 movement(0,0,0);
	float movementDelta = 1.0f;

	if(GetAsyncKeyState(VK_UP) != 0)
	{
		movement.z = movementDelta;
	}

	if(GetAsyncKeyState(VK_DOWN) != 0)
	{
		movement.z = -movementDelta;
	}

	if(keys['Z'] & 0x80)
	{
		movement.y = movementDelta;
	}

	if(keys['X'] & 0x80)
	{
		movement.y = -movementDelta;
	}

	if(GetAsyncKeyState(VK_LEFT) != 0)
	{
		cameraHeading -= 0.05f;
	}

	if(GetAsyncKeyState(VK_RIGHT) != 0)
	{
		cameraHeading += 0.05f;
	}

	if(D3DXVec3Length(&movement)>0)
	{
		D3DXMATRIX rotation;

		D3DXMatrixRotationY(&rotation,cameraHeading);

		D3DXVECTOR4 out;
		D3DXVec3Transform(&out,&movement,&rotation);
		// checks camera movement if, out of the game world
		if(cameraPosition.x+out.x < 19 && cameraPosition.x+out.x>-19)
		{
		cameraPosition.x += out.x;
		}
		if(cameraPosition.y+out.y <19 && cameraPosition.y+out.y >-19)
		{
		cameraPosition.y += out.y;
		}
		if(cameraPosition.z+out.z <19 && cameraPosition.z+out.z >-19)
		{
		cameraPosition.z += out.z;
		}
	}

	D3DXVECTOR3 CameraTarget(0,0, distance);

	D3DXMATRIX cameraRotation;

	D3DXMatrixRotationY(&cameraRotation,cameraHeading);

	D3DXVECTOR4 out;
	D3DXVec3Transform(&out,&CameraTarget,&cameraRotation);

	CameraTarget.x = out.x;
	CameraTarget.y = 0;
	CameraTarget.z = out.z;


	CameraTarget += cameraPosition;

	D3DXVECTOR3 Up(0,1,0);

	D3DXMatrixLookAtLH(&g_matView, &cameraPosition,&CameraTarget,&Up);

	return g_matView;	
}

// this function renders primitives, models and meshes to screen
void DirectXShell::Render()
{
	frameTime.Stop();
	float frameTimeTaken = frameTime.GetmS();
	frameTime.Start();
	
	//variables to rotate my interactive objects
	static float a = 0;
	static float b = 0;
	static float c = 0;
	/*static float distance = 4;*/
	static float objectAngle =0;

	a += 0.01f;

	objectAngle -= 0.01f;

	static bool selectedTable = false;
	static bool selectedBarrel = false;

	TVertex Skybox[24] =
{
	// Front quad,All quads face inward
	{-20.0f, -20.0f,  20.0f,  0.0f, 1.0f },
	{-20.0f,  20.0f,  20.0f,  0.0f, 0.0f },
	{ 20.0f, -20.0f,  20.0f,  1.0f, 1.0f },
	{ 20.0f,  20.0f,  20.0f,  1.0f, 0.0f },
	
	// Back quad
	{ 20.0f, -20.0f, -20.0f,  0.0f, 1.0f },
	{ 20.0f,  20.0f, -20.0f,  0.0f, 0.0f },
	{-20.0f, -20.0f, -20.0f,  1.0f, 1.0f },
	{-20.0f,  20.0f, -20.0f,  1.0f, 0.0f },
	
	// Left quad
	{-20.0f, -20.0f, -20.0f,  0.0f, 1.0f },
	{-20.0f,  20.0f, -20.0f,  0.0f, 0.0f },
	{-20.0f, -20.0f,  20.0f,  1.0f, 1.0f },
	{-20.0f,  20.0f,  20.0f,  1.0f, 0.0f },
	
	// Right quad
	{ 20.0f, -20.0f,  20.0f,  0.0f, 1.0f },
	{ 20.0f,  20.0f,  20.0f,  0.0f, 0.0f },
	{ 20.0f, -20.0f, -20.0f,  1.0f, 1.0f },
	{ 20.0f,  20.0f, -20.0f,  1.0f, 0.0f },

	// Top quad
	{-20.0f,  20.0f,  20.0f,  0.0f, 1.0f },
	{-20.0f,  20.0f, -20.0f,  0.0f, 0.0f },
	{ 20.0f,  20.0f,  20.0f,  1.0f, 1.0f },
	{ 20.0f,  20.0f, -20.0f,  1.0f, 0.0f },
	
	// Bottom quad
	{-20.0f, -20.0f, -20.0f,  0.0f, 1.0f },
	{-20.0f, -20.0f,  20.0f,  0.0f, 0.0f },
	{ 20.0f, -20.0f, -20.0f,  1.0f, 1.0f },
	{ 20.0f, -20.0f,  20.0f,  1.0f, 0.0f }
};
	
	// vertex buffer ( 24 vertices (4 verts * 6 faces) )
    hRet = g_pd3dDevice->CreateVertexBuffer( sizeof(TVertex) * 24,			
											 0,							
											 FVF_FLAGS,					
											 D3DPOOL_MANAGED,			
											 &g_pVertexBuffer,				
											 NULL );						
    if ( FAILED( hRet ) )
	{
	MessageBox(NULL, "Failed to create the vertex buffer!", "Error in BuildSkybox()", MB_OK | MB_ICONSTOP);

	}

	void *pVertices = NULL;

	// Copy the skybox mesh into the vertex buffer.  I initialized the whole mesh array
	// above in global space.
	g_pVertexBuffer->Lock( 0, sizeof(TVertex) * 24, (void**)&pVertices, 0 );
    memcpy( pVertices, Skybox, sizeof(TVertex) * 24 );
    g_pVertexBuffer->Unlock();



	// A skybox should appear to be the farthest "thing" away.  
	// In order to make sure the skybox is in the correct order i disabled Z buffering
	// Set up rendering states for the sky box

    g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, false );
	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, false );

	// Set up a projection matrix
    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DXToRadian( 90.0f ), 
                                SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

	// Set a default world matrix
	D3DXMATRIX matWorld;
	D3DXMatrixIdentity(&matWorld);
	g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld);

	// this gets rid of the ugly seams inbetween the cube faces
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

	if( SUCCEEDED( g_pd3dDevice->BeginScene() ) )
	{
		g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,  D3DCOLOR_COLORVALUE(0.0f,0.0f,0.0f,1.0f), 1.0f, 0 );
		g_pd3dDevice->SetFVF( FVF_FLAGS );
		g_pd3dDevice->SetStreamSource( 0, g_pVertexBuffer, 0, sizeof(TVertex));
    
		// Render the 6 faces of the skybox
		// i am using vertex buffer to store the vertices
		for ( ULONG i = 0; i < 6; ++i )
		 {
			// Set the texture for this primitive
			g_pd3dDevice->SetTexture( 0, g_SkyTextures[i] );

			// Render the face (one strip per face from the vertex buffer)
			 g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, i * 4, 2 );

		 } 
		
		// initialise the view matrix to the result of the camera processing
		g_matView = ProcessCamera();

		// setup my view transform
		g_pd3dDevice->SetTransform(D3DTS_VIEW, &g_matView);
	
		// variable for the mouse
		D3DXVECTOR2 mousePos;

		POINT cursorPos;
		GetCursorPos(&cursorPos);
		RECT aRect;

		extern HWND hWnd;
		GetWindowRect( hWnd, &aRect );

		mousePos.x = (float)(cursorPos.x - (aRect.left - sRect.left));
		mousePos.y = (float)(cursorPos.y - (aRect.top - sRect.top));

		D3DXMatrixIdentity(&matWorld);// reset the world matrix

		D3DXMATRIX  objectOrientation,rot,trans,yTrans,xTrans;// some variables for transformation

		D3DLIGHT9 light, light1;// variables for lights
		ZeroMemory( &light, sizeof(D3DLIGHT9) );
		ZeroMemory(&light1, sizeof(D3DLIGHT9));
		
		light.Type  = D3DLIGHT_DIRECTIONAL;
		light1.Type = D3DLIGHT_DIRECTIONAL;
		
		// colours for first light
		SetColour(light.Diffuse ,1.0f,1.0f,0.7f);
		SetColour(light.Ambient ,1.0f,1.0f,0.7f);
		SetColour(light.Specular,1.0f,1.0f,0.7f);
		
		//colours for second light
		SetColour(light1.Diffuse ,1.0f,1.0f,0.7f);
		SetColour(light1.Ambient ,1.0f,1.0f,0.7f);
		SetColour(light1.Specular,1.0f,1.0f,0.7f);
		
		D3DXVec3Normalize( (D3DXVECTOR3*)&light.Direction, &D3DXVECTOR3(0.0f,-10.0f,0.0f));
		D3DXVec3Normalize( (D3DXVECTOR3*)&light1.Direction, &D3DXVECTOR3(10.0f,10.0f,10.0f));
		
		g_pd3dDevice->SetRenderState(D3DRS_LIGHTING,TRUE);

		g_pd3dDevice->SetLight( 0, &light );
		g_pd3dDevice->LightEnable( 0, TRUE ); 
		g_pd3dDevice->SetLight( 1, &light1 );
		g_pd3dDevice->LightEnable( 1, TRUE ); 
		g_pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS,TRUE);
		
		D3DXMatrixScaling(&matWorld,1/8.0f,1/8.0f,1/8.0f);
		D3DXMatrixTranslation(&trans,-4,-22,10);

		matWorld = matWorld * trans;
		
		g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld);

		g_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, TRUE );
		g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, true );
		g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE);
		g_pd3dDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);

		// import house model and render
		if(house){
			house->Render();
			g_pd3dDevice->SetRenderState(D3DRS_LIGHTING,FALSE);

		}

		// if there is a table then render
		if(table)	
		{
			D3DXMatrixIdentity(&matWorld);

			D3DXMATRIX scale,roty;

			D3DXMatrixScaling(&scale,5.0f,5.0f,5.0f);

			D3DXMATRIX translate;

			D3DXMatrixTranslation(&translate,10,-22,10);
		
			D3DXMatrixMultiply(&matWorld,&translate,&matWorld);

			D3DXMatrixMultiply(&matWorld,&scale,&matWorld);
			D3DXMatrixRotationY(&roty,b);
			
			D3DXMatrixMultiply(&matWorld,&roty,&matWorld);

			if(selectedTable == true)
			{
				b  = a;
				selectedTable = false;
			}
			
			else
			{
				b = b;
			}
			table->Render(&matWorld);

			if(table->IsScreenPointInMe(matWorld * g_matView *matProj,mousePos) == true)
			{					
				
	
				if(GetAsyncKeyState(VK_LBUTTON ))
				{
					sprintf(str,"table selected");
					Print(600,60,str);
					selectedTable = true;
				}
				
	
			}
			else
			{
				sprintf(str,"table not selected");
				Print(600,60,str);
	
			}
		}
		
		// render the church model if available
		if(church)	
		{
			D3DXMatrixIdentity(&matWorld);

			D3DXMATRIX scalei,rots;

			D3DXMatrixScaling(&scalei,1.0f,1.0f,1.0f);

			D3DXMATRIX translate;

			D3DXMatrixTranslation(&translate,-20,-24,-8);
			D3DXMatrixMultiply(&matWorld,&scalei,&translate);
			g_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, TRUE );
			g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, true );
			church->Render(&matWorld);
		}
		
		// render the barrel if available
		if(barrel)
		{
			D3DXMatrixIdentity(&matWorld);

			D3DXMATRIX scalex,rotx;

			D3DXMatrixScaling(&scalex,1/100.0f,1/100.0f,1/100.0f);

			D3DXMATRIX translateb;

			D3DXMatrixTranslation(&translateb,-10,-24,-8);
			D3DXMatrixMultiply(&matWorld,&scalex,&translateb);

			if(selectedBarrel == true)
			{
				c = a;
				selectedBarrel = false;
			}

			else 
			{
				c = c;
			}
			D3DXMatrixRotationY(&rotx,c);
			D3DXMatrixMultiply(&matWorld,&rotx,&matWorld);
			
			g_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, TRUE );
			g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, true );
	
			barrel->Render(&matWorld);
			
			if(barrel->IsScreenPointInMe(matWorld * g_matView *matProj,mousePos) == true)
			{					
				
				if(GetAsyncKeyState(VK_LBUTTON))
				{
					selectedBarrel = true;
					sprintf(str,"barrel selected");
					Print(600,80,str);
				}
			}
			else
			{
				
				sprintf(str,"barrel not selected");
				Print(600,80,str);
				
			}
		}

		// render chair if available
		if (chair)
		{
			D3DXMatrixIdentity(&matWorld);

			D3DXMATRIX scalec,rotc;

			D3DXMatrixScaling(&scalec,1.0f,1.0f,1.0f);

			D3DXMATRIX translatec;

			D3DXMatrixTranslation(&translatec,12,-22,4);
			D3DXMatrixRotationY(&rotc,D3DXToRadian(90.0f ));
			
			D3DXMatrixMultiply(&matWorld,&rotc,&matWorld);
			D3DXMatrixMultiply(&matWorld,&scalec,&translatec);
			g_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, TRUE );
			g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, true );
			chair->Render(&matWorld);
		}

		// render tree if available
		if(tree)
		{
			D3DXMatrixIdentity(&matWorld);

			D3DXMATRIX scalet,rott;

			D3DXMatrixScaling(&scalet,1/1.0f,1/1.0f,1/1.0f);

			D3DXMATRIX translatet;

			D3DXMatrixTranslation(&translatet,-20,-24,-18);
			D3DXMatrixMultiply(&matWorld,&scalet,&translatet);
			g_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, TRUE );
			g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, true );
			tree->Render(&matWorld);
			
		}

		// render the rotating teapot
		D3DXMatrixIdentity(&matWorld);
		D3DXMatrixIdentity(&trans);
		D3DXMatrixIdentity(&rot);

		D3DXMatrixTranslation(&trans,10,-17,10);
		D3DXMatrixRotationY(&rot,a); // rotates the teapot in the y axis
		matWorld = matWorld*rot*trans;
		g_pd3dDevice->SetRenderState(D3DRS_LIGHTING,TRUE);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		D3DMATERIAL9 mtrl;
		ZeroMemory( &mtrl, sizeof( D3DMATERIAL9 ) );

		SetColour(mtrl.Diffuse ,1.0f,0.0f,1.0f);
		SetColour(mtrl.Ambient ,0.3f,0.0f,0.2f);
		SetColour(mtrl.Emissive,0.0f,0.0f,0.0f);
		SetColour(mtrl.Specular,1.0f,1.0f,1.0f);
		mtrl.Power = 25.0f;
		g_pd3dDevice->SetMaterial( &mtrl );
		g_pd3dDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);
		RenderD3DXMESH(pTeapot);

		// render the rotating box
		D3DXMatrixIdentity(&matWorld);
		D3DXMatrixIdentity(&trans);
		D3DXMATRIX rotBox;
		D3DXMatrixTranslation(&trans,10,-17,8);
		D3DXMatrixRotationX(&rotBox,a); // rotates the box in the x axis
		matWorld = matWorld*rotBox*trans;
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		ZeroMemory( &mtrl, sizeof( D3DMATERIAL9 ) );

		SetColour(mtrl.Diffuse ,1.0f,1.0f,0.0f);
		SetColour(mtrl.Ambient ,0.0f,0.0f,0.0f);
		SetColour(mtrl.Emissive,1.0f,0.0f,0.0f);
		SetColour(mtrl.Specular,1.0f,1.0f,1.0f);
		mtrl.Power = 25.0f;
		g_pd3dDevice->SetMaterial( &mtrl );
		g_pd3dDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);
		RenderD3DXMESH(pBox);

		// write the frames per second to screen 
		sprintf(str,"Time: %3.3f\nFPS: %3.2f", frameTimeTaken, 1000 / frameTimeTaken);
		Print(600,10,str);

		sprintf(str, "Welcome to bright's 3d village");
		Print(100,10,str);

		sprintf(str, "Move around using the arrow keys, Z and X");
		Print(100,30,str);

		sprintf(str, "Press and hold mouse button for interactivity");
		Print(100,50,str);

		// render the mouse cursor
		Print(mousePos.x,mousePos.y,"-o-");
		ShowCursor(FALSE);
		
		g_pd3dDevice->SetRenderState(D3DRS_LIGHTING,FALSE);
			
		g_pd3dDevice->EndScene();
	}

	g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}

void DirectXShell::Print(int x, int y, char* str)
{
	RECT r;
	r.right = 800;r.bottom = 600;

	r.left = x+1; r.top=y+1;

	pFont->DrawTextA(NULL, str, -1,&r,0, D3DCOLOR_XRGB(0, 0, 0));
	r.left--;r.top--;
	pFont->DrawTextA(NULL, str, -1,&r,0, D3DCOLOR_XRGB(255, 255, 255));
}

// release devices and shutdown
void DirectXShell::Shutdown()
{
	if(myTexture != NULL)
	{
		myTexture->Release();
		myTexture = NULL;
	}

	if(PosUV0::VertexDeclaration != NULL)
	{
		PosUV0::VertexDeclaration->Release();
		PosUV0::VertexDeclaration = NULL;
	}

	if(PosColor::vertexDeclaration != NULL)
	{
		PosColor::vertexDeclaration->Release();
		PosColor::vertexDeclaration = NULL;
	}

	if(g_pd3dDevice != NULL)
	{
		g_pd3dDevice->Release();
		g_pd3dDevice = NULL;
	}

	if(g_pD3D != NULL)
	{
		g_pD3D->Release();
		g_pD3D = NULL;
	}

	for (int i = 0; i < 6; i++)
	{
		g_SkyTextures[i]->Release();
		g_SkyTextures[i] = NULL;
	}

	// Release the vertex buffer
	if (g_pVertexBuffer)
	{
		g_pVertexBuffer->Release();
		g_pVertexBuffer = NULL;
	}
}

