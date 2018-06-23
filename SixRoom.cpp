// 
// Cubic mapping (Environment Mapping)
// 
// 執行前的準備工作
// 關閉 CShape.h 中的 #define PERVERTEX_LIGHTING，使用 PERPIXEL_LIGHTING 才會有作用
// 設定 #define MULTITEXTURE  (DIFFUSE_MAP|LIGHT_MAP|NORMAL_MAP)
// 開啟 #define CUBIC_MAP 1

#include "header/Angel.h"
#include "Common/CQuad.h"
#include "Common/CSolidCube.h"
#include "Common/CSolidSphere.h"
#include "Common/CWireSphere.h"
#include "Common/CWireCube.h"
#include "Common/CChecker.h"
#include "Common/CCamera.h"
#include "Common/CTexturePool.h"
#include "png_loader.h"
#include "Common/CObjReader.h"

#define SPACE_KEY 32
#define SCREEN_SIZE 800
#define HALF_SIZE SCREEN_SIZE /2 
#define VP_HALFWIDTH  20.0f
#define VP_HALFHEIGHT 20.0f
#define GRID_SIZE 20 // must be an even number

#define FLOOR_SCALE 60.f
#define WALKING_SPACE (FLOOR_SCALE/2 - 5.0f)	//行走範圍


// For Model View and Projection Matrix
mat4 g_mxModelView(1.0f);
mat4 g_mxProjection;

// For Objects
CQuad		  *g_pFloor, *g_pCeiling;
CSolidCube    *g_pLeftWall1, *g_pLeftWall2, *g_pRightWall1, *g_pRightWall2;
CSolidCube    *g_pFrontWall1, *g_pFrontWall2, *g_pBackWall1, *g_pBackWall2;
CSolidCube	  *g_pLeftRoomWalls[9], *g_pRightRoomWalls[9];

CSolidSphere  *g_pSphere;

CObjReader	  *g_pStairs;
CObjReader	  *g_pGemSweet, *g_pGemToy, *g_pGemGarden;

// For View Point
GLfloat g_fRadius = 8.0;
GLfloat g_fTheta = 45.0f*DegreesToRadians;
GLfloat g_fPhi = 45.0f*DegreesToRadians;
GLfloat g_fCameraMoveX = 0.f;				// for camera movment
GLfloat g_fCameraMoveY = 7.0f;				// for camera movment
GLfloat g_fCameraMoveZ = 0.f;				// for camera movment
mat4	g_matMoveDir;		// 鏡頭移動方向
point4  g_MoveDir;
point4  g_at;				// 鏡頭觀看方向
point4  g_eye;				// 鏡頭位置

//----------------------------------------------------------------------------
// Part 2 : for single light source
bool g_bAutoRotating = false;
float g_fElapsedTime = 0;
float g_fLightRadius = 6;
float g_fLightTheta = 0;

float g_fLightR = 0.85f;
float g_fLightG = 0.85f;
float g_fLightB = 0.85f;

LightSource g_Light1 = {
	color4(g_fLightR, g_fLightG, g_fLightB, 1.0f), // ambient 
	color4(g_fLightR, g_fLightG, g_fLightB, 1.0f), // diffuse
	color4(g_fLightR, g_fLightG, g_fLightB, 1.0f), // specular
	point4(6.0f, 5.0f, 0.0f, 1.0f),   // position
	point4(0.0f, 0.0f, 0.0f, 1.0f),   // halfVector
	vec3(0.0f, 0.0f, 0.0f),			  //spotDirection
	2.0f,	// spotExponent(parameter e); cos^(e)(phi) 
	45.0f,	// spotCutoff;	// (range: [0.0, 90.0], 180.0)  spot 的照明範圍
	0.707f,	// spotCosCutoff; // (range: [1.0,0.0],-1.0), 照明方向與被照明點之間的角度取 cos 後, cut off 的值
	1,	// constantAttenuation	(a + bd + cd^2)^-1 中的 a, d 為光源到被照明點的距離
	0,	// linearAttenuation	    (a + bd + cd^2)^-1 中的 b
	0		// quadraticAttenuation (a + bd + cd^2)^-1 中的 c
};

CWireSphere *g_pLight;
//----------------------------------------------------------------------------

// Texture 
GLuint g_uiFTexID[20]; // 三個物件分別給不同的貼圖
int g_iTexWidth,  g_iTexHeight;
GLuint g_uiSphereCubeMap; // for Cubic Texture

//----------------------------------------------------------------------------
// 函式的原型宣告
extern void IdleProcess();

void init( void )
{
	mat4 mxT, mxS;
	vec4 vT;
	vec3 vS;
	// 產生所需之 Model View 與 Projection Matrix
	// 產生所需之 Model View 與 Projection Matrix
	point4  eye(g_fRadius*sin(g_fTheta)*sin(g_fPhi), g_fRadius*cos(g_fTheta), g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.0f);
	point4  at(0.0f, 0.0f, 0.0f, 1.0f);
	auto camera = CCamera::create();
	camera->updateViewLookAt(eye, at);
	camera->updatePerspective(60.0, (GLfloat)SCREEN_SIZE / (GLfloat)SCREEN_SIZE, 1.0, 1000.0);

	auto texturepool = CTexturePool::create();
	g_uiFTexID[0] = texturepool->AddTexture("texture/DiffuseMap/stone-tile-1.png");		// floor
	g_uiFTexID[1] = texturepool->AddTexture("texture/NormalMap/stone-tile-1_NRM.png");	// floor Normal
	g_uiFTexID[2] = texturepool->AddTexture("texture/metal.png");////////////////////////////////
	g_uiFTexID[3] = texturepool->AddTexture("texture/DiffuseMap/stair.png");			// stair Diffuse
	g_uiFTexID[4] = texturepool->AddTexture("texture/NormalMap/stair_NRM.png");			// stair Normal
	g_uiFTexID[5] = texturepool->AddTexture("texture/LightMap/star2.png");				// red gem Light
	g_uiFTexID[6] = texturepool->AddTexture("texture/LightMap/star4.png");				// blue gem Light
	g_uiFTexID[7] = texturepool->AddTexture("texture/LightMap/flower2.png");			// green gem Light
	
	g_uiSphereCubeMap = CubeMap_load_SOIL();


	// ------------------- 產生物件的實體 --------------------
	//WALLS
	g_pFloor = new CQuad;													// Floor
	g_pFloor->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
	g_pFloor->SetShader();
	vT.x = 0.0f; vT.y = 0.0f; vT.z = 0.0f;			// Location
	mxT = Translate(vT);
	vS.x = vS.y = vS.z = FLOOR_SCALE;				// Scale
	mxS = Scale(vS);
	g_pFloor->SetTRSMatrix(mxT * mxS);
	g_pFloor->SetShadingMode(GOURAUD_SHADING);
	g_pFloor->SetTiling(6, 6);					// 設定貼圖
	g_pFloor->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pFloor->SetKaKdKsShini(0, 0.8f, 0.5f, 1);

	g_pCeiling = new CQuad;													// Ceiling
	g_pCeiling->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
	g_pCeiling->SetShader();
	vT.x = 0.0f; vT.y = FLOOR_SCALE / 2.0f; vT.z = 0.0f;	// Location
	mxT = Translate(vT);
	g_pCeiling->SetTRSMatrix(mxT * RotateX(180.0f) * mxS);
	g_pCeiling->SetShadingMode(GOURAUD_SHADING);
	g_pCeiling->SetTiling(6, 6);					// 設定貼圖
	g_pCeiling->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pCeiling->SetKaKdKsShini(0, 0.8f, 0.5f, 1);

	g_pLeftWall1 = new CSolidCube;											// Left Wall-1
	g_pLeftWall1->SetTextureLayer( DIFFUSE_MAP | NORMAL_MAP);
	g_pLeftWall1->SetShader();
	vT.x = -FLOOR_SCALE / 2.0f; vT.y = FLOOR_SCALE / 4.0f; vT.z = -FLOOR_SCALE / 4.0f;
	mxT = Translate(vT);
	mxS = Scale(FLOOR_SCALE / 2.0f, 1, FLOOR_SCALE / 2.0f);
	g_pLeftWall1->SetTRSMatrix(mxT * RotateX(90.0f) * RotateZ(90.0f) * mxS);
	g_pLeftWall1->SetShadingMode(GOURAUD_SHADING);
	g_pLeftWall1->SetTiling(3, 3);					// 設定貼圖
	g_pLeftWall1->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pLeftWall1->SetKaKdKsShini(0.25f, 0.8f, 0.2f, 2);

	g_pLeftWall2 = new CSolidCube;											// Left Wall-2
	g_pLeftWall2->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
	g_pLeftWall2->SetShader();
	vT.x = -FLOOR_SCALE / 2.0f; vT.y = FLOOR_SCALE / 4.0f; vT.z = FLOOR_SCALE / 4.0f;
	mxT = Translate(vT);
	g_pLeftWall2->SetTRSMatrix(mxT * RotateX(90.0f) * RotateZ(90.0f) * mxS);
	g_pLeftWall2->SetShadingMode(GOURAUD_SHADING);
	g_pLeftWall2->SetTiling(3, 3);					// 設定貼圖
	g_pLeftWall2->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pLeftWall2->SetKaKdKsShini(0.25f, 0.8f, 0.2f, 2);

	g_pRightWall1 = new CSolidCube;											// Right Wall-1
	g_pRightWall1->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
	g_pRightWall1->SetShader();
	vT.x = FLOOR_SCALE / 2.0f; vT.y = FLOOR_SCALE / 4.0f; vT.z = -FLOOR_SCALE / 4.0f;
	mxT = Translate(vT);
	g_pRightWall1->SetTRSMatrix(mxT * RotateX(90.0f) * RotateZ(90.0f) * mxS);
	g_pRightWall1->SetShadingMode(GOURAUD_SHADING);
	g_pRightWall1->SetTiling(3, 3);					// 設定貼圖
	g_pRightWall1->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pRightWall1->SetKaKdKsShini(0.25f, 0.8f, 0.2f, 2);

	g_pRightWall2 = new CSolidCube;											// Right Wall-2
	g_pRightWall2->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
	g_pRightWall2->SetShader();
	vT.x = FLOOR_SCALE / 2.0f; vT.y = FLOOR_SCALE / 4.0f; vT.z = FLOOR_SCALE / 4.0f;
	mxT = Translate(vT);
	g_pRightWall2->SetTRSMatrix(mxT * RotateX(90.0f) * RotateZ(90.0f) * mxS);
	g_pRightWall2->SetShadingMode(GOURAUD_SHADING);
	g_pRightWall2->SetTiling(3, 3);					// 設定貼圖
	g_pRightWall2->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pRightWall2->SetKaKdKsShini(0.25f, 0.8f, 0.2f, 2);

	g_pFrontWall1 = new CSolidCube;											// Front Wall-1
	g_pFrontWall1->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
	g_pFrontWall1->SetShader();
	vT.x = -FLOOR_SCALE / 4.0f; vT.y = FLOOR_SCALE / 4.0f; vT.z = -FLOOR_SCALE / 2.0f;
	mxT = Translate(vT);
	g_pFrontWall1->SetTRSMatrix(mxT * RotateX(90.0f) * mxS);
	g_pFrontWall1->SetShadingMode(GOURAUD_SHADING);
	g_pFrontWall1->SetTiling(3, 3);					// 設定貼圖
	g_pFrontWall1->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pFrontWall1->SetKaKdKsShini(0.25f, 0.8f, 0.2f, 2);

	g_pFrontWall2 = new CSolidCube;											// Front Wall-2
	g_pFrontWall2->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
	g_pFrontWall2->SetShader();
	vT.x = FLOOR_SCALE / 4.0f; vT.y = FLOOR_SCALE / 4.0f; vT.z = -FLOOR_SCALE / 2.0f;
	mxT = Translate(vT);
	g_pFrontWall2->SetTRSMatrix(mxT * RotateX(90.0f) * mxS);
	g_pFrontWall2->SetShadingMode(GOURAUD_SHADING);
	g_pFrontWall2->SetTiling(3, 3);					// 設定貼圖
	g_pFrontWall2->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pFrontWall2->SetKaKdKsShini(0.25f, 0.8f, 0.2f, 2);

	g_pBackWall1 = new CSolidCube;											// Back Wall-1
	g_pBackWall1->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
	g_pBackWall1->SetShader();
	vT.x = -FLOOR_SCALE / 4.0f; vT.y = FLOOR_SCALE / 4.0f; vT.z = FLOOR_SCALE / 2.0f;
	mxT = Translate(vT);
	g_pBackWall1->SetTRSMatrix(mxT * RotateX(90.0f) * mxS);
	g_pBackWall1->SetShadingMode(GOURAUD_SHADING);
	g_pBackWall1->SetTiling(3, 3);					// 設定貼圖
	g_pBackWall1->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pBackWall1->SetKaKdKsShini(0.25f, 0.8f, 0.2f, 2);

	g_pBackWall2 = new CSolidCube;											// Back Wall-2
	g_pBackWall2->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
	g_pBackWall2->SetShader();
	vT.x = FLOOR_SCALE / 4.0f; vT.y = FLOOR_SCALE / 4.0f; vT.z = FLOOR_SCALE / 2.0f;
	mxT = Translate(vT);
	g_pBackWall2->SetTRSMatrix(mxT * RotateX(90.0f) * mxS);
	g_pBackWall2->SetShadingMode(GOURAUD_SHADING);
	g_pBackWall2->SetTiling(3, 3);					// 設定貼圖
	g_pBackWall2->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pBackWall2->SetKaKdKsShini(0.25f, 0.8f, 0.2f, 2);

	for (int i = 0; i < 9; i++) {
		g_pLeftRoomWalls[i] = new CSolidCube;											// Left Room Walls
		g_pLeftRoomWalls[i]->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
		g_pLeftRoomWalls[i]->SetShader();
		if (i == 0) {			// Left side 1
			vT.x = -FLOOR_SCALE; vT.y = FLOOR_SCALE / 4.0f; vT.z = -FLOOR_SCALE / 4.0f;
			mxT = Translate(vT);
			g_pLeftRoomWalls[i]->SetTRSMatrix(mxT * RotateX(90.0f) * RotateZ(90.0f) * mxS);
		}
		else if (i == 1) {		// Left side 2
			vT.x = -FLOOR_SCALE; vT.y = FLOOR_SCALE / 4.0f; vT.z = FLOOR_SCALE / 4.0f;
			mxT = Translate(vT);
			g_pLeftRoomWalls[i]->SetTRSMatrix(mxT * RotateX(90.0f) * RotateZ(90.0f) * mxS);
		}
		else if (i == 2) {		// Left Front 1
			vT.x = -FLOOR_SCALE * 0.75f; vT.y = FLOOR_SCALE / 4.0f; vT.z = -FLOOR_SCALE / 2.0f;
			mxT = Translate(vT);
			g_pLeftRoomWalls[i]->SetTRSMatrix(mxT * RotateX(90.0f) * mxS);
		}
		else if (i == 3) {		// Left Front 2
			vT.x = -FLOOR_SCALE * 0.75f; vT.y = FLOOR_SCALE / 4.0f; vT.z = 0.f;
			mxT = Translate(vT);
			g_pLeftRoomWalls[i]->SetTRSMatrix(mxT * RotateX(90.0f) * mxS);
		}
		else if (i == 4) {		// Left Front 3
			vT.x = -FLOOR_SCALE * 0.75f; vT.y = FLOOR_SCALE / 4.0f; vT.z = FLOOR_SCALE / 2.0f;
			mxT = Translate(vT);
			g_pLeftRoomWalls[i]->SetTRSMatrix(mxT * RotateX(90.0f) * mxS);
		}
		else if (i == 5) {		// Left Floor 1
			vT.x = -FLOOR_SCALE * 0.75f; vT.y = -0.5f; vT.z = -FLOOR_SCALE / 4.0f;
			mxT = Translate(vT);
			g_pLeftRoomWalls[i]->SetTRSMatrix(mxT * mxS);
		}
		else if (i == 6) {		// Left Ceiling 1
			vT.x = -FLOOR_SCALE * 0.75f; vT.y = FLOOR_SCALE / 2.0f + 0.5f; vT.z = -FLOOR_SCALE / 4.0f;
			mxT = Translate(vT);
			g_pLeftRoomWalls[i]->SetTRSMatrix(mxT * mxS);
		}
		else if (i == 7) {		// Left Floor 2
			vT.x = -FLOOR_SCALE * 0.75f; vT.y = -0.5f; vT.z = FLOOR_SCALE / 4.0f;
			mxT = Translate(vT);
			g_pLeftRoomWalls[i]->SetTRSMatrix(mxT * mxS);
		}
		else if (i == 8) {		// Left Ceiling 2
			vT.x = -FLOOR_SCALE * 0.75f; vT.y = FLOOR_SCALE / 2.0f + 0.5f; vT.z = FLOOR_SCALE / 4.0f;
			mxT = Translate(vT);
			g_pLeftRoomWalls[i]->SetTRSMatrix(mxT * mxS);
		}
		g_pLeftRoomWalls[i]->SetShadingMode(GOURAUD_SHADING);
		g_pLeftRoomWalls[i]->SetTiling(3, 3);					// 設定貼圖
		g_pLeftRoomWalls[i]->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
		g_pLeftRoomWalls[i]->SetKaKdKsShini(0.25f, 0.8f, 0.2f, 2);

		//---------------------------------------------

		g_pRightRoomWalls[i] = new CSolidCube;											// Right Room Walls
		g_pRightRoomWalls[i]->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
		g_pRightRoomWalls[i]->SetShader();
		if (i == 0) {			// Right side 1
			vT.x = FLOOR_SCALE; vT.y = FLOOR_SCALE / 4.0f; vT.z = -FLOOR_SCALE / 4.0f;
			mxT = Translate(vT);
			g_pRightRoomWalls[i]->SetTRSMatrix(mxT * RotateX(90.0f) * RotateZ(90.0f) * mxS);
		}
		else if (i == 1) {		// Right side 2
			vT.x = FLOOR_SCALE; vT.y = FLOOR_SCALE / 4.0f; vT.z = FLOOR_SCALE / 4.0f;
			mxT = Translate(vT);
			g_pRightRoomWalls[i]->SetTRSMatrix(mxT * RotateX(90.0f) * RotateZ(90.0f) * mxS);
		}
		else if (i == 2) {		// Right Front 1
			vT.x = FLOOR_SCALE * 0.75f; vT.y = FLOOR_SCALE / 4.0f; vT.z = -FLOOR_SCALE / 2.0f;
			mxT = Translate(vT);
			g_pRightRoomWalls[i]->SetTRSMatrix(mxT * RotateX(90.0f) * mxS);
		}
		else if (i == 3) {		// Right Front 2
			vT.x = FLOOR_SCALE * 0.75f; vT.y = FLOOR_SCALE / 4.0f; vT.z = 0.f;
			mxT = Translate(vT);
			g_pRightRoomWalls[i]->SetTRSMatrix(mxT * RotateX(90.0f) * mxS);
		}
		else if (i == 4) {		// Right Front 3
			vT.x = FLOOR_SCALE * 0.75f; vT.y = FLOOR_SCALE / 4.0f; vT.z = FLOOR_SCALE / 2.0f;
			mxT = Translate(vT);
			g_pRightRoomWalls[i]->SetTRSMatrix(mxT * RotateX(90.0f) * mxS);
		}
		else if (i == 5) {		// Right Floor 1
			vT.x = FLOOR_SCALE * 0.75f; vT.y = -0.5f; vT.z = -FLOOR_SCALE / 4.0f;
			mxT = Translate(vT);
			g_pRightRoomWalls[i]->SetTRSMatrix(mxT * mxS);
		}
		else if (i == 6) {		// Right Ceiling 1
			vT.x = FLOOR_SCALE * 0.75f; vT.y = FLOOR_SCALE / 2.0f + 0.5f; vT.z = -FLOOR_SCALE / 4.0f;
			mxT = Translate(vT);
			g_pRightRoomWalls[i]->SetTRSMatrix(mxT * mxS);
		}
		else if (i == 7) {		// Right Floor 2
			vT.x = FLOOR_SCALE * 0.75f; vT.y = -0.5f; vT.z = FLOOR_SCALE / 4.0f;
			mxT = Translate(vT);
			g_pRightRoomWalls[i]->SetTRSMatrix(mxT * mxS);
		}
		else if (i == 8) {		// Right Ceiling 2
			vT.x = FLOOR_SCALE * 0.75f; vT.y = FLOOR_SCALE / 2.0f + 0.5f; vT.z = FLOOR_SCALE / 4.0f;
			mxT = Translate(vT);
			g_pRightRoomWalls[i]->SetTRSMatrix(mxT * mxS);
		}
		g_pRightRoomWalls[i]->SetShadingMode(GOURAUD_SHADING);
		g_pRightRoomWalls[i]->SetTiling(3, 3);					// 設定貼圖
		g_pRightRoomWalls[i]->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
		g_pRightRoomWalls[i]->SetKaKdKsShini(0.25f, 0.8f, 0.2f, 2);
	}

	//----------------------------------------------------------------------

	// For Reflecting Sphere
	g_pSphere = new CSolidSphere(1.0f, 24, 12);
	g_pSphere->SetTextureLayer(DIFFUSE_MAP);  // 使用 
	g_pSphere->SetCubeMapTexName(1);
	g_pSphere->SetViewPosition(eye);
	g_pSphere->SetShaderName("vsCubeMapping.glsl", "fsCubeMapping.glsl");
	g_pSphere->SetShader();
	vT.x = 0.0f; vT.y = 2.0f; vT.z = 0.0f;
	mxT = Translate(vT);
	mxT._m[0][0] = 2.0f; mxT._m[1][1] = 2.0f; mxT._m[2][2] = 2.0f;
	g_pSphere->SetTRSMatrix(mxT*RotateX(90.0f));
	g_pSphere->SetShadingMode(GOURAUD_SHADING);
	// 設定貼圖
	g_pSphere->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pSphere->SetKaKdKsShini(0, 0.8f, 0.5f, 1);
	g_pSphere->SetColor(vec4(0.9f, 0.9f, 0.9f, 1.0f));

	//----------------------------------------------------------------------

	g_pStairs = new CObjReader("obj/stairs_fix.obj");					//階梯
	g_pStairs->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);	//貼圖
	g_pStairs->SetMaterials(vec4(0), vec4(0.95f, 0.95f, 0.95f, 1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f));	// materials
	g_pStairs->SetKaKdKsShini(0.15f, 0.95f, 0.5f, 5);
	g_pStairs->SetShader();
	vT.x = 0.0f; vT.y = 0.0f; vT.z = -20.0f;	//Location
	mxT = Translate(vT);
	vS.x = vS.y = vS.z = 0.3f;				//Scale
	mxS = Scale(vS);
	g_pStairs->SetTRSMatrix(mxT /** mxS*/);
	g_pStairs->SetShadingMode(GOURAUD_SHADING);

	//----------------------------------------------------------------------
	g_pGemSweet = new CObjReader("obj/gem_sweet.obj");			//紅水晶
	g_pGemSweet->SetTextureLayer(DIFFUSE_MAP | LIGHT_MAP);	//貼圖
	g_pGemSweet->SetLightMapColor(vec4(1.0f, 0.3f, 0.3f, 0.5f));	//Light Map : 半透明粉色
	// materials
	g_pGemSweet->SetMaterials(vec4(0), vec4(0.85f, 0, 0, 0.7f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pGemSweet->SetKaKdKsShini(0.15f, 0.95f, 0.5f, 5);
	g_pGemSweet->SetShader();
	vT.x = -FLOOR_SCALE * 0.75f; vT.y = 1.0f; vT.z = -FLOOR_SCALE / 4.0f;	//Location : LR1
	mxT = Translate(vT);
	vS.x = vS.y = vS.z = 0.5f;				//Scale
	mxS = Scale(vS);
	g_pGemSweet->SetTRSMatrix(mxT * mxS);
	g_pGemSweet->SetShadingMode(GOURAUD_SHADING);

	//-----------------------------------------
	g_pGemToy = new CObjReader("obj/gem_toy.obj");				//藍水晶
	g_pGemToy->SetTextureLayer(DIFFUSE_MAP | LIGHT_MAP);	//貼圖
	g_pGemToy->SetLightMapColor(vec4(0.0f, 1.0f, 1.0f, 0.5f));	//Light Map : 半透明青色
	// materials
	g_pGemToy->SetMaterials(vec4(0), vec4(0, 0, 0.85f, 0.7f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pGemToy->SetKaKdKsShini(0.15f, 0.95f, 0.95f, 5);
	g_pGemToy->SetShader();
	vT.x = -FLOOR_SCALE * 0.75f; vT.y = 0.0f; vT.z = FLOOR_SCALE / 4.0f;	//Location : LR2
	mxT = Translate(vT);
	vS.x = vS.y = vS.z = 0.5f;				//Scale
	mxS = Scale(vS);
	g_pGemToy->SetTRSMatrix(mxT * RotateY(90.f) * mxS);
	g_pGemToy->SetShadingMode(GOURAUD_SHADING);

	//-----------------------------------------
	g_pGemGarden = new CObjReader("obj/gem_garden.obj");		//綠水晶
	g_pGemGarden->SetTextureLayer(DIFFUSE_MAP | LIGHT_MAP);	//貼圖
	g_pGemGarden->SetLightMapColor(vec4(1.0f, 1.0f, 0.0f, 0.3f));	//Light Map : 半透明黃色
	// materials
	g_pGemGarden->SetMaterials(vec4(0), vec4(0.f, 0.85f, 0.f, 0.7f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pGemGarden->SetKaKdKsShini(0.15f, 0.95f, 0.8f, 5);
	g_pGemGarden->SetShader();
	vT.x = FLOOR_SCALE * 0.75f; vT.y = 1.0f; vT.z = -FLOOR_SCALE / 4.0f;	//Location : RR1
	mxT = Translate(vT);
	vS.x = vS.y = vS.z = 0.5f;				//Scale
	mxS = Scale(vS);
	g_pGemGarden->SetTRSMatrix(mxT * mxS);
	g_pGemGarden->SetShadingMode(GOURAUD_SHADING);


	//------------------------------------------
	// 設定 代表 Light 的 WireSphere
	g_pLight = new CWireSphere(0.25f, 6, 3);
	g_pLight->SetLightingDisable();
	g_pLight->SetTextureLayer(NONE_MAP);	// 沒有貼圖
	g_pLight->SetShader();
	mxT = Translate(g_Light1.position);
	g_pLight->SetTRSMatrix(mxT);
	g_pLight->SetColor(g_Light1.diffuse);


	// 因為本範例不會動到 Projection Matrix 所以在這裡設定一次即可
	// 就不寫在 OnFrameMove 中每次都 Check
	bool bPDirty;
	mat4 mpx = camera->getProjectionMatrix(bPDirty);
	g_pFloor->SetProjectionMatrix(mpx);
	g_pCeiling->SetProjectionMatrix(mpx);
	g_pLeftWall1->SetProjectionMatrix(mpx);
	g_pLeftWall2->SetProjectionMatrix(mpx);
	g_pRightWall1->SetProjectionMatrix(mpx);
	g_pRightWall2->SetProjectionMatrix(mpx);
	g_pFrontWall1->SetProjectionMatrix(mpx);
	g_pFrontWall2->SetProjectionMatrix(mpx);
	g_pBackWall1->SetProjectionMatrix(mpx);
	g_pBackWall2->SetProjectionMatrix(mpx);
	for (int i = 0; i < 9; i++) {
		g_pLeftRoomWalls[i]->SetProjectionMatrix(mpx);
		g_pRightRoomWalls[i]->SetProjectionMatrix(mpx);
	}

	g_pSphere->SetProjectionMatrix(mpx);
	g_pLight->SetProjectionMatrix(mpx);

	g_pStairs->SetProjectionMatrix(mpx);

	g_pGemSweet->SetProjectionMatrix(mpx);
	g_pGemToy->SetProjectionMatrix(mpx);
	g_pGemGarden->SetProjectionMatrix(mpx);
}

//----------------------------------------------------------------------------
void GL_Display( void )
{

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); // clear the window

	glEnable(GL_BLEND);								// 設定2D Texure Mapping 有作用
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glActiveTexture(GL_TEXTURE0);					// select active texture 0
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[0]);	// 與 Diffuse Map 結合
	glActiveTexture(GL_TEXTURE2);					// normal map
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[1]);
	g_pFloor->Draw();
	g_pCeiling->Draw();
	//g_pLeftWall1->Draw();
	//g_pLeftWall2->Draw();
	//g_pRightWall1->Draw();
	//g_pRightWall2->Draw();
	g_pFrontWall1->Draw();
	g_pFrontWall2->Draw();
	g_pBackWall1->Draw();
	g_pBackWall2->Draw();
	for (int i = 0; i < 9; i++) {
		g_pLeftRoomWalls[i]->Draw();
		g_pRightRoomWalls[i]->Draw();
	}

	glActiveTexture(GL_TEXTURE0);							// select active texture 0
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[2]);			// 與 Diffuse Map 結合
	glActiveTexture(GL_TEXTURE1);							// select active texture 1
	glBindTexture(GL_TEXTURE_CUBE_MAP, g_uiSphereCubeMap);	// 與 Light Map 結合
	g_pSphere->Draw();

	glBindTexture(GL_TEXTURE_2D, 0);
	g_pLight->Draw();

	glDepthMask(GL_FALSE);	//---------------------------------------  透明度物件

	glActiveTexture(GL_TEXTURE1); // select active texture 1
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[5]); // 與 Light Map 結合
	g_pGemSweet->Draw();		//GEMS

	glActiveTexture(GL_TEXTURE1); // select active texture 1
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[6]); // 與 Light Map 結合
	g_pGemToy->Draw();

	glActiveTexture(GL_TEXTURE1); // select active texture 1
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[7]); // 與 Light Map 結合
	g_pGemGarden->Draw();

	glDisable(GL_BLEND);	// 關閉 Blending
	glDepthMask(GL_TRUE);	// --------------------------------------  開啟對 Z-Buffer 的寫入操作

	glActiveTexture(GL_TEXTURE0);					// diffuse map
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[3]);
	glActiveTexture(GL_TEXTURE2);					// normal map
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[4]);
	g_pStairs->Draw();

	//-------------------------------------
	glutSwapBuffers();	// 交換 Frame Buffer
}

//----------------------------------------------------------------------------
// Part 2 : for single light source
void UpdateLightPosition(float dt)
{
	mat4 mxT;
	// 每秒繞 Y 軸轉 90 度
	g_fElapsedTime += dt;
	g_fLightTheta = g_fElapsedTime*(float)M_PI_2;
	if( g_fLightTheta >= (float)M_PI*2.0f ) {
		g_fLightTheta -= (float)M_PI*2.0f;
		g_fElapsedTime -= 4.0f;
	}
	g_Light1.position.x = g_fLightRadius * cosf(g_fLightTheta);
	g_Light1.position.z = g_fLightRadius * sinf(g_fLightTheta);
	mxT = Translate(g_Light1.position);
	g_pLight->SetTRSMatrix(mxT);
}
//----------------------------------------------------------------------------

void onFrameMove(float delta)
{
	// for camera
	g_at = vec4(g_fRadius*sin(g_fTheta)*sin(g_fPhi) + g_fCameraMoveX,
		g_fRadius*cos(g_fTheta) + g_fCameraMoveY,
		g_fRadius*sin(g_fTheta)*cos(g_fPhi) + g_fCameraMoveZ,
		1.0f);
	g_eye = vec4(g_fCameraMoveX, g_fCameraMoveY, g_fCameraMoveZ, 1.0f);	//第一人稱視角
	auto camera = CCamera::getInstance();
	camera->updateViewLookAt(g_eye, g_at);

	//---------------------------------------------------------------

	if( g_bAutoRotating ) { // Part 2 : 重新計算 Light 的位置
		UpdateLightPosition(delta);
	}

	mat4 mvx;	// view matrix & projection matrix
	bool bVDirty;	// view 與 projection matrix 是否需要更新給物件
	//auto camera = CCamera::getInstance();
	mvx = camera->getViewMatrix(bVDirty);
	if (bVDirty) {
		g_pFloor->SetViewMatrix(mvx);
		g_pCeiling->SetViewMatrix(mvx);
		g_pLeftWall1->SetViewMatrix(mvx);
		g_pLeftWall2->SetViewMatrix(mvx);
		g_pRightWall1->SetViewMatrix(mvx);
		g_pRightWall2->SetViewMatrix(mvx);
		g_pFrontWall1->SetViewMatrix(mvx);
		g_pFrontWall2->SetViewMatrix(mvx);
		g_pBackWall1->SetViewMatrix(mvx);
		g_pBackWall2->SetViewMatrix(mvx);
		for (int i = 0; i < 9; i++) {
			g_pLeftRoomWalls[i]->SetViewMatrix(mvx);
			g_pRightRoomWalls[i]->SetViewMatrix(mvx);
		}

		g_pSphere->SetViewMatrix(mvx);
		g_pSphere->SetViewPosition(camera->getViewPosition());
		g_pLight->SetViewMatrix(mvx);

		g_pStairs->SetViewMatrix(mvx);

		g_pGemSweet->SetViewMatrix(mvx);
		g_pGemToy->SetViewMatrix(mvx);
		g_pGemGarden->SetViewMatrix(mvx);
	}

	// 如果需要重新計算時，在這邊計算每一個物件的顏色
	g_pFloor->Update(delta, g_Light1);			// WALLS
	g_pCeiling->Update(delta, g_Light1);
	g_pLeftWall1->Update(delta, g_Light1);
	g_pLeftWall2->Update(delta, g_Light1);
	g_pRightWall1->Update(delta, g_Light1);
	g_pRightWall2->Update(delta, g_Light1);
	g_pFrontWall1->Update(delta, g_Light1);
	g_pFrontWall2->Update(delta, g_Light1);
	g_pBackWall1->Update(delta, g_Light1);
	g_pBackWall2->Update(delta, g_Light1);
	for (int i = 0; i < 9; i++) {
		g_pLeftRoomWalls[i]->Update(delta, g_Light1);
		g_pRightRoomWalls[i]->Update(delta, g_Light1);
	}

	g_pSphere->Update(delta, g_Light1);
	g_pLight->Update(delta);

	g_pStairs->Update(delta, g_Light1);

	g_pGemSweet->Update(delta, g_Light1);		// gems
	g_pGemToy->Update(delta, g_Light1);
	g_pGemGarden->Update(delta, g_Light1);

	GL_Display();
}

//----------------------------------------------------------------------------

void Win_Keyboard( unsigned char key, int x, int y )
{
    switch ( key ) {
	case  SPACE_KEY:
		g_bAutoRotating = !g_bAutoRotating;
		break;
//----------------------------------------------------------------------------

	// ---------- for camera movment -----------
	case 'W':
	case 'w':
		g_MoveDir = vec4(g_fRadius*sin(g_fTheta)*sin(g_fPhi), 0.f, g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.f);
		g_MoveDir = normalize(g_MoveDir);
		g_matMoveDir = Translate(g_MoveDir.x, 0.f, g_MoveDir.z);
		if (g_fCameraMoveX <= WALKING_SPACE && g_fCameraMoveX >= -WALKING_SPACE && g_fCameraMoveZ <= WALKING_SPACE && g_fCameraMoveZ >= -WALKING_SPACE)  
		{	
			g_fCameraMoveX += (g_matMoveDir._m[0][3] * 0.2f);	//限制空間
			g_fCameraMoveZ += (g_matMoveDir._m[2][3] * 0.2f);
		}
		else {	// 修正卡牆
			if (g_fCameraMoveX > WALKING_SPACE) g_fCameraMoveX = WALKING_SPACE;
			else if (g_fCameraMoveX < -WALKING_SPACE) g_fCameraMoveX = -WALKING_SPACE;
			if (g_fCameraMoveZ > WALKING_SPACE) g_fCameraMoveZ = WALKING_SPACE;
			else if (g_fCameraMoveZ < -WALKING_SPACE) g_fCameraMoveZ = -WALKING_SPACE;
		}
		break;
	case 'S':
	case 's':
		g_MoveDir = vec4(g_fRadius*sin(g_fTheta)*sin(g_fPhi), 0.f, g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.f);
		g_MoveDir = normalize(g_MoveDir);
		g_matMoveDir = Translate(g_MoveDir.x, 0.f, g_MoveDir.z);
		if (g_fCameraMoveX <= WALKING_SPACE && g_fCameraMoveX >= -WALKING_SPACE && g_fCameraMoveZ <= WALKING_SPACE && g_fCameraMoveZ >= -WALKING_SPACE) 
		{	
			g_fCameraMoveX -= (g_matMoveDir._m[0][3] * 0.2f);	//限制空間
			g_fCameraMoveZ -= (g_matMoveDir._m[2][3] * 0.2f);
		}
		else {	// 修正卡牆
			if (g_fCameraMoveX > WALKING_SPACE) g_fCameraMoveX = WALKING_SPACE;
			else if (g_fCameraMoveX < -WALKING_SPACE) g_fCameraMoveX = -WALKING_SPACE;
			if (g_fCameraMoveZ > WALKING_SPACE) g_fCameraMoveZ = WALKING_SPACE;
			else if (g_fCameraMoveZ < -WALKING_SPACE) g_fCameraMoveZ = -WALKING_SPACE;
		}
		break;
	case 'A':
	case 'a':
		g_MoveDir = vec4(g_fRadius*sin(g_fTheta)*sin(g_fPhi), 0.f, g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.f);
		g_MoveDir = normalize(g_MoveDir);
		g_matMoveDir = RotateY(90.f) * Translate(g_MoveDir.x, 0.f, g_MoveDir.z);
		if (g_fCameraMoveX <= WALKING_SPACE && g_fCameraMoveX >= -WALKING_SPACE && g_fCameraMoveZ <= WALKING_SPACE && g_fCameraMoveZ >= -WALKING_SPACE) 
		{	
			g_fCameraMoveX += (g_matMoveDir._m[0][3] * 0.2f);	//限制空間
			g_fCameraMoveZ += (g_matMoveDir._m[2][3] * 0.2f);
		}
		else {	// 修正卡牆
			if (g_fCameraMoveX > WALKING_SPACE) g_fCameraMoveX = WALKING_SPACE;
			else if (g_fCameraMoveX < -WALKING_SPACE) g_fCameraMoveX = -WALKING_SPACE;
			if (g_fCameraMoveZ > WALKING_SPACE) g_fCameraMoveZ = WALKING_SPACE;
			else if (g_fCameraMoveZ < -WALKING_SPACE) g_fCameraMoveZ = -WALKING_SPACE;
		}
		break;
	case 'D':
	case 'd':
		g_MoveDir = vec4(g_fRadius*sin(g_fTheta)*sin(g_fPhi), 0.f, g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.f);
		g_MoveDir = normalize(g_MoveDir);
		g_matMoveDir = RotateY(90.f) * Translate(g_MoveDir.x, 0.f, g_MoveDir.z);
		if (g_fCameraMoveX <= WALKING_SPACE && g_fCameraMoveX >= -WALKING_SPACE && g_fCameraMoveZ <= WALKING_SPACE && g_fCameraMoveZ >= -WALKING_SPACE) 
		{	
			g_fCameraMoveX -= (g_matMoveDir._m[0][3] * 0.2f);	//限制空間
			g_fCameraMoveZ -= (g_matMoveDir._m[2][3] * 0.2f);
		}
		else {	// 修正卡牆
			if (g_fCameraMoveX > WALKING_SPACE) g_fCameraMoveX = WALKING_SPACE;
			else if (g_fCameraMoveX < -WALKING_SPACE) g_fCameraMoveX = -WALKING_SPACE;
			if (g_fCameraMoveZ > WALKING_SPACE) g_fCameraMoveZ = WALKING_SPACE;
			else if (g_fCameraMoveZ < -WALKING_SPACE) g_fCameraMoveZ = -WALKING_SPACE;
		}
		break;

		// --------- for light color ---------
	case 82: // R key
		if( g_fLightR <= 0.95f ) g_fLightR += 0.05f;
		g_Light1.diffuse.x = g_fLightR;
		g_pLight->SetColor(g_Light1.diffuse);
		break;
	case 114: // r key
		if( g_fLightR >= 0.05f ) g_fLightR -= 0.05f;
		g_Light1.diffuse.x = g_fLightR;
		g_pLight->SetColor(g_Light1.diffuse);
		break;
	case 71: // G key
		if( g_fLightG <= 0.95f ) g_fLightG += 0.05f;
		g_Light1.diffuse.y = g_fLightG;
		g_pLight->SetColor(g_Light1.diffuse);
		break;
	case 103: // g key
		if( g_fLightG >= 0.05f ) g_fLightG -= 0.05f;
		g_Light1.diffuse.y = g_fLightG;
		g_pLight->SetColor(g_Light1.diffuse);
		break;
	case 66: // B key
		if( g_fLightB <= 0.95f ) g_fLightB += 0.05f;
		g_Light1.diffuse.z = g_fLightB;
		g_pLight->SetColor(g_Light1.diffuse);
		break;
	case 98: // b key
		if( g_fLightB >= 0.05f ) g_fLightB -= 0.05f;
		g_Light1.diffuse.z = g_fLightB;
		g_pLight->SetColor(g_Light1.diffuse);
		break;
//----------------------------------------------------------------------------
    case 033:
		glutIdleFunc( NULL );
		delete g_pFloor, g_pCeiling;
		delete g_pLeftWall1, g_pLeftWall2;
		delete g_pRightWall1, g_pRightWall2;
		delete g_pFrontWall1, g_pFrontWall2;
		delete g_pBackWall1 ,g_pBackWall2;
		for (int i = 0; i < 9; i++) {
			delete g_pLeftRoomWalls[i];
			delete g_pRightRoomWalls[i];
		}
		
		delete g_pLight;

		delete g_pStairs;

		delete g_pGemSweet;
		delete g_pGemToy;
		delete g_pGemGarden;

		CCamera::getInstance()->destroyInstance();
		CTexturePool::getInstance()->destroyInstance();
        exit( EXIT_SUCCESS );
        break;
    }
}

//----------------------------------------------------------------------------
void Win_Mouse(int button, int state, int x, int y) {
	switch(button) {
		case GLUT_LEFT_BUTTON:   // 目前按下的是滑鼠左鍵
			//if ( state == GLUT_DOWN ) ; 
			break;
		case GLUT_MIDDLE_BUTTON:  // 目前按下的是滑鼠中鍵 ，換成 Y 軸
			//if ( state == GLUT_DOWN ) ; 
			break;
		case GLUT_RIGHT_BUTTON:   // 目前按下的是滑鼠右鍵
			//if ( state == GLUT_DOWN ) ;
			break;
		default:
			break;
	} 
}
//----------------------------------------------------------------------------
void Win_SpecialKeyboard(int key, int x, int y) {

	switch(key) {
		case GLUT_KEY_UP:		// 目前按下的是向上方向鍵
			g_MoveDir = vec4(g_fRadius*sin(g_fTheta)*sin(g_fPhi), 0.f, g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.f);
			g_MoveDir = normalize(g_MoveDir);
			g_matMoveDir = Translate(g_MoveDir.x, 0.f, g_MoveDir.z);
			if (g_fCameraMoveX <= WALKING_SPACE && g_fCameraMoveX >= -WALKING_SPACE && g_fCameraMoveZ <= WALKING_SPACE && g_fCameraMoveZ >= -WALKING_SPACE) {	//限制空間
				g_fCameraMoveX += (g_matMoveDir._m[0][3] * 0.2f);
				g_fCameraMoveZ += (g_matMoveDir._m[2][3] * 0.2f);
			}
			else {	// 修正卡牆
				if (g_fCameraMoveX > WALKING_SPACE) g_fCameraMoveX = WALKING_SPACE;
				else if (g_fCameraMoveX < -WALKING_SPACE) g_fCameraMoveX = -WALKING_SPACE;
				if (g_fCameraMoveZ > WALKING_SPACE) g_fCameraMoveZ = WALKING_SPACE;
				else if (g_fCameraMoveZ < -WALKING_SPACE) g_fCameraMoveZ = -WALKING_SPACE;
			}
			break;
		case GLUT_KEY_DOWN:		// 目前按下的是向下方向鍵
			g_MoveDir = vec4(g_fRadius*sin(g_fTheta)*sin(g_fPhi), 0.f, g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.f);
			g_MoveDir = normalize(g_MoveDir);
			g_matMoveDir = Translate(g_MoveDir.x, 0.f, g_MoveDir.z);
			if (g_fCameraMoveX <= WALKING_SPACE && g_fCameraMoveX >= -WALKING_SPACE && g_fCameraMoveZ <= WALKING_SPACE && g_fCameraMoveZ >= -WALKING_SPACE) {	//限制空間
				g_fCameraMoveX -= (g_matMoveDir._m[0][3] * 0.2f);
				g_fCameraMoveZ -= (g_matMoveDir._m[2][3] * 0.2f);
			}
			else {	// 修正卡牆
				if (g_fCameraMoveX > WALKING_SPACE) g_fCameraMoveX = WALKING_SPACE;
				else if (g_fCameraMoveX < -WALKING_SPACE) g_fCameraMoveX = -WALKING_SPACE;
				if (g_fCameraMoveZ > WALKING_SPACE) g_fCameraMoveZ = WALKING_SPACE;
				else if (g_fCameraMoveZ < -WALKING_SPACE) g_fCameraMoveZ = -WALKING_SPACE;
			}
			break;
		case GLUT_KEY_LEFT:		// 目前按下的是向左方向鍵
			g_MoveDir = vec4(g_fRadius*sin(g_fTheta)*sin(g_fPhi), 0.f, g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.f);
			g_MoveDir = normalize(g_MoveDir);
			g_matMoveDir = RotateY(90.f) * Translate(g_MoveDir.x, 0.f, g_MoveDir.z);
			if (g_fCameraMoveX <= WALKING_SPACE && g_fCameraMoveX >= -WALKING_SPACE && g_fCameraMoveZ <= WALKING_SPACE && g_fCameraMoveZ >= -WALKING_SPACE) {	//限制空間
				g_fCameraMoveX += (g_matMoveDir._m[0][3] * 0.2f);
				g_fCameraMoveZ += (g_matMoveDir._m[2][3] * 0.2f);
			}
			else {	// 修正卡牆
				if (g_fCameraMoveX > WALKING_SPACE) g_fCameraMoveX = WALKING_SPACE;
				else if (g_fCameraMoveX < -WALKING_SPACE) g_fCameraMoveX = -WALKING_SPACE;
				if (g_fCameraMoveZ > WALKING_SPACE) g_fCameraMoveZ = WALKING_SPACE;
				else if (g_fCameraMoveZ < -WALKING_SPACE) g_fCameraMoveZ = -WALKING_SPACE;
			}
			break;
		case GLUT_KEY_RIGHT:	// 目前按下的是向右方向鍵
			g_MoveDir = vec4(g_fRadius*sin(g_fTheta)*sin(g_fPhi), 0.f, g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.f);
			g_MoveDir = normalize(g_MoveDir);
			g_matMoveDir = RotateY(90.f) * Translate(g_MoveDir.x, 0.f, g_MoveDir.z);
			if (g_fCameraMoveX <= WALKING_SPACE && g_fCameraMoveX >= -WALKING_SPACE && g_fCameraMoveZ <= WALKING_SPACE && g_fCameraMoveZ >= -WALKING_SPACE) {	//限制空間
				g_fCameraMoveX -= (g_matMoveDir._m[0][3] * 0.2f);
				g_fCameraMoveZ -= (g_matMoveDir._m[2][3] * 0.2f);
			}
			else {	// 修正卡牆
				if (g_fCameraMoveX > WALKING_SPACE) g_fCameraMoveX = WALKING_SPACE;
				else if (g_fCameraMoveX < -WALKING_SPACE) g_fCameraMoveX = -WALKING_SPACE;
				if (g_fCameraMoveZ > WALKING_SPACE) g_fCameraMoveZ = WALKING_SPACE;
				else if (g_fCameraMoveZ < -WALKING_SPACE) g_fCameraMoveZ = -WALKING_SPACE;
			}
			break;
		default:
			break;
	}
}

//----------------------------------------------------------------------------
// The passive motion callback for a window is called when the mouse moves within the window while no mouse buttons are pressed.
void Win_PassiveMotion(int x, int y) {
	g_fPhi = (float)M_PI*(x - HALF_SIZE) / (HALF_SIZE); // 轉換成 g_fPhi 介於 -PI 到 PI 之間 (-180 ~ 180 之間)
	g_fTheta = (float)-M_PI*(float)y / SCREEN_SIZE;
}

// The motion callback for a window is called when the mouse moves within the window while one or more mouse buttons are pressed.
void Win_MouseMotion(int x, int y) {
	g_fPhi = (float)M_PI*(x - HALF_SIZE) / (HALF_SIZE); // 轉換成 g_fPhi 介於 -PI 到 PI 之間 (-180 ~ 180 之間)
	g_fTheta = (float)-M_PI*(float)y / SCREEN_SIZE;
}
//----------------------------------------------------------------------------
void GL_Reshape(GLsizei w, GLsizei h)
{
	glViewport(0, 0, w, h);
	glClearColor( 0.0, 0.0, 0.0, 1.0 ); // black background
	glEnable(GL_DEPTH_TEST);
}

//----------------------------------------------------------------------------

int main( int argc, char **argv )
{
    
	glutInit(&argc, argv);
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( SCREEN_SIZE, SCREEN_SIZE );

	// If you use freeglut the two lines of code can be added to your application 
	glutInitContextVersion( 3, 2 );
	glutInitContextProfile( GLUT_CORE_PROFILE );

    glutCreateWindow("Six Room");

	// The glewExperimental global switch can be turned on by setting it to GL_TRUE before calling glewInit(), 
	// which ensures that all extensions with valid entry points will be exposed.
	glewExperimental = GL_TRUE; 
    glewInit();  

    init();

	glutMouseFunc(Win_Mouse);
	glutMotionFunc(Win_MouseMotion);
	glutPassiveMotionFunc(Win_PassiveMotion);  
    glutKeyboardFunc( Win_Keyboard );	// 處理 ASCI 按鍵如 A、a、ESC 鍵...等等
	glutSpecialFunc( Win_SpecialKeyboard);	// 處理 NON-ASCI 按鍵如 F1、Home、方向鍵...等等
    glutDisplayFunc( GL_Display );
	glutReshapeFunc( GL_Reshape );
	glutIdleFunc( IdleProcess );
	
    glutMainLoop();
    return 0;
}