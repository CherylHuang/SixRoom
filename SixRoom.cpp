// Cubic mapping (Environment Mapping)
// 
// 執行前的準備工作
// 關閉 CShape.h 中的 #define PERVERTEX_LIGHTING，使用 PERPIXEL_LIGHTING 才會有作用
// 設定 #define MULTITEXTURE  (DIFFUSE_MAP|LIGHT_MAP|NORMAL_MAP)
// 開啟 #define CUBIC_MAP 1

#include <time.h>
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
CQuad			*g_pFloor, *g_pCeiling;
CSolidCube		*g_pLeftWall1, *g_pLeftWall2, *g_pRightWall1, *g_pRightWall2;
CSolidCube		*g_pFrontWall1, *g_pFrontWall2, *g_pBackWall1, *g_pBackWall2;
CSolidCube		*g_pLeftRoomWalls[9], *g_pRightRoomWalls[9];
CSolidCube		*g_pDoorUpperWalls[4], *g_pBigDoorUpperWall, *g_pBigDoorBottomWall;
CSolidCube		*g_pBalconyWall;

CSolidCube		*g_pBigDoor;
CObjReader		*g_pStairs, *g_pDoors[4], *g_pFences[3];
CObjReader		*g_pGemSweet, *g_pGemToy, *g_pGemGarden, *g_pDiamond;
CObjReader		*g_pGun;

CQuad			*g_pFire;

CSolidSphere	*g_pSphere, *g_pSkyBox;

// For View Point
GLfloat g_fRadius = 8.0;
GLfloat g_fTheta = 45.0f*DegreesToRadians;
GLfloat g_fPhi = 45.0f*DegreesToRadians;
GLfloat g_fCameraMoveX = 0.f;				// for camera movment
GLfloat g_fCameraMoveY = 13.0f;				// for camera movment
GLfloat g_fCameraMoveZ = 10.f;				// for camera movment
mat4	g_matMoveDir;		// 鏡頭移動方向
point4  g_MoveDir;
point4  g_at;				// 鏡頭觀看點
point4  g_eye;				// 鏡頭位置

// For Billboard
point4	g_BillboardPos;		// billboard 位置
point4	g_CameraPos;		// Camera XY 位置
point4	g_ViewDir;			// 鏡頭觀看方向
point4	g_BillboardNormalDir = vec4(0.f, 0.0f, 1.f, 0.f);		// billboard 物件 normal 方向
GLfloat g_fBillboardTheta;	// billboard 旋轉角

//----------------------------------------------------------------------------
// Part 2 : for single light source
bool g_bAutoRotating = false;
float g_fElapsedTime = 0;
float g_fLightRadius = 6;
float g_fLightTheta = 0;

float g_fLightR = 0.85f;
float g_fLightG = 0.85f;
float g_fLightB = 0.85f;

LightSource g_Light_out_f = {									//室外前光源
	color4(0.95f, 0.95f, 0.95f, 1.0f),		// ambient 
	color4(0.95f, 0.95f, 0.95f, 1.0f),		// diffuse
	color4(0.95f, 0.95f, 0.95f, 1.0f),		// specular
	point4(0.0f, 10.0f, -50.0f, 1.0f),   // position
	point4(0.0f, 0.0f, 0.0f, 1.0f),   // halfVector
	vec3(0.0f, 0.0f, 0.0f),			  //spotDirection
	2.0f,	// spotExponent(parameter e); cos^(e)(phi) 
	90.0f,	// spotCutoff;	// (range: [0.0, 90.0], 180.0)  spot 的照明範圍
	0.707f,	// spotCosCutoff; // (range: [1.0,0.0],-1.0), 照明方向與被照明點之間的角度取 cos 後, cut off 的值
	1,	// constantAttenuation	(a + bd + cd^2)^-1 中的 a, d 為光源到被照明點的距離
	0,	// linearAttenuation	    (a + bd + cd^2)^-1 中的 b
	0		// quadraticAttenuation (a + bd + cd^2)^-1 中的 c
};

LightSource g_Light_out_r = {									//室外右光源
	color4(0.95f, 0.95f, 0.95f, 1.0f),		// ambient 
	color4(0.95f, 0.95f, 0.95f, 1.0f),		// diffuse
	color4(0.95f, 0.95f, 0.95f, 1.0f),		// specular
	point4(80.0f, 10.0f, 0.0f, 1.0f),   // position
	point4(0.0f, 0.0f, 0.0f, 1.0f),   // halfVector
	vec3(0.0f, 0.0f, 0.0f),			  //spotDirection
	2.0f,	// spotExponent(parameter e); cos^(e)(phi) 
	90.0f,	// spotCutoff;	// (range: [0.0, 90.0], 180.0)  spot 的照明範圍
	0.707f,	// spotCosCutoff; // (range: [1.0,0.0],-1.0), 照明方向與被照明點之間的角度取 cos 後, cut off 的值
	1,	// constantAttenuation	(a + bd + cd^2)^-1 中的 a, d 為光源到被照明點的距離
	0,	// linearAttenuation	    (a + bd + cd^2)^-1 中的 b
	0		// quadraticAttenuation (a + bd + cd^2)^-1 中的 c
};

LightSource g_Light_out_l = {									//室外左光源
	color4(0.95f, 0.95f, 0.95f, 1.0f),		// ambient 
	color4(0.95f, 0.95f, 0.95f, 1.0f),		// diffuse
	color4(0.95f, 0.95f, 0.95f, 1.0f),		// specular
	point4(-80.0f, 10.0f, 0.0f, 1.0f),   // position
	point4(0.0f, 0.0f, 0.0f, 1.0f),   // halfVector
	vec3(0.0f, 0.0f, 0.0f),			  //spotDirection
	2.0f,	// spotExponent(parameter e); cos^(e)(phi) 
	90.0f,	// spotCutoff;	// (range: [0.0, 90.0], 180.0)  spot 的照明範圍
	0.707f,	// spotCosCutoff; // (range: [1.0,0.0],-1.0), 照明方向與被照明點之間的角度取 cos 後, cut off 的值
	1,	// constantAttenuation	(a + bd + cd^2)^-1 中的 a, d 為光源到被照明點的距離
	0,	// linearAttenuation	    (a + bd + cd^2)^-1 中的 b
	0		// quadraticAttenuation (a + bd + cd^2)^-1 中的 c
};

LightSource g_Light_out_b = {									//室外後光源
	color4(0.95f, 0.95f, 0.95f, 1.0f),		// ambient 
	color4(0.95f, 0.95f, 0.95f, 1.0f),		// diffuse
	color4(0.95f, 0.95f, 0.95f, 1.0f),		// specular
	point4(0.0f, 10.0f, 50.0f, 1.0f),   // position
	point4(0.0f, 0.0f, 0.0f, 1.0f),   // halfVector
	vec3(0.0f, 0.0f, 0.0f),			  //spotDirection
	2.0f,	// spotExponent(parameter e); cos^(e)(phi) 
	90.0f,	// spotCutoff;	// (range: [0.0, 90.0], 180.0)  spot 的照明範圍
	0.707f,	// spotCosCutoff; // (range: [1.0,0.0],-1.0), 照明方向與被照明點之間的角度取 cos 後, cut off 的值
	1,	// constantAttenuation	(a + bd + cd^2)^-1 中的 a, d 為光源到被照明點的距離
	0,	// linearAttenuation	    (a + bd + cd^2)^-1 中的 b
	0		// quadraticAttenuation (a + bd + cd^2)^-1 中的 c
};

//------------------------------

CWireSphere *g_pLight;

LightSource g_Light_main = {
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

LightSource g_Light_left1 = {													//左1房間
	color4(1.0f, 1.0f, 1.0f, 1.0f), // ambient 
	color4(1.0f, 1.0f, 1.0f, 1.0f), // diffuse
	color4(1.0f, 1.0f, 1.0f, 1.0f), // specular
	point4(-FLOOR_SCALE * 0.75f, 7.0f, -FLOOR_SCALE / 4.0f, 1.0f),   // position
	point4(0.0f, 0.0f, 0.0f, 1.0f),   // halfVector
	vec3(0.0f, 0.0f, 0.0f),			  //spotDirection
	2.0f,	// spotExponent(parameter e); cos^(e)(phi) 
	45.0f,	// spotCutoff;	// (range: [0.0, 90.0], 180.0)  spot 的照明範圍
	0.707f,	// spotCosCutoff; // (range: [1.0,0.0],-1.0), 照明方向與被照明點之間的角度取 cos 後, cut off 的值
	1,	// constantAttenuation	(a + bd + cd^2)^-1 中的 a, d 為光源到被照明點的距離
	0,	// linearAttenuation	    (a + bd + cd^2)^-1 中的 b
	0		// quadraticAttenuation (a + bd + cd^2)^-1 中的 c
};

LightSource g_Light_left2 = {													//左2房間
	color4(1.0f, 1.0f, 1.0f, 1.0f), // ambient 
	color4(1.0f, 1.0f, 1.0f, 1.0f), // diffuse
	color4(1.0f, 1.0f, 1.0f, 1.0f), // specular
	point4(-FLOOR_SCALE * 0.75f, 7.0f, FLOOR_SCALE / 4.0f, 1.0f),   // position
	point4(0.0f, 0.0f, 0.0f, 1.0f),   // halfVector
	vec3(0.0f, 0.0f, 0.0f),			  //spotDirection
	2.0f,	// spotExponent(parameter e); cos^(e)(phi) 
	45.0f,	// spotCutoff;	// (range: [0.0, 90.0], 180.0)  spot 的照明範圍
	0.707f,	// spotCosCutoff; // (range: [1.0,0.0],-1.0), 照明方向與被照明點之間的角度取 cos 後, cut off 的值
	1,	// constantAttenuation	(a + bd + cd^2)^-1 中的 a, d 為光源到被照明點的距離
	0,	// linearAttenuation	    (a + bd + cd^2)^-1 中的 b
	0		// quadraticAttenuation (a + bd + cd^2)^-1 中的 c
};

LightSource g_Light_right1 = {													//右1房間
	color4(1.0f, 1.0f, 1.0f, 1.0f), // ambient 
	color4(1.0f, 1.0f, 1.0f, 1.0f), // diffuse
	color4(1.0f, 1.0f, 1.0f, 1.0f), // specular
	point4(FLOOR_SCALE * 0.75f, 7.0f, -FLOOR_SCALE / 4.0f, 1.0f),   // position
	point4(0.0f, 0.0f, 0.0f, 1.0f),   // halfVector
	vec3(0.0f, 0.0f, 0.0f),			  //spotDirection
	2.0f,	// spotExponent(parameter e); cos^(e)(phi) 
	45.0f,	// spotCutoff;	// (range: [0.0, 90.0], 180.0)  spot 的照明範圍
	0.707f,	// spotCosCutoff; // (range: [1.0,0.0],-1.0), 照明方向與被照明點之間的角度取 cos 後, cut off 的值
	1,	// constantAttenuation	(a + bd + cd^2)^-1 中的 a, d 為光源到被照明點的距離
	0,	// linearAttenuation	    (a + bd + cd^2)^-1 中的 b
	0		// quadraticAttenuation (a + bd + cd^2)^-1 中的 c
};

LightSource g_Light_right2 = {													//右2房間
	color4(1.0f, 1.0f, 1.0f, 1.0f), // ambient 
	color4(1.0f, 1.0f, 1.0f, 1.0f), // diffuse
	color4(1.0f, 1.0f, 1.0f, 1.0f), // specular
	point4(FLOOR_SCALE * 0.75f, 7.0f, FLOOR_SCALE / 4.0f, 1.0f),   // position
	point4(0.0f, 0.0f, 0.0f, 1.0f),   // halfVector
	vec3(0.0f, 0.0f, 0.0f),			  //spotDirection
	2.0f,	// spotExponent(parameter e); cos^(e)(phi) 
	45.0f,	// spotCutoff;	// (range: [0.0, 90.0], 180.0)  spot 的照明範圍
	0.707f,	// spotCosCutoff; // (range: [1.0,0.0],-1.0), 照明方向與被照明點之間的角度取 cos 後, cut off 的值
	1,	// constantAttenuation	(a + bd + cd^2)^-1 中的 a, d 為光源到被照明點的距離
	0,	// linearAttenuation	    (a + bd + cd^2)^-1 中的 b
	0		// quadraticAttenuation (a + bd + cd^2)^-1 中的 c
};

//----------------------------------------------------------------------------

// Texture 
GLuint g_uiFTexID[30]; // 三個物件分別給不同的貼圖
int g_iTexWidth,  g_iTexHeight;
GLuint g_uiSphereCubeMap; // for Cubic Texture

//----------------------------------------------------------------------------
// 函式的原型宣告
extern void IdleProcess();
void CreateWalls();

void init( void )
{
	srand((unsigned)time(NULL));

	mat4 mxT, mxS;
	vec4 vT;
	vec3 vS;
	// 產生所需之 Model View 與 Projection Matrix
	// 產生所需之 Model View 與 Projection Matrix
	point4  eye(g_fRadius*sin(g_fTheta)*sin(g_fPhi), g_fRadius*cos(g_fTheta), g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.0f);
	point4  at(0.0f, 0.0f, 0.0f, 1.0f);
	auto camera = CCamera::create();
	camera->updateViewLookAt(eye, at);
	camera->updatePerspective(60.0, (GLfloat)SCREEN_SIZE / (GLfloat)SCREEN_SIZE, 0.0, 1000.0);

	auto texturepool = CTexturePool::create();
	g_uiFTexID[0] = texturepool->AddTexture("texture/DiffuseMap/stone-tile-1.png");		// floor
	g_uiFTexID[1] = texturepool->AddTexture("texture/NormalMap/stone-tile-1_NRM.png");	// floor Normal
	g_uiFTexID[2] = texturepool->AddTexture("texture/metal.png");						// cubmap(sphere) & fence
	g_uiFTexID[3] = texturepool->AddTexture("texture/DiffuseMap/stair.png");			// stair Diffuse
	g_uiFTexID[4] = texturepool->AddTexture("texture/NormalMap/stair_NRM.png");			// stair Normal
	g_uiFTexID[5] = texturepool->AddTexture("texture/LightMap/star2.png");				// red gem Light
	g_uiFTexID[6] = texturepool->AddTexture("texture/LightMap/star4.png");				// blue gem Light
	g_uiFTexID[7] = texturepool->AddTexture("texture/LightMap/flower2.png");			// green gem Light
	g_uiFTexID[8] = texturepool->AddTexture("texture/LightMap/flower.png");				// diamond Light
	g_uiFTexID[9] = texturepool->AddTexture("texture/DiffuseMap/door.png");				// door Diffuse
	g_uiFTexID[10] = texturepool->AddTexture("texture/NormalMap/door_NRM.png");			// door Normal
	g_uiFTexID[11] = texturepool->AddTexture("texture/DiffuseMap/wood.png");			// big door Diffuse
	g_uiFTexID[12] = texturepool->AddTexture("texture/NormalMap/wood_NRM.png");			// big door Normal
	g_uiFTexID[13] = texturepool->AddTexture("texture/DiffuseMap/fireParticle.png");	// fire particle
	g_uiFTexID[14] = texturepool->AddTexture("texture/lightMap2.png");					// fire Light
	g_uiFTexID[15] = texturepool->AddTexture("texture/DiffuseMap/gun.png");				// gun Diffuse
	g_uiFTexID[16] = texturepool->AddTexture("texture/NormalMap/gun_NRM.png");			// gun Normal
	
	g_uiSphereCubeMap = CubeMap_load_SOIL();	// Cub Map 設置


	// ------------------- 產生物件的實體 --------------------
	CreateWalls();	//房間結構牆

	//---------------------------------------------

	for (int i = 0; i < 4; i++) {
		g_pDoors[i] = new CObjReader("obj/door.obj");											// Doors
		g_pDoors[i]->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
		g_pDoors[i]->SetShader();
		if (i == 0) {			// Left door 1
			vT.x = -FLOOR_SCALE / 2.0f; vT.y = 0.5f; vT.z = -FLOOR_SCALE / 4.0f - FLOOR_SCALE / 6.0f + 0.3f;
			mxT = Translate(vT);
			vS.x = vS.y = vS.z = 0.8f;				//Scale
			mxS = Scale(vS);
			g_pDoors[i]->SetTRSMatrix(mxT * RotateY(90.0f) * mxS);
		}
		else if (i == 1) {		// Left door 2
			vT.x = -FLOOR_SCALE / 2.0f; vT.y = 0.5f; vT.z = FLOOR_SCALE / 4.0f + FLOOR_SCALE / 6.0f - 0.3f;
			mxT = Translate(vT);
			vS.x = vS.y = vS.z = 0.8f;				//Scale
			mxS = Scale(vS);
			g_pDoors[i]->SetTRSMatrix(mxT * RotateY(-90.0f) * mxS);
		}
		else if (i == 2) {		// Right door 1
			vT.x = FLOOR_SCALE / 2.0f; vT.y = 0.5f; vT.z = -FLOOR_SCALE / 4.0f - FLOOR_SCALE / 6.0f + 0.3f;
			mxT = Translate(vT);
			vS.x = vS.y = vS.z = 0.8f;				//Scale
			mxS = Scale(vS);
			g_pDoors[i]->SetTRSMatrix(mxT * RotateY(90.0f) * mxS);
		}
		else if (i == 3) {		// Right door 2
			vT.x = FLOOR_SCALE / 2.0f; vT.y = 0.5f; vT.z = FLOOR_SCALE / 4.0f + FLOOR_SCALE / 6.0f - 0.3f;
			mxT = Translate(vT);
			vS.x = vS.y = vS.z = 0.8f;				//Scale
			mxS = Scale(vS);
			g_pDoors[i]->SetTRSMatrix(mxT * RotateY(-90.0f) * mxS);
		}
		g_pDoors[i]->SetShadingMode(GOURAUD_SHADING);
		g_pDoors[i]->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
		g_pDoors[i]->SetKaKdKsShini(0.25f, 0.8f, 0.2f, 2);
	}

	//----------------------------------------------------------------------

	//SkyBox
	g_pSkyBox = new CSolidSphere(1.0f, 24, 12);
	g_pSkyBox->SetTextureLayer(DIFFUSE_MAP);
	g_pSkyBox->SetCubeMapTexName(1);
	g_pSkyBox->SetViewPosition(eye);
	g_pSkyBox->SetShaderName("vsSkyBox.glsl", "fsSkyBox.glsl");		// SkyBox shader
	g_pSkyBox->SetShader();
	vT.x = 0.0f; vT.y = 0.0f; vT.z = 0.0f;
	mxT = Translate(vT);
	mxT._m[0][0] = mxT._m[1][1] = mxT._m[2][2] = 990.0f;
	g_pSkyBox->SetTRSMatrix(mxT*RotateX(90.0f));
	g_pSkyBox->SetShadingMode(GOURAUD_SHADING);
	// 設定貼圖
	g_pSkyBox->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pSkyBox->SetKaKdKsShini(0, 0.8f, 0.5f, 1);
	g_pSkyBox->SetColor(vec4(0.9f, 0.9f, 0.9f, 1.0f));

	// For Reflecting Sphere
	g_pSphere = new CSolidSphere(1.0f, 24, 12);
	g_pSphere->SetTextureLayer(DIFFUSE_MAP);
	g_pSphere->SetCubeMapTexName(1);
	g_pSphere->SetViewPosition(eye);
	g_pSphere->SetShaderName("vsCubeMapping.glsl", "fsCubeMapping.glsl");
	g_pSphere->SetShader();
	vT.x = -0.4f; vT.y = 15.0f; vT.z = -FLOOR_SCALE / 2.0f - FLOOR_SCALE / 12.0f + 0.2f;
	mxT = Translate(vT);
	mxT._m[0][0] = mxT._m[1][1] = mxT._m[2][2] = 1.0f;	//Scale
	g_pSphere->SetTRSMatrix(mxT*RotateX(90.0f));
	g_pSphere->SetShadingMode(GOURAUD_SHADING);
	// 設定貼圖
	g_pSphere->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pSphere->SetKaKdKsShini(0, 0.8f, 0.5f, 1);
	g_pSphere->SetColor(vec4(0.9f, 0.9f, 0.9f, 1.0f));

	//----------------------------------------------------------------------

	g_pStairs = new CObjReader("obj/stairs_fix.obj");						//階梯
	g_pStairs->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);	//貼圖
	g_pStairs->SetMaterials(vec4(0), vec4(0.95f, 0.95f, 0.95f, 1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f));	// materials
	g_pStairs->SetKaKdKsShini(0.15f, 0.95f, 0.5f, 5);
	g_pStairs->SetShader();
	vT.x = 0.0f; vT.y = 0.0f; vT.z = -20.3f;	//Location
	mxT = Translate(vT);
	vS.x = vS.y = vS.z = 0.3f;				//Scale
	mxS = Scale(vS);
	g_pStairs->SetTRSMatrix(mxT /** mxS*/);
	g_pStairs->SetShadingMode(GOURAUD_SHADING);

	g_pBigDoor = new CSolidCube;											// 大門
	g_pBigDoor->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
	g_pBigDoor->SetShader();
	vT.x = 0.f; vT.y = FLOOR_SCALE / 3.0f - 2.6f; vT.z = -FLOOR_SCALE / 2.0f;
	mxT = Translate(vT);
	mxS = Scale(FLOOR_SCALE / 6.0f, 0.5f, FLOOR_SCALE / 3.0f - 0.4f);
	g_pBigDoor->SetTRSMatrix(mxT * RotateX(90.0f) * mxS);
	g_pBigDoor->SetShadingMode(GOURAUD_SHADING);
	g_pBigDoor->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pBigDoor->SetKaKdKsShini(0.25f, 0.8f, 0.2f, 2);

	for (int i = 0; i < 3; i++) {
		g_pFences[i] = new CObjReader("obj/fence.obj");							// 圍欄
		g_pFences[i]->SetTextureLayer(DIFFUSE_MAP);
		g_pFences[i]->SetShader();
		if (i == 0) {
			vT.x = -0.4f; vT.y = 7.4f; vT.z = -FLOOR_SCALE / 2.0f - FLOOR_SCALE / 6.0f + 0.2f;
			mxT = Translate(vT);
			mxS = Scale(0.4f, 0.4f, 0.57f);
			g_pFences[i]->SetTRSMatrix(mxT * RotateY(90.f) * mxS);
		}
		else if (i == 1) {
			vT.x = FLOOR_SCALE / 12.0f + 2.9f; vT.y = 7.4f; vT.z = -FLOOR_SCALE / 2.0f - FLOOR_SCALE / 12.0f - 0.3f;
			mxT = Translate(vT);
			mxS = Scale(0.4f, 0.4f, 0.34f);
			g_pFences[i]->SetTRSMatrix(mxT * mxS);
		}
		else {
			vT.x = -FLOOR_SCALE / 12.0f - 2.9f; vT.y = 7.4f; vT.z = -FLOOR_SCALE / 2.0f - FLOOR_SCALE / 12.0f - 0.3f;
			mxT = Translate(vT);
			mxS = Scale(0.4f, 0.4f, 0.34f);
			g_pFences[i]->SetTRSMatrix(mxT * mxS);
		}
		g_pFences[i]->SetShadingMode(GOURAUD_SHADING);
		g_pFences[i]->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
		g_pFences[i]->SetKaKdKsShini(0.25f, 0.8f, 0.2f, 2);
	}

	//----------------------------------------------------------------------
	g_pGemSweet = new CObjReader("obj/gem_sweet.obj");			//紅水晶
	g_pGemSweet->SetTextureLayer(DIFFUSE_MAP | LIGHT_MAP);	//貼圖
	g_pGemSweet->SetLightMapColor(vec4(1.0f, 0.3f, 0.3f, 0.5f));	//Light Map : 半透明粉色
	// materials
	g_pGemSweet->SetMaterials(vec4(0), vec4(0.85f, 0, 0, 0.7f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pGemSweet->SetKaKdKsShini(0.15f, 0.95f, 0.5f, 5);
	g_pGemSweet->SetShader();
	vT.x = -FLOOR_SCALE * 0.75f; vT.y = 1.5f; vT.z = -FLOOR_SCALE / 4.0f;	//Location : LR1
	mxT = Translate(vT);
	vS.x = vS.y = vS.z = 0.7f;				//Scale
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
	vS.x = vS.y = vS.z = 0.7f;				//Scale
	mxS = Scale(vS);
	g_pGemToy->SetTRSMatrix(mxT * RotateY(90.f) * mxS);
	g_pGemToy->SetShadingMode(GOURAUD_SHADING);

	//-----------------------------------------
	g_pGemGarden = new CObjReader("obj/gem_garden.obj");		//綠水晶
	g_pGemGarden->SetTextureLayer(DIFFUSE_MAP | LIGHT_MAP);	//貼圖
	g_pGemGarden->SetLightMapColor(vec4(1.0f, 1.0f, 0.0f, 0.3f));	//Light Map : 半透明黃色
	// materials
	g_pGemGarden->SetMaterials(vec4(0), vec4(0.f, 0.85f, 0.f, 0.7f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pGemGarden->SetKaKdKsShini(0.15f, 0.95f, 0.95f, 5);
	g_pGemGarden->SetShader();
	vT.x = FLOOR_SCALE * 0.75f; vT.y = 1.5f; vT.z = -FLOOR_SCALE / 4.0f;	//Location : RR1
	mxT = Translate(vT);
	vS.x = vS.y = vS.z = 0.7f;				//Scale
	mxS = Scale(vS);
	g_pGemGarden->SetTRSMatrix(mxT * mxS);
	g_pGemGarden->SetShadingMode(GOURAUD_SHADING);

	//-----------------------------------------
	g_pDiamond = new CObjReader("obj/diamond.obj");				//鑽石
	g_pDiamond->SetTextureLayer(DIFFUSE_MAP | LIGHT_MAP);	//貼圖
	g_pDiamond->SetLightMapColor(vec4(0.95f, 0.5f, 0.f, 0.3f));	//Light Map : 半透明橘色
	// materials
	g_pDiamond->SetMaterials(vec4(0), vec4(0.95f, 0.f, 0.95f, 0.7f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pDiamond->SetKaKdKsShini(0.15f, 0.95f, 0.95f, 5);
	g_pDiamond->SetShader();
	vT.x = FLOOR_SCALE * 0.75f; vT.y = 1.0f; vT.z = FLOOR_SCALE / 4.0f;	//Location : RR2
	mxT = Translate(vT);
	vS.x = vS.y = vS.z = 0.35f;				//Scale
	mxS = Scale(vS);
	g_pDiamond->SetTRSMatrix(mxT * mxS);
	g_pDiamond->SetShadingMode(GOURAUD_SHADING);

	//---------------------------------------------------------------------------

	g_pGun = new CObjReader("obj/gun.obj");						//槍
	g_pGun->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);	//貼圖
	g_pGun->SetMaterials(vec4(0), vec4(0.95f, 0.95f, 0.95f, 1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f));	// materials
	g_pGun->SetKaKdKsShini(0.15f, 0.95f, 0.5f, 5);
	g_pGun->SetShader();
	vT.x = 0.0f; vT.y = 5.0f; vT.z = 0.0f;	//Location
	mxT = Translate(vT);
	vS.x = vS.y = vS.z = 0.3f;				//Scale
	mxS = Scale(vS);
	g_pGun->SetTRSMatrix(mxT * mxS);
	g_pGun->SetShadingMode(GOURAUD_SHADING);

	//---------------------------------------------------------------------------
	
	g_pFire = new CQuad;											// Fire
	g_pFire->SetTextureLayer(DIFFUSE_MAP | LIGHT_MAP);
	//g_pFire->SetLightMapColor(vec4(0.95f, 0.95f, 0.95f, 0.8f));
	g_pFire->SetShader();
	vT.x = 0.f; vT.y = 5.0f; vT.z = -5.0f;		// Location
	mxT = Translate(vT);
	vS.x = vS.y = vS.z = 5.0f;						// Scale
	mxS = Scale(vS);
	g_pFire->SetTRSMatrix(mxT * RotateX(90.f) * mxS);
	g_pFire->SetShadingMode(GOURAUD_SHADING);
	g_pFire->SetTiling(1, 1);					// 設定貼圖
	g_pFire->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 0.5f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pFire->SetKaKdKsShini(0, 0.8f, 0.5f, 1);

	// for billboard
	vT.y = 0.0f; vT.w = 0.f;		// Location
	g_BillboardPos = vT;

	//------------------------------------------
	// 設定 代表 Light 的 WireSphere
	g_pLight = new CWireSphere(0.25f, 6, 3);
	g_pLight->SetLightingDisable();
	g_pLight->SetTextureLayer(NONE_MAP);	// 沒有貼圖
	g_pLight->SetShader();
	mxT = Translate(g_Light_main.position);
	g_pLight->SetTRSMatrix(mxT);
	g_pLight->SetColor(g_Light_main.diffuse);


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
	for (int i = 0; i < 4; i++) {
		g_pDoors[i]->SetProjectionMatrix(mpx);
		g_pDoorUpperWalls[i]->SetProjectionMatrix(mpx);
	}
	for (int i = 0; i < 9; i++) {
		g_pLeftRoomWalls[i]->SetProjectionMatrix(mpx);
		g_pRightRoomWalls[i]->SetProjectionMatrix(mpx);
	}
	g_pBigDoorUpperWall->SetProjectionMatrix(mpx);
	g_pBigDoorBottomWall->SetProjectionMatrix(mpx);
	g_pBalconyWall->SetProjectionMatrix(mpx);

	g_pFire->SetProjectionMatrix(mpx);

	g_pSkyBox->SetProjectionMatrix(mpx);
	g_pSphere->SetProjectionMatrix(mpx);
	g_pLight->SetProjectionMatrix(mpx);

	g_pStairs->SetProjectionMatrix(mpx);
	g_pBigDoor->SetProjectionMatrix(mpx);
	for (int i = 0; i < 3; i++) g_pFences[i]->SetProjectionMatrix(mpx);

	g_pGemSweet->SetProjectionMatrix(mpx);
	g_pGemToy->SetProjectionMatrix(mpx);
	g_pGemGarden->SetProjectionMatrix(mpx);
	g_pDiamond->SetProjectionMatrix(mpx);

	g_pGun->SetProjectionMatrix(mpx);
}
//----------------------------------------------------------------------------

void CreateWalls()
{
	mat4 mxT, mxS;
	vec4 vT;
	vec3 vS;

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
	g_pLeftWall1->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
	g_pLeftWall1->SetShader();
	vT.x = -FLOOR_SCALE / 2.0f; vT.y = FLOOR_SCALE / 4.0f; vT.z = -FLOOR_SCALE / 4.0f + FLOOR_SCALE / 12.0f;	//+ FLOOR_SCALE / 12.0f : 門框空間
	mxT = Translate(vT);
	mxS = Scale(FLOOR_SCALE / 3.0f, 0.5f, FLOOR_SCALE / 2.0f);
	g_pLeftWall1->SetTRSMatrix(mxT * RotateX(90.0f) * RotateZ(90.0f) * mxS);
	g_pLeftWall1->SetShadingMode(GOURAUD_SHADING);
	g_pLeftWall1->SetTiling(3, 3);					// 設定貼圖
	g_pLeftWall1->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pLeftWall1->SetKaKdKsShini(0.25f, 0.8f, 0.2f, 2);

	g_pLeftWall2 = new CSolidCube;											// Left Wall-2
	g_pLeftWall2->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
	g_pLeftWall2->SetShader();
	vT.x = -FLOOR_SCALE / 2.0f; vT.y = FLOOR_SCALE / 4.0f; vT.z = FLOOR_SCALE / 4.0f - FLOOR_SCALE / 12.0f;
	mxT = Translate(vT);
	g_pLeftWall2->SetTRSMatrix(mxT * RotateX(90.0f) * RotateZ(90.0f) * mxS);
	g_pLeftWall2->SetShadingMode(GOURAUD_SHADING);
	g_pLeftWall2->SetTiling(3, 3);					// 設定貼圖
	g_pLeftWall2->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pLeftWall2->SetKaKdKsShini(0.25f, 0.8f, 0.2f, 2);

	g_pRightWall1 = new CSolidCube;											// Right Wall-1
	g_pRightWall1->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
	g_pRightWall1->SetShader();
	vT.x = FLOOR_SCALE / 2.0f; vT.y = FLOOR_SCALE / 4.0f; vT.z = -FLOOR_SCALE / 4.0f + FLOOR_SCALE / 12.0f;
	mxT = Translate(vT);
	g_pRightWall1->SetTRSMatrix(mxT * RotateX(90.0f) * RotateZ(90.0f) * mxS);
	g_pRightWall1->SetShadingMode(GOURAUD_SHADING);
	g_pRightWall1->SetTiling(3, 3);					// 設定貼圖
	g_pRightWall1->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pRightWall1->SetKaKdKsShini(0.25f, 0.8f, 0.2f, 2);

	g_pRightWall2 = new CSolidCube;											// Right Wall-2
	g_pRightWall2->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
	g_pRightWall2->SetShader();
	vT.x = FLOOR_SCALE / 2.0f; vT.y = FLOOR_SCALE / 4.0f; vT.z = FLOOR_SCALE / 4.0f - FLOOR_SCALE / 12.0f;
	mxT = Translate(vT);
	g_pRightWall2->SetTRSMatrix(mxT * RotateX(90.0f) * RotateZ(90.0f) * mxS);
	g_pRightWall2->SetShadingMode(GOURAUD_SHADING);
	g_pRightWall2->SetTiling(3, 3);					// 設定貼圖
	g_pRightWall2->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pRightWall2->SetKaKdKsShini(0.25f, 0.8f, 0.2f, 2);

	g_pFrontWall1 = new CSolidCube;											// Front Wall-1
	g_pFrontWall1->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
	g_pFrontWall1->SetShader();
	vT.x = -FLOOR_SCALE / 4.0f - FLOOR_SCALE / 24.0f; vT.y = FLOOR_SCALE / 4.0f; vT.z = -FLOOR_SCALE / 2.0f;
	mxT = Translate(vT);
	mxS = Scale(FLOOR_SCALE * 5.0f / 12.0f, 0.5f, FLOOR_SCALE / 2.0f);
	g_pFrontWall1->SetTRSMatrix(mxT * RotateX(90.0f) * mxS);
	g_pFrontWall1->SetShadingMode(GOURAUD_SHADING);
	g_pFrontWall1->SetTiling(3, 3);					// 設定貼圖
	g_pFrontWall1->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pFrontWall1->SetKaKdKsShini(0.25f, 0.8f, 0.2f, 2);

	g_pFrontWall2 = new CSolidCube;											// Front Wall-2
	g_pFrontWall2->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
	g_pFrontWall2->SetShader();
	vT.x = FLOOR_SCALE / 4.0f + FLOOR_SCALE / 24.0f; vT.y = FLOOR_SCALE / 4.0f; vT.z = -FLOOR_SCALE / 2.0f;
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
	mxS = Scale(FLOOR_SCALE / 2.0f, 0.5f, FLOOR_SCALE / 2.0f);
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

	//--------------------------------------------------

	for (int i = 0; i < 4; i++) {
		g_pDoorUpperWalls[i] = new CSolidCube;											// Doors upper walls : 玻璃
		g_pDoorUpperWalls[i]->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
		g_pDoorUpperWalls[i]->SetShader();
		if (i == 0) {			// Left door 1
			vT.x = -FLOOR_SCALE / 2.0f; vT.y = FLOOR_SCALE / 4.0f + FLOOR_SCALE / 6.0f - 0.1f; vT.z = -FLOOR_SCALE / 4.0f - FLOOR_SCALE / 6.0f;
			mxT = Translate(vT);
			mxS = Scale(FLOOR_SCALE / 6.0f, 0.5f, FLOOR_SCALE / 6.0f);
			g_pDoorUpperWalls[i]->SetTRSMatrix(mxT * RotateX(90.0f) * RotateZ(90.0f) * mxS);
		}
		else if (i == 1) {		// Left door 2
			vT.x = -FLOOR_SCALE / 2.0f; vT.y = FLOOR_SCALE / 4.0f + FLOOR_SCALE / 6.0f - 0.1f; vT.z = FLOOR_SCALE / 4.0f + FLOOR_SCALE / 6.0f;
			mxT = Translate(vT);
			mxS = Scale(FLOOR_SCALE / 6.0f, 0.5f, FLOOR_SCALE / 6.0f);
			g_pDoorUpperWalls[i]->SetTRSMatrix(mxT * RotateX(90.0f) * RotateZ(90.0f) * mxS);
		}
		else if (i == 2) {		// Right door 1
			vT.x = FLOOR_SCALE / 2.0f; vT.y = FLOOR_SCALE / 4.0f + FLOOR_SCALE / 6.0f - 0.1f; vT.z = -FLOOR_SCALE / 4.0f - FLOOR_SCALE / 6.0f;
			mxT = Translate(vT);
			mxS = Scale(FLOOR_SCALE / 6.0f, 0.5f, FLOOR_SCALE / 6.0f);
			g_pDoorUpperWalls[i]->SetTRSMatrix(mxT * RotateX(90.0f) * RotateZ(90.0f) * mxS);
		}
		else if (i == 3) {		// Right door 2
			vT.x = FLOOR_SCALE / 2.0f; vT.y = FLOOR_SCALE / 4.0f + FLOOR_SCALE / 6.0f - 0.1f; vT.z = FLOOR_SCALE / 4.0f + FLOOR_SCALE / 6.0f;
			mxT = Translate(vT);
			mxS = Scale(FLOOR_SCALE / 6.0f, 0.5f, FLOOR_SCALE / 6.0f);
			g_pDoorUpperWalls[i]->SetTRSMatrix(mxT * RotateX(90.0f) * RotateZ(90.0f) * mxS);
		}
		g_pDoorUpperWalls[i]->SetMaterials(vec4(0), vec4(0.1f, 0.1f, 0.1f, 0.3f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
		g_pDoorUpperWalls[i]->SetKaKdKsShini(0.15f, 0.95f, 0.95f, 5);
		g_pDoorUpperWalls[i]->SetShadingMode(GOURAUD_SHADING);
	}

	//--------------------------------------------------------------------------
	for (int i = 0; i < 9; i++) {
		g_pLeftRoomWalls[i] = new CSolidCube;											// Left Room Walls
		g_pLeftRoomWalls[i]->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
		g_pLeftRoomWalls[i]->SetShader();
		if (i == 0) {			// Left side 1
			vT.x = -FLOOR_SCALE; vT.y = FLOOR_SCALE / 4.0f; vT.z = -FLOOR_SCALE / 4.0f;
			mxT = Translate(vT);
			mxS = Scale(FLOOR_SCALE / 2.0f, 0.5f, FLOOR_SCALE / 2.0f);
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
			vT.x = -FLOOR_SCALE * 0.75f; vT.y = 0.f; vT.z = -FLOOR_SCALE / 4.0f;
			mxT = Translate(vT);
			g_pLeftRoomWalls[i]->SetTRSMatrix(mxT * mxS);
		}
		else if (i == 6) {		// Left Ceiling 1
			vT.x = -FLOOR_SCALE * 0.75f; vT.y = FLOOR_SCALE / 2.0f + 0.5f; vT.z = -FLOOR_SCALE / 4.0f;
			mxT = Translate(vT);
			g_pLeftRoomWalls[i]->SetTRSMatrix(mxT * mxS);
		}
		else if (i == 7) {		// Left Floor 2
			vT.x = -FLOOR_SCALE * 0.75f; vT.y = 0.f; vT.z = FLOOR_SCALE / 4.0f;
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

		//---------------------------------------

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
			vT.x = FLOOR_SCALE * 0.75f; vT.y = 0.f; vT.z = -FLOOR_SCALE / 4.0f;
			mxT = Translate(vT);
			g_pRightRoomWalls[i]->SetTRSMatrix(mxT * mxS);
		}
		else if (i == 6) {		// Right Ceiling 1
			vT.x = FLOOR_SCALE * 0.75f; vT.y = FLOOR_SCALE / 2.0f + 0.5f; vT.z = -FLOOR_SCALE / 4.0f;
			mxT = Translate(vT);
			g_pRightRoomWalls[i]->SetTRSMatrix(mxT * mxS);
		}
		else if (i == 7) {		// Right Floor 2
			vT.x = FLOOR_SCALE * 0.75f; vT.y = 0.f; vT.z = FLOOR_SCALE / 4.0f;
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

	//---------------------------------------------

	g_pBigDoorUpperWall = new CSolidCube;											// 大門上方 小牆
	g_pBigDoorUpperWall->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
	g_pBigDoorUpperWall->SetShader();
	vT.x = 0.f; vT.y = FLOOR_SCALE / 2.f - 1.4f; vT.z = -FLOOR_SCALE / 2.0f;
	mxT = Translate(vT);
	mxS = Scale(FLOOR_SCALE / 6.0f, 0.5f, 2.8f);
	g_pBigDoorUpperWall->SetTRSMatrix(mxT * RotateX(90.0f) * mxS);
	g_pBigDoorUpperWall->SetShadingMode(GOURAUD_SHADING);
	g_pBigDoorUpperWall->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pBigDoorUpperWall->SetKaKdKsShini(0.25f, 0.8f, 0.2f, 2);

	//-----------------

	g_pBigDoorBottomWall = new CSolidCube;											// 大門下方 補牆
	g_pBigDoorBottomWall->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
	g_pBigDoorBottomWall->SetShader();
	vT.x = 0.f; vT.y = FLOOR_SCALE / 12.f - 1.2f; vT.z = -FLOOR_SCALE / 2.0f;
	mxT = Translate(vT);
	mxS = Scale(FLOOR_SCALE / 6.0f, 0.5f, FLOOR_SCALE / 6.0f - 2.4f);
	g_pBigDoorBottomWall->SetTRSMatrix(mxT * RotateX(90.0f) * mxS);
	g_pBigDoorBottomWall->SetShadingMode(GOURAUD_SHADING);
	g_pBigDoorBottomWall->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pBigDoorBottomWall->SetKaKdKsShini(0.25f, 0.8f, 0.2f, 2);

	//---------------------------------------------
	
	g_pBalconyWall = new CSolidCube;											// 陽台地板
	g_pBalconyWall->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
	g_pBalconyWall->SetShader();
	vT.x = 0.f; vT.y = 7.2f; vT.z = -FLOOR_SCALE / 2.0f - FLOOR_SCALE / 12.0f + 0.06f;
	mxT = Translate(vT);
	mxS = Scale(FLOOR_SCALE / 6.0f + 6.f, 0.5f, FLOOR_SCALE / 6.0f);
	g_pBalconyWall->SetTRSMatrix(mxT * mxS);
	g_pBalconyWall->SetShadingMode(GOURAUD_SHADING);
	g_pBalconyWall->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pBalconyWall->SetKaKdKsShini(0.25f, 0.8f, 0.2f, 2);
}

//----------------------------------------------------------------------------
void GL_Display( void )
{

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); // clear the window

	glEnable(GL_BLEND);								// 設定2D Texure Mapping 有作用
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//WALLS
	glActiveTexture(GL_TEXTURE0);					// select active texture 0
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[0]);	// 與 Diffuse Map 結合
	glActiveTexture(GL_TEXTURE2);					// normal map
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[1]);
	g_pFloor->Draw();
	g_pCeiling->Draw();
	g_pLeftWall1->Draw();
	g_pLeftWall2->Draw();
	g_pRightWall1->Draw();
	g_pRightWall2->Draw();
	g_pFrontWall1->Draw();
	g_pFrontWall2->Draw();
	g_pBackWall1->Draw();
	g_pBackWall2->Draw();
	for (int i = 0; i < 9; i++) {
		g_pLeftRoomWalls[i]->Draw();
		g_pRightRoomWalls[i]->Draw();
	}
	g_pBigDoorUpperWall->Draw();
	g_pBigDoorBottomWall->Draw();
	g_pBalconyWall->Draw();

	//階梯
	glActiveTexture(GL_TEXTURE0);					// diffuse map
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[3]);
	glActiveTexture(GL_TEXTURE2);					// normal map
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[4]);
	g_pStairs->Draw();

	//DOORS
	glActiveTexture(GL_TEXTURE0);					// select active texture 0
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[9]);	// 與 Diffuse Map 結合
	glActiveTexture(GL_TEXTURE2);					// normal map
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[10]);
	for (int i = 0; i < 4; i++) g_pDoors[i]->Draw();

	//大門
	glActiveTexture(GL_TEXTURE0);					// diffuse map
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[11]);
	glActiveTexture(GL_TEXTURE2);					// normal map
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[12]);
	g_pBigDoor->Draw();

	// 槍
	glActiveTexture(GL_TEXTURE0);					// diffuse map
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[15]);
	glActiveTexture(GL_TEXTURE2);					// normal map
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[16]);
	g_pGun->Draw();

	//-----------------------------------

	glBindTexture(GL_TEXTURE_2D, 0);
	g_pLight->Draw();

	//-----------------------------------

	glActiveTexture(GL_TEXTURE0);							// select active texture 0
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[2]);			// 與 Diffuse Map 結合
	glActiveTexture(GL_TEXTURE1);							// select active texture 1
	glBindTexture(GL_TEXTURE_CUBE_MAP, g_uiSphereCubeMap);	// 與 Light Map 結合
	g_pSphere->Draw();
	for (int i = 0; i < 3; i++) g_pFences[i]->Draw();		//圍欄

	glDepthMask(GL_FALSE);	//---------------------------------------  關閉深度測試
	g_pSkyBox->Draw();	//天空

	//----------透明度物件--------
	for (int i = 0; i < 4; i++) g_pDoorUpperWalls[i]->Draw();	//門上玻璃

	glActiveTexture(GL_TEXTURE1); // select active texture 1
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[5]); // 與 Light Map 結合
	g_pGemSweet->Draw();		//GEMS

	glActiveTexture(GL_TEXTURE1); // select active texture 1
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[6]); // 與 Light Map 結合
	g_pGemToy->Draw();

	glActiveTexture(GL_TEXTURE1); // select active texture 1
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[7]); // 與 Light Map 結合
	g_pGemGarden->Draw();

	glActiveTexture(GL_TEXTURE1); // select active texture 1
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[8]); // 與 Light Map 結合
	g_pDiamond->Draw();

	glActiveTexture(GL_TEXTURE0);							// 火焰
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[13]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[14]); // Light Map
	g_pFire->Draw();


	glDisable(GL_BLEND);	// 關閉 Blending
	glDepthMask(GL_TRUE);	// --------------------------------------  開啟對 Z-Buffer 的寫入操作


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
	g_Light_main.position.x = g_fLightRadius * cosf(g_fLightTheta);
	g_Light_main.position.z = g_fLightRadius * sinf(g_fLightTheta);
	mxT = Translate(g_Light_main.position);
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

	// for billboard
	g_CameraPos = vec4(g_eye.x, 0, g_eye.z, 0);	// 鏡頭xz位置
	g_ViewDir = g_BillboardPos - g_CameraPos;	// 計算鏡頭方向
	g_ViewDir = normalize(g_ViewDir);
	if (g_CameraPos.x < 0) g_fBillboardTheta = -180.f + 60.f * acos(dot(g_BillboardNormalDir, g_ViewDir));	// billboard 旋轉角
	else g_fBillboardTheta = 180.f - 60.f * acos(dot(g_BillboardNormalDir, g_ViewDir));
	g_pFire->SetTRSMatrix(Translate(0.f, 5.f, -3.f) * RotateY(g_fBillboardTheta) * RotateX(90.f) * Scale(3.f, 3.f, 3.f));

	// for gun
	g_pGun->SetTRSMatrix(	Translate(g_eye) * RotateY(g_fPhi*60.f) * RotateX((g_fTheta + 1.5f)*60.f) *						//玩家位置/視角
							Translate(0.5f, -1.4f, -2.8f) * RotateY(5.f) * RotateX(5.f) * Scale(0.3f, 0.3f, 0.3f));			//槍枝位置/轉角/大小

	//---------------------------------------------------------------

	if( g_bAutoRotating ) { // Part 2 : 重新計算 Light 的位置
		UpdateLightPosition(delta);
	}

	//---------------------------------------------------------------

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
		for (int i = 0; i < 4; i++) {
			g_pDoors[i]->SetViewMatrix(mvx);
			g_pDoorUpperWalls[i]->SetViewMatrix(mvx);
		}
		for (int i = 0; i < 9; i++) {
			g_pLeftRoomWalls[i]->SetViewMatrix(mvx);
			g_pRightRoomWalls[i]->SetViewMatrix(mvx);
		}
		g_pBigDoorUpperWall->SetViewMatrix(mvx);
		g_pBigDoorBottomWall->SetViewMatrix(mvx);
		g_pBalconyWall->SetViewMatrix(mvx);

		g_pSkyBox->SetViewMatrix(mvx);
		g_pSkyBox->SetViewPosition(camera->getViewPosition());
		g_pSphere->SetViewMatrix(mvx);
		g_pSphere->SetViewPosition(camera->getViewPosition());
		g_pLight->SetViewMatrix(mvx);

		g_pStairs->SetViewMatrix(mvx);
		g_pBigDoor->SetViewMatrix(mvx);
		for (int i = 0; i < 3; i++) g_pFences[i]->SetViewMatrix(mvx);
		g_pFire->SetViewMatrix(mvx);

		g_pGemSweet->SetViewMatrix(mvx);
		g_pGemToy->SetViewMatrix(mvx);
		g_pGemGarden->SetViewMatrix(mvx);
		g_pDiamond->SetViewMatrix(mvx);
		g_pGun->SetViewMatrix(mvx);
	}

	// 如果需要重新計算時，在這邊計算每一個物件的顏色
	g_pFloor->Update(delta, g_Light_main, g_Light_out_f);			// WALLS
	g_pCeiling->Update(delta, g_Light_main, g_Light_out_f);
	g_pLeftWall1->Update(delta, g_Light_main, g_Light_left1);
	g_pLeftWall2->Update(delta, g_Light_main, g_Light_left2);
	g_pRightWall1->Update(delta, g_Light_main, g_Light_right1);
	g_pRightWall2->Update(delta, g_Light_main, g_Light_right2);
	g_pFrontWall1->Update(delta, g_Light_main, g_Light_out_f);
	g_pFrontWall2->Update(delta, g_Light_main, g_Light_out_f);
	g_pBackWall1->Update(delta, g_Light_main, g_Light_out_b);
	g_pBackWall2->Update(delta, g_Light_main, g_Light_out_b);
	for (int i = 0; i < 4; i++) g_pDoorUpperWalls[i]->Update(delta, g_Light_main);

	g_pDoors[0]->Update(delta, g_Light_main, g_Light_left1);
	g_pDoors[1]->Update(delta, g_Light_main, g_Light_left2);
	g_pDoors[2]->Update(delta, g_Light_main, g_Light_right1);
	g_pDoors[3]->Update(delta, g_Light_main, g_Light_right2);

	// ROOM WALLS
	g_pLeftRoomWalls[0]->Update(delta, g_Light_main, g_Light_out_l, g_Light_left1);
	g_pRightRoomWalls[0]->Update(delta, g_Light_main, g_Light_out_r, g_Light_right1);
	g_pLeftRoomWalls[1]->Update(delta, g_Light_main, g_Light_out_l, g_Light_left2);
	g_pRightRoomWalls[1]->Update(delta, g_Light_main, g_Light_out_r, g_Light_right2);
	g_pLeftRoomWalls[2]->Update(delta, g_Light_main, g_Light_out_f, g_Light_left1);
	g_pRightRoomWalls[2]->Update(delta, g_Light_main, g_Light_out_f, g_Light_right1);
	g_pLeftRoomWalls[3]->Update(delta, g_Light_main, g_Light_left1, g_Light_left2);
	g_pRightRoomWalls[3]->Update(delta, g_Light_main, g_Light_right2, g_Light_right1);
	g_pLeftRoomWalls[4]->Update(delta, g_Light_main, g_Light_out_b, g_Light_left2);
	g_pRightRoomWalls[4]->Update(delta, g_Light_main, g_Light_out_b, g_Light_right2);
	g_pLeftRoomWalls[5]->Update(delta, g_Light_main, g_Light_left1);
	g_pRightRoomWalls[5]->Update(delta, g_Light_main, g_Light_right1);
	g_pLeftRoomWalls[6]->Update(delta, g_Light_main, g_Light_left1);
	g_pRightRoomWalls[6]->Update(delta, g_Light_main, g_Light_right1);
	g_pLeftRoomWalls[7]->Update(delta, g_Light_main, g_Light_left2);
	g_pRightRoomWalls[7]->Update(delta, g_Light_main, g_Light_right2);
	g_pLeftRoomWalls[8]->Update(delta, g_Light_main, g_Light_left2);
	g_pRightRoomWalls[8]->Update(delta, g_Light_main, g_Light_right2);

	g_pSkyBox->Update(delta);
	g_pSphere->Update(delta, g_Light_out_f);
	g_pLight->Update(delta);

	g_pStairs->Update(delta, g_Light_main, g_Light_out_f);
	g_pBigDoor->Update(delta, g_Light_main, g_Light_out_f);
	g_pBigDoorUpperWall->Update(delta, g_Light_main, g_Light_out_f);
	g_pBigDoorBottomWall->Update(delta, g_Light_main, g_Light_out_f);
	g_pBalconyWall->Update(delta, g_Light_main, g_Light_out_f);
	for (int i = 0; i < 3; i++) g_pFences[i]->Update(delta, g_Light_main, g_Light_out_f);

	g_pFire->Update(delta);

	g_pGemSweet->Update(delta, g_Light_main, g_Light_left1);		// gems
	g_pGemToy->Update(delta, g_Light_main, g_Light_left2);
	g_pGemGarden->Update(delta, g_Light_main, g_Light_right1);
	g_pDiamond->Update(delta, g_Light_main, g_Light_right2);

	g_pGun->Update(delta, g_Light_main);		// 槍

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
		//if (g_fCameraMoveX <= WALKING_SPACE && g_fCameraMoveX >= -WALKING_SPACE && g_fCameraMoveZ <= WALKING_SPACE && g_fCameraMoveZ >= -WALKING_SPACE)  
		//{	
			g_fCameraMoveX += (g_matMoveDir._m[0][3] * 0.2f);	//限制空間
			g_fCameraMoveZ += (g_matMoveDir._m[2][3] * 0.2f);
		//}
		//else {	// 修正卡牆
		//	if (g_fCameraMoveX > WALKING_SPACE) g_fCameraMoveX = WALKING_SPACE;
		//	else if (g_fCameraMoveX < -WALKING_SPACE) g_fCameraMoveX = -WALKING_SPACE;
		//	if (g_fCameraMoveZ > WALKING_SPACE) g_fCameraMoveZ = WALKING_SPACE;
		//	else if (g_fCameraMoveZ < -WALKING_SPACE) g_fCameraMoveZ = -WALKING_SPACE;
		//}
		break;
	case 'S':
	case 's':
		g_MoveDir = vec4(g_fRadius*sin(g_fTheta)*sin(g_fPhi), 0.f, g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.f);
		g_MoveDir = normalize(g_MoveDir);
		g_matMoveDir = Translate(g_MoveDir.x, 0.f, g_MoveDir.z);
		//if (g_fCameraMoveX <= WALKING_SPACE && g_fCameraMoveX >= -WALKING_SPACE && g_fCameraMoveZ <= WALKING_SPACE && g_fCameraMoveZ >= -WALKING_SPACE) 
		//{	
			g_fCameraMoveX -= (g_matMoveDir._m[0][3] * 0.2f);	//限制空間
			g_fCameraMoveZ -= (g_matMoveDir._m[2][3] * 0.2f);
		//}
		//else {	// 修正卡牆
		//	if (g_fCameraMoveX > WALKING_SPACE) g_fCameraMoveX = WALKING_SPACE;
		//	else if (g_fCameraMoveX < -WALKING_SPACE) g_fCameraMoveX = -WALKING_SPACE;
		//	if (g_fCameraMoveZ > WALKING_SPACE) g_fCameraMoveZ = WALKING_SPACE;
		//	else if (g_fCameraMoveZ < -WALKING_SPACE) g_fCameraMoveZ = -WALKING_SPACE;
		//}
		break;
	case 'A':
	case 'a':
		g_MoveDir = vec4(g_fRadius*sin(g_fTheta)*sin(g_fPhi), 0.f, g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.f);
		g_MoveDir = normalize(g_MoveDir);
		g_matMoveDir = RotateY(90.f) * Translate(g_MoveDir.x, 0.f, g_MoveDir.z);
		//if (g_fCameraMoveX <= WALKING_SPACE && g_fCameraMoveX >= -WALKING_SPACE && g_fCameraMoveZ <= WALKING_SPACE && g_fCameraMoveZ >= -WALKING_SPACE) 
		//{	
			g_fCameraMoveX += (g_matMoveDir._m[0][3] * 0.2f);	//限制空間
			g_fCameraMoveZ += (g_matMoveDir._m[2][3] * 0.2f);
		//}
		//else {	// 修正卡牆
		//	if (g_fCameraMoveX > WALKING_SPACE) g_fCameraMoveX = WALKING_SPACE;
		//	else if (g_fCameraMoveX < -WALKING_SPACE) g_fCameraMoveX = -WALKING_SPACE;
		//	if (g_fCameraMoveZ > WALKING_SPACE) g_fCameraMoveZ = WALKING_SPACE;
		//	else if (g_fCameraMoveZ < -WALKING_SPACE) g_fCameraMoveZ = -WALKING_SPACE;
		//}
		break;
	case 'D':
	case 'd':
		g_MoveDir = vec4(g_fRadius*sin(g_fTheta)*sin(g_fPhi), 0.f, g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.f);
		g_MoveDir = normalize(g_MoveDir);
		g_matMoveDir = RotateY(90.f) * Translate(g_MoveDir.x, 0.f, g_MoveDir.z);
		//if (g_fCameraMoveX <= WALKING_SPACE && g_fCameraMoveX >= -WALKING_SPACE && g_fCameraMoveZ <= WALKING_SPACE && g_fCameraMoveZ >= -WALKING_SPACE) 
		//{	
			g_fCameraMoveX -= (g_matMoveDir._m[0][3] * 0.2f);	//限制空間
			g_fCameraMoveZ -= (g_matMoveDir._m[2][3] * 0.2f);
		//}
		//else {	// 修正卡牆
		//	if (g_fCameraMoveX > WALKING_SPACE) g_fCameraMoveX = WALKING_SPACE;
		//	else if (g_fCameraMoveX < -WALKING_SPACE) g_fCameraMoveX = -WALKING_SPACE;
		//	if (g_fCameraMoveZ > WALKING_SPACE) g_fCameraMoveZ = WALKING_SPACE;
		//	else if (g_fCameraMoveZ < -WALKING_SPACE) g_fCameraMoveZ = -WALKING_SPACE;
		//}
		break;

		// --------- for light color ---------
	case 82: // R key
		if( g_fLightR <= 0.95f ) g_fLightR += 0.05f;
		g_Light_main.diffuse.x = g_fLightR;
		g_pLight->SetColor(g_Light_main.diffuse);
		break;
	case 114: // r key
		if( g_fLightR >= 0.05f ) g_fLightR -= 0.05f;
		g_Light_main.diffuse.x = g_fLightR;
		g_pLight->SetColor(g_Light_main.diffuse);
		break;
	case 71: // G key
		if( g_fLightG <= 0.95f ) g_fLightG += 0.05f;
		g_Light_main.diffuse.y = g_fLightG;
		g_pLight->SetColor(g_Light_main.diffuse);
		break;
	case 103: // g key
		if( g_fLightG >= 0.05f ) g_fLightG -= 0.05f;
		g_Light_main.diffuse.y = g_fLightG;
		g_pLight->SetColor(g_Light_main.diffuse);
		break;
	case 66: // B key
		if( g_fLightB <= 0.95f ) g_fLightB += 0.05f;
		g_Light_main.diffuse.z = g_fLightB;
		g_pLight->SetColor(g_Light_main.diffuse);
		break;
	case 98: // b key
		if( g_fLightB >= 0.05f ) g_fLightB -= 0.05f;
		g_Light_main.diffuse.z = g_fLightB;
		g_pLight->SetColor(g_Light_main.diffuse);
		break;
//----------------------------------------------------------------------------
    case 033:
		glutIdleFunc( NULL );
		delete g_pFloor, g_pCeiling;
		delete g_pLeftWall1, g_pLeftWall2, g_pRightWall1, g_pRightWall2;
		delete g_pFrontWall1, g_pFrontWall2, g_pBackWall1 ,g_pBackWall2;
		for (int i = 0; i < 4; i++) {
			delete g_pDoors[i];
			delete g_pDoorUpperWalls[i];
		}
		for (int i = 0; i < 9; i++) {
			delete g_pLeftRoomWalls[i];
			delete g_pRightRoomWalls[i];
		}
		delete g_pBigDoorUpperWall, g_pBigDoorBottomWall;
		delete g_pBalconyWall;
		
		delete g_pSkyBox, g_pSphere;
		delete g_pLight;

		delete g_pStairs;
		delete g_pBigDoor;
		for (int i = 0; i < 3; i++) delete g_pFences[i];
		delete g_pFire;

		delete g_pGemSweet, g_pGemToy, g_pGemGarden, g_pDiamond;
		delete g_pGun;

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