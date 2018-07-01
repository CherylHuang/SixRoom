// Cubic mapping (Environment Mapping)
// 
// ����e���ǳƤu�@
// ���� CShape.h ���� #define PERVERTEX_LIGHTING�A�ϥ� PERPIXEL_LIGHTING �~�|���@��
// �]�w #define MULTITEXTURE  (DIFFUSE_MAP|LIGHT_MAP|NORMAL_MAP)
// �}�� #define CUBIC_MAP 1

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
#define WALKING_SPACE (FLOOR_SCALE/2 - 5.0f)	//�樫�d��


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
CObjReader		*g_pStairs, *g_pDoors[4];
CObjReader		*g_pFences;
CObjReader		*g_pGemSweet, *g_pGemToy, *g_pGemGarden, *g_pDiamond;

CSolidSphere	*g_pSphere, *g_pSkyBox;

// For View Point
GLfloat g_fRadius = 8.0;
GLfloat g_fTheta = 45.0f*DegreesToRadians;
GLfloat g_fPhi = 45.0f*DegreesToRadians;
GLfloat g_fCameraMoveX = 0.f;				// for camera movment
GLfloat g_fCameraMoveY = 10.0f;				// for camera movment
GLfloat g_fCameraMoveZ = 0.f;				// for camera movment
mat4	g_matMoveDir;		// ���Y���ʤ�V
point4  g_MoveDir;
point4  g_at;				// ���Y�[�ݤ�V
point4  g_eye;				// ���Y��m

//----------------------------------------------------------------------------
// Part 2 : for single light source
bool g_bAutoRotating = false;
float g_fElapsedTime = 0;
float g_fLightRadius = 6;
float g_fLightTheta = 0;

float g_fLightR = 0.85f;
float g_fLightG = 0.85f;
float g_fLightB = 0.85f;

LightSource g_Light_out_f = {									//�ǥ~�e����
	color4(0.95f, 0.95f, 0.95f, 1.0f),		// ambient 
	color4(0.95f, 0.95f, 0.95f, 1.0f),		// diffuse
	color4(0.95f, 0.95f, 0.95f, 1.0f),		// specular
	point4(0.0f, 10.0f, -50.0f, 1.0f),   // position
	point4(0.0f, 0.0f, 0.0f, 1.0f),   // halfVector
	vec3(0.0f, 0.0f, 0.0f),			  //spotDirection
	2.0f,	// spotExponent(parameter e); cos^(e)(phi) 
	90.0f,	// spotCutoff;	// (range: [0.0, 90.0], 180.0)  spot ���ө��d��
	0.707f,	// spotCosCutoff; // (range: [1.0,0.0],-1.0), �ө���V�P�Q�ө��I���������ר� cos ��, cut off ����
	1,	// constantAttenuation	(a + bd + cd^2)^-1 ���� a, d ��������Q�ө��I���Z��
	0,	// linearAttenuation	    (a + bd + cd^2)^-1 ���� b
	0		// quadraticAttenuation (a + bd + cd^2)^-1 ���� c
};

LightSource g_Light_out_r = {									//�ǥ~�k����
	color4(0.95f, 0.95f, 0.95f, 1.0f),		// ambient 
	color4(0.95f, 0.95f, 0.95f, 1.0f),		// diffuse
	color4(0.95f, 0.95f, 0.95f, 1.0f),		// specular
	point4(80.0f, 10.0f, 0.0f, 1.0f),   // position
	point4(0.0f, 0.0f, 0.0f, 1.0f),   // halfVector
	vec3(0.0f, 0.0f, 0.0f),			  //spotDirection
	2.0f,	// spotExponent(parameter e); cos^(e)(phi) 
	90.0f,	// spotCutoff;	// (range: [0.0, 90.0], 180.0)  spot ���ө��d��
	0.707f,	// spotCosCutoff; // (range: [1.0,0.0],-1.0), �ө���V�P�Q�ө��I���������ר� cos ��, cut off ����
	1,	// constantAttenuation	(a + bd + cd^2)^-1 ���� a, d ��������Q�ө��I���Z��
	0,	// linearAttenuation	    (a + bd + cd^2)^-1 ���� b
	0		// quadraticAttenuation (a + bd + cd^2)^-1 ���� c
};

LightSource g_Light_out_l = {									//�ǥ~������
	color4(0.95f, 0.95f, 0.95f, 1.0f),		// ambient 
	color4(0.95f, 0.95f, 0.95f, 1.0f),		// diffuse
	color4(0.95f, 0.95f, 0.95f, 1.0f),		// specular
	point4(-80.0f, 10.0f, 0.0f, 1.0f),   // position
	point4(0.0f, 0.0f, 0.0f, 1.0f),   // halfVector
	vec3(0.0f, 0.0f, 0.0f),			  //spotDirection
	2.0f,	// spotExponent(parameter e); cos^(e)(phi) 
	90.0f,	// spotCutoff;	// (range: [0.0, 90.0], 180.0)  spot ���ө��d��
	0.707f,	// spotCosCutoff; // (range: [1.0,0.0],-1.0), �ө���V�P�Q�ө��I���������ר� cos ��, cut off ����
	1,	// constantAttenuation	(a + bd + cd^2)^-1 ���� a, d ��������Q�ө��I���Z��
	0,	// linearAttenuation	    (a + bd + cd^2)^-1 ���� b
	0		// quadraticAttenuation (a + bd + cd^2)^-1 ���� c
};

LightSource g_Light_out_b = {									//�ǥ~�����
	color4(0.95f, 0.95f, 0.95f, 1.0f),		// ambient 
	color4(0.95f, 0.95f, 0.95f, 1.0f),		// diffuse
	color4(0.95f, 0.95f, 0.95f, 1.0f),		// specular
	point4(0.0f, 10.0f, 50.0f, 1.0f),   // position
	point4(0.0f, 0.0f, 0.0f, 1.0f),   // halfVector
	vec3(0.0f, 0.0f, 0.0f),			  //spotDirection
	2.0f,	// spotExponent(parameter e); cos^(e)(phi) 
	90.0f,	// spotCutoff;	// (range: [0.0, 90.0], 180.0)  spot ���ө��d��
	0.707f,	// spotCosCutoff; // (range: [1.0,0.0],-1.0), �ө���V�P�Q�ө��I���������ר� cos ��, cut off ����
	1,	// constantAttenuation	(a + bd + cd^2)^-1 ���� a, d ��������Q�ө��I���Z��
	0,	// linearAttenuation	    (a + bd + cd^2)^-1 ���� b
	0		// quadraticAttenuation (a + bd + cd^2)^-1 ���� c
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
	45.0f,	// spotCutoff;	// (range: [0.0, 90.0], 180.0)  spot ���ө��d��
	0.707f,	// spotCosCutoff; // (range: [1.0,0.0],-1.0), �ө���V�P�Q�ө��I���������ר� cos ��, cut off ����
	1,	// constantAttenuation	(a + bd + cd^2)^-1 ���� a, d ��������Q�ө��I���Z��
	0,	// linearAttenuation	    (a + bd + cd^2)^-1 ���� b
	0		// quadraticAttenuation (a + bd + cd^2)^-1 ���� c
};

//----------------------------------------------------------------------------

// Texture 
GLuint g_uiFTexID[30]; // �T�Ӫ�����O�����P���K��
int g_iTexWidth,  g_iTexHeight;
GLuint g_uiSphereCubeMap; // for Cubic Texture

//----------------------------------------------------------------------------
// �禡���쫬�ŧi
extern void IdleProcess();
void CreateWalls();

void init( void )
{
	mat4 mxT, mxS;
	vec4 vT;
	vec3 vS;
	// ���ͩһݤ� Model View �P Projection Matrix
	// ���ͩһݤ� Model View �P Projection Matrix
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
	g_uiFTexID[8] = texturepool->AddTexture("texture/LightMap/flower.png");				// diamond Light
	g_uiFTexID[9] = texturepool->AddTexture("texture/DiffuseMap/door.png");				// door Diffuse
	g_uiFTexID[10] = texturepool->AddTexture("texture/NormalMap/door_NRM.png");			// door Normal
	g_uiFTexID[11] = texturepool->AddTexture("texture/DiffuseMap/wood.png");			// big door Diffuse
	g_uiFTexID[12] = texturepool->AddTexture("texture/NormalMap/wood_NRM.png");			// big door Normal
	
	g_uiSphereCubeMap = CubeMap_load_SOIL();	// Cub Map �]�m


	// ------------------- ���ͪ��󪺹��� --------------------
	CreateWalls();	//�ж����c��

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
	// �]�w�K��
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
	vT.x = 0.0f; vT.y = 2.0f; vT.z = 0.0f;
	mxT = Translate(vT);
	mxT._m[0][0] = mxT._m[1][1] = mxT._m[2][2] = 2.0f;
	g_pSphere->SetTRSMatrix(mxT*RotateX(90.0f));
	g_pSphere->SetShadingMode(GOURAUD_SHADING);
	// �]�w�K��
	g_pSphere->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pSphere->SetKaKdKsShini(0, 0.8f, 0.5f, 1);
	g_pSphere->SetColor(vec4(0.9f, 0.9f, 0.9f, 1.0f));

	//----------------------------------------------------------------------

	g_pStairs = new CObjReader("obj/stairs_fix.obj");						//����
	g_pStairs->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);	//�K��
	g_pStairs->SetMaterials(vec4(0), vec4(0.95f, 0.95f, 0.95f, 1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f));	// materials
	g_pStairs->SetKaKdKsShini(0.15f, 0.95f, 0.5f, 5);
	g_pStairs->SetShader();
	vT.x = 0.0f; vT.y = 0.0f; vT.z = -20.3f;	//Location
	mxT = Translate(vT);
	vS.x = vS.y = vS.z = 0.3f;				//Scale
	mxS = Scale(vS);
	g_pStairs->SetTRSMatrix(mxT /** mxS*/);
	g_pStairs->SetShadingMode(GOURAUD_SHADING);

	g_pBigDoor = new CSolidCube;											// �j��
	g_pBigDoor->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
	g_pBigDoor->SetShader();
	vT.x = 0.f; vT.y = FLOOR_SCALE / 3.0f - 2.6f; vT.z = -FLOOR_SCALE / 2.0f;
	mxT = Translate(vT);
	mxS = Scale(FLOOR_SCALE / 6.0f, 0.5f, FLOOR_SCALE / 3.0f - 0.4f);
	g_pBigDoor->SetTRSMatrix(mxT * RotateX(90.0f) * mxS);
	g_pBigDoor->SetShadingMode(GOURAUD_SHADING);
	g_pBigDoor->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pBigDoor->SetKaKdKsShini(0.25f, 0.8f, 0.2f, 2);

	g_pFences = new CObjReader("obj/fence.obj");							// ����
	g_pFences->SetTextureLayer(DIFFUSE_MAP);
	g_pFences->SetShader();
	vT.x = 0.f; vT.y = 7.7f; vT.z = -FLOOR_SCALE / 2.0f - FLOOR_SCALE / 12.0f + 0.06f;
	mxT = Translate(vT);
	mxS = Scale(0.4f, 0.4f, 0.4f);
	g_pFences->SetTRSMatrix(mxT * RotateY(90.f) * mxS);
	g_pFences->SetShadingMode(GOURAUD_SHADING);
	g_pFences->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pFences->SetKaKdKsShini(0.25f, 0.8f, 0.2f, 2);

	//----------------------------------------------------------------------
	g_pGemSweet = new CObjReader("obj/gem_sweet.obj");			//������
	g_pGemSweet->SetTextureLayer(DIFFUSE_MAP | LIGHT_MAP);	//�K��
	g_pGemSweet->SetLightMapColor(vec4(1.0f, 0.3f, 0.3f, 0.5f));	//Light Map : �b�z������
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
	g_pGemToy = new CObjReader("obj/gem_toy.obj");				//�Ť���
	g_pGemToy->SetTextureLayer(DIFFUSE_MAP | LIGHT_MAP);	//�K��
	g_pGemToy->SetLightMapColor(vec4(0.0f, 1.0f, 1.0f, 0.5f));	//Light Map : �b�z���C��
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
	g_pGemGarden = new CObjReader("obj/gem_garden.obj");		//�����
	g_pGemGarden->SetTextureLayer(DIFFUSE_MAP | LIGHT_MAP);	//�K��
	g_pGemGarden->SetLightMapColor(vec4(1.0f, 1.0f, 0.0f, 0.3f));	//Light Map : �b�z������
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
	g_pDiamond = new CObjReader("obj/diamond.obj");				//�p��
	g_pDiamond->SetTextureLayer(DIFFUSE_MAP | LIGHT_MAP);	//�K��
	g_pDiamond->SetLightMapColor(vec4(0.95f, 0.5f, 0.f, 0.3f));	//Light Map : �b�z�����
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

	//------------------------------------------
	// �]�w �N�� Light �� WireSphere
	g_pLight = new CWireSphere(0.25f, 6, 3);
	g_pLight->SetLightingDisable();
	g_pLight->SetTextureLayer(NONE_MAP);	// �S���K��
	g_pLight->SetShader();
	mxT = Translate(g_Light_main.position);
	g_pLight->SetTRSMatrix(mxT);
	g_pLight->SetColor(g_Light_main.diffuse);


	// �]�����d�Ҥ��|�ʨ� Projection Matrix �ҥH�b�o�̳]�w�@���Y�i
	// �N���g�b OnFrameMove ���C���� Check
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

	g_pSkyBox->SetProjectionMatrix(mpx);
	g_pSphere->SetProjectionMatrix(mpx);
	g_pLight->SetProjectionMatrix(mpx);

	g_pStairs->SetProjectionMatrix(mpx);
	g_pBigDoor->SetProjectionMatrix(mpx);
	g_pFences->SetProjectionMatrix(mpx);

	g_pGemSweet->SetProjectionMatrix(mpx);
	g_pGemToy->SetProjectionMatrix(mpx);
	g_pGemGarden->SetProjectionMatrix(mpx);
	g_pDiamond->SetProjectionMatrix(mpx);
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
	g_pFloor->SetTiling(6, 6);					// �]�w�K��
	g_pFloor->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pFloor->SetKaKdKsShini(0, 0.8f, 0.5f, 1);

	g_pCeiling = new CQuad;													// Ceiling
	g_pCeiling->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
	g_pCeiling->SetShader();
	vT.x = 0.0f; vT.y = FLOOR_SCALE / 2.0f; vT.z = 0.0f;	// Location
	mxT = Translate(vT);
	g_pCeiling->SetTRSMatrix(mxT * RotateX(180.0f) * mxS);
	g_pCeiling->SetShadingMode(GOURAUD_SHADING);
	g_pCeiling->SetTiling(6, 6);					// �]�w�K��
	g_pCeiling->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pCeiling->SetKaKdKsShini(0, 0.8f, 0.5f, 1);

	g_pLeftWall1 = new CSolidCube;											// Left Wall-1
	g_pLeftWall1->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
	g_pLeftWall1->SetShader();
	vT.x = -FLOOR_SCALE / 2.0f; vT.y = FLOOR_SCALE / 4.0f; vT.z = -FLOOR_SCALE / 4.0f + FLOOR_SCALE / 12.0f;	//+ FLOOR_SCALE / 12.0f : ���تŶ�
	mxT = Translate(vT);
	mxS = Scale(FLOOR_SCALE / 3.0f, 0.5f, FLOOR_SCALE / 2.0f);
	g_pLeftWall1->SetTRSMatrix(mxT * RotateX(90.0f) * RotateZ(90.0f) * mxS);
	g_pLeftWall1->SetShadingMode(GOURAUD_SHADING);
	g_pLeftWall1->SetTiling(3, 3);					// �]�w�K��
	g_pLeftWall1->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pLeftWall1->SetKaKdKsShini(0.25f, 0.8f, 0.2f, 2);

	g_pLeftWall2 = new CSolidCube;											// Left Wall-2
	g_pLeftWall2->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
	g_pLeftWall2->SetShader();
	vT.x = -FLOOR_SCALE / 2.0f; vT.y = FLOOR_SCALE / 4.0f; vT.z = FLOOR_SCALE / 4.0f - FLOOR_SCALE / 12.0f;
	mxT = Translate(vT);
	g_pLeftWall2->SetTRSMatrix(mxT * RotateX(90.0f) * RotateZ(90.0f) * mxS);
	g_pLeftWall2->SetShadingMode(GOURAUD_SHADING);
	g_pLeftWall2->SetTiling(3, 3);					// �]�w�K��
	g_pLeftWall2->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pLeftWall2->SetKaKdKsShini(0.25f, 0.8f, 0.2f, 2);

	g_pRightWall1 = new CSolidCube;											// Right Wall-1
	g_pRightWall1->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
	g_pRightWall1->SetShader();
	vT.x = FLOOR_SCALE / 2.0f; vT.y = FLOOR_SCALE / 4.0f; vT.z = -FLOOR_SCALE / 4.0f + FLOOR_SCALE / 12.0f;
	mxT = Translate(vT);
	g_pRightWall1->SetTRSMatrix(mxT * RotateX(90.0f) * RotateZ(90.0f) * mxS);
	g_pRightWall1->SetShadingMode(GOURAUD_SHADING);
	g_pRightWall1->SetTiling(3, 3);					// �]�w�K��
	g_pRightWall1->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pRightWall1->SetKaKdKsShini(0.25f, 0.8f, 0.2f, 2);

	g_pRightWall2 = new CSolidCube;											// Right Wall-2
	g_pRightWall2->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
	g_pRightWall2->SetShader();
	vT.x = FLOOR_SCALE / 2.0f; vT.y = FLOOR_SCALE / 4.0f; vT.z = FLOOR_SCALE / 4.0f - FLOOR_SCALE / 12.0f;
	mxT = Translate(vT);
	g_pRightWall2->SetTRSMatrix(mxT * RotateX(90.0f) * RotateZ(90.0f) * mxS);
	g_pRightWall2->SetShadingMode(GOURAUD_SHADING);
	g_pRightWall2->SetTiling(3, 3);					// �]�w�K��
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
	g_pFrontWall1->SetTiling(3, 3);					// �]�w�K��
	g_pFrontWall1->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pFrontWall1->SetKaKdKsShini(0.25f, 0.8f, 0.2f, 2);

	g_pFrontWall2 = new CSolidCube;											// Front Wall-2
	g_pFrontWall2->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
	g_pFrontWall2->SetShader();
	vT.x = FLOOR_SCALE / 4.0f + FLOOR_SCALE / 24.0f; vT.y = FLOOR_SCALE / 4.0f; vT.z = -FLOOR_SCALE / 2.0f;
	mxT = Translate(vT);
	g_pFrontWall2->SetTRSMatrix(mxT * RotateX(90.0f) * mxS);
	g_pFrontWall2->SetShadingMode(GOURAUD_SHADING);
	g_pFrontWall2->SetTiling(3, 3);					// �]�w�K��
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
	g_pBackWall1->SetTiling(3, 3);					// �]�w�K��
	g_pBackWall1->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pBackWall1->SetKaKdKsShini(0.25f, 0.8f, 0.2f, 2);

	g_pBackWall2 = new CSolidCube;											// Back Wall-2
	g_pBackWall2->SetTextureLayer(DIFFUSE_MAP | NORMAL_MAP);
	g_pBackWall2->SetShader();
	vT.x = FLOOR_SCALE / 4.0f; vT.y = FLOOR_SCALE / 4.0f; vT.z = FLOOR_SCALE / 2.0f;
	mxT = Translate(vT);
	g_pBackWall2->SetTRSMatrix(mxT * RotateX(90.0f) * mxS);
	g_pBackWall2->SetShadingMode(GOURAUD_SHADING);
	g_pBackWall2->SetTiling(3, 3);					// �]�w�K��
	g_pBackWall2->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pBackWall2->SetKaKdKsShini(0.25f, 0.8f, 0.2f, 2);

	//--------------------------------------------------

	for (int i = 0; i < 4; i++) {
		g_pDoorUpperWalls[i] = new CSolidCube;											// Doors upper walls : ����
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
		g_pLeftRoomWalls[i]->SetTiling(3, 3);					// �]�w�K��
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
		g_pRightRoomWalls[i]->SetTiling(3, 3);					// �]�w�K��
		g_pRightRoomWalls[i]->SetMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
		g_pRightRoomWalls[i]->SetKaKdKsShini(0.25f, 0.8f, 0.2f, 2);
	}

	//---------------------------------------------

	g_pBigDoorUpperWall = new CSolidCube;											// �j���W�� �p��
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

	g_pBigDoorBottomWall = new CSolidCube;											// �j���U�� ����
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
	
	g_pBalconyWall = new CSolidCube;											// ���x�a�O
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

	glEnable(GL_BLEND);								// �]�w2D Texure Mapping ���@��
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//WALLS
	glActiveTexture(GL_TEXTURE0);					// select active texture 0
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[0]);	// �P Diffuse Map ���X
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

	//����
	glActiveTexture(GL_TEXTURE0);					// diffuse map
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[3]);
	glActiveTexture(GL_TEXTURE2);					// normal map
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[4]);
	g_pStairs->Draw();

	//DOORS
	glActiveTexture(GL_TEXTURE0);					// select active texture 0
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[9]);	// �P Diffuse Map ���X
	glActiveTexture(GL_TEXTURE2);					// normal map
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[10]);
	for (int i = 0; i < 4; i++) g_pDoors[i]->Draw();

	//�j��
	glActiveTexture(GL_TEXTURE0);					// diffuse map
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[11]);
	glActiveTexture(GL_TEXTURE2);					// normal map
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[12]);
	g_pBigDoor->Draw();

	//-----------------------------------

	glBindTexture(GL_TEXTURE_2D, 0);
	g_pLight->Draw();

	//-----------------------------------

	glActiveTexture(GL_TEXTURE0);							// select active texture 0
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[2]);			// �P Diffuse Map ���X
	glActiveTexture(GL_TEXTURE1);							// select active texture 1
	glBindTexture(GL_TEXTURE_CUBE_MAP, g_uiSphereCubeMap);	// �P Light Map ���X
	g_pSphere->Draw();
	g_pFences->Draw();		//����

	glDepthMask(GL_FALSE);	//---------------------------------------  �����`�״���
	g_pSkyBox->Draw();	//�Ѫ�

	//----------�z���ת���--------
	for (int i = 0; i < 4; i++) g_pDoorUpperWalls[i]->Draw();	//���W����

	glActiveTexture(GL_TEXTURE1); // select active texture 1
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[5]); // �P Light Map ���X
	g_pGemSweet->Draw();		//GEMS

	glActiveTexture(GL_TEXTURE1); // select active texture 1
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[6]); // �P Light Map ���X
	g_pGemToy->Draw();

	glActiveTexture(GL_TEXTURE1); // select active texture 1
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[7]); // �P Light Map ���X
	g_pGemGarden->Draw();

	glActiveTexture(GL_TEXTURE1); // select active texture 1
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[8]); // �P Light Map ���X
	g_pDiamond->Draw();

	glDisable(GL_BLEND);	// ���� Blending
	glDepthMask(GL_TRUE);	// --------------------------------------  �}�ҹ� Z-Buffer ���g�J�ާ@

	//-------------------------------------
	glutSwapBuffers();	// �洫 Frame Buffer
}

//----------------------------------------------------------------------------
// Part 2 : for single light source
void UpdateLightPosition(float dt)
{
	mat4 mxT;
	// �C��¶ Y �b�� 90 ��
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
	g_eye = vec4(g_fCameraMoveX, g_fCameraMoveY, g_fCameraMoveZ, 1.0f);	//�Ĥ@�H�ٵ���
	auto camera = CCamera::getInstance();
	camera->updateViewLookAt(g_eye, g_at);

	//---------------------------------------------------------------

	if( g_bAutoRotating ) { // Part 2 : ���s�p�� Light ����m
		UpdateLightPosition(delta);
	}

	mat4 mvx;	// view matrix & projection matrix
	bool bVDirty;	// view �P projection matrix �O�_�ݭn��s������
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
		g_pFences->SetViewMatrix(mvx);

		g_pGemSweet->SetViewMatrix(mvx);
		g_pGemToy->SetViewMatrix(mvx);
		g_pGemGarden->SetViewMatrix(mvx);
		g_pDiamond->SetViewMatrix(mvx);
	}

	// �p�G�ݭn���s�p��ɡA�b�o��p��C�@�Ӫ����C��
	g_pFloor->Update(delta, g_Light_main, g_Light_out_f);			// WALLS
	g_pCeiling->Update(delta, g_Light_main, g_Light_out_f);
	g_pLeftWall1->Update(delta, g_Light_main);
	g_pLeftWall2->Update(delta, g_Light_main);
	g_pRightWall1->Update(delta, g_Light_main);
	g_pRightWall2->Update(delta, g_Light_main);
	g_pFrontWall1->Update(delta, g_Light_main, g_Light_out_f);
	g_pFrontWall2->Update(delta, g_Light_main, g_Light_out_f);
	g_pBackWall1->Update(delta, g_Light_main, g_Light_out_b);
	g_pBackWall2->Update(delta, g_Light_main, g_Light_out_b);
	for (int i = 0; i < 4; i++) {
		g_pDoors[i]->Update(delta, g_Light_main);
		g_pDoorUpperWalls[i]->Update(delta, g_Light_main);
	}
	for (int i = 0; i < 9; i++) {
		if (i == 0 || i == 1) {
			g_pRightRoomWalls[i]->Update(delta, g_Light_main, g_Light_out_r);
			g_pLeftRoomWalls[i]->Update(delta, g_Light_main, g_Light_out_l);
		}
		else if (i == 2) {
			g_pLeftRoomWalls[i]->Update(delta, g_Light_main, g_Light_out_f);
			g_pRightRoomWalls[i]->Update(delta, g_Light_main, g_Light_out_f);
		}
		else if (i == 4) {
			g_pLeftRoomWalls[i]->Update(delta, g_Light_main, g_Light_out_b);
			g_pRightRoomWalls[i]->Update(delta, g_Light_main, g_Light_out_b);
		}
		else {
			g_pLeftRoomWalls[i]->Update(delta, g_Light_main);
			g_pRightRoomWalls[i]->Update(delta, g_Light_main);
		}
	}

	g_pSkyBox->Update(delta);
	g_pSphere->Update(delta, g_Light_main);
	g_pLight->Update(delta);

	g_pStairs->Update(delta, g_Light_main, g_Light_out_f);
	g_pBigDoor->Update(delta, g_Light_main, g_Light_out_f);
	g_pBigDoorUpperWall->Update(delta, g_Light_main, g_Light_out_f);
	g_pBigDoorBottomWall->Update(delta, g_Light_main, g_Light_out_f);
	g_pBalconyWall->Update(delta, g_Light_main, g_Light_out_f);
	g_pFences->Update(delta, g_Light_main, g_Light_out_f);

	g_pGemSweet->Update(delta, g_Light_main);		// gems
	g_pGemToy->Update(delta, g_Light_main);
	g_pGemGarden->Update(delta, g_Light_main);
	g_pDiamond->Update(delta, g_Light_main);

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
			g_fCameraMoveX += (g_matMoveDir._m[0][3] * 0.2f);	//����Ŷ�
			g_fCameraMoveZ += (g_matMoveDir._m[2][3] * 0.2f);
		//}
		//else {	// �ץ��d��
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
			g_fCameraMoveX -= (g_matMoveDir._m[0][3] * 0.2f);	//����Ŷ�
			g_fCameraMoveZ -= (g_matMoveDir._m[2][3] * 0.2f);
		//}
		//else {	// �ץ��d��
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
			g_fCameraMoveX += (g_matMoveDir._m[0][3] * 0.2f);	//����Ŷ�
			g_fCameraMoveZ += (g_matMoveDir._m[2][3] * 0.2f);
		//}
		//else {	// �ץ��d��
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
			g_fCameraMoveX -= (g_matMoveDir._m[0][3] * 0.2f);	//����Ŷ�
			g_fCameraMoveZ -= (g_matMoveDir._m[2][3] * 0.2f);
		//}
		//else {	// �ץ��d��
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
		delete g_pFences;

		delete g_pGemSweet, g_pGemToy, g_pGemGarden, g_pDiamond;

		CCamera::getInstance()->destroyInstance();
		CTexturePool::getInstance()->destroyInstance();
        exit( EXIT_SUCCESS );
        break;
    }
}

//----------------------------------------------------------------------------
void Win_Mouse(int button, int state, int x, int y) {
	switch(button) {
		case GLUT_LEFT_BUTTON:   // �ثe���U���O�ƹ�����
			//if ( state == GLUT_DOWN ) ; 
			break;
		case GLUT_MIDDLE_BUTTON:  // �ثe���U���O�ƹ����� �A���� Y �b
			//if ( state == GLUT_DOWN ) ; 
			break;
		case GLUT_RIGHT_BUTTON:   // �ثe���U���O�ƹ��k��
			//if ( state == GLUT_DOWN ) ;
			break;
		default:
			break;
	} 
}
//----------------------------------------------------------------------------
void Win_SpecialKeyboard(int key, int x, int y) {

	switch(key) {
		case GLUT_KEY_UP:		// �ثe���U���O�V�W��V��
			g_MoveDir = vec4(g_fRadius*sin(g_fTheta)*sin(g_fPhi), 0.f, g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.f);
			g_MoveDir = normalize(g_MoveDir);
			g_matMoveDir = Translate(g_MoveDir.x, 0.f, g_MoveDir.z);
			if (g_fCameraMoveX <= WALKING_SPACE && g_fCameraMoveX >= -WALKING_SPACE && g_fCameraMoveZ <= WALKING_SPACE && g_fCameraMoveZ >= -WALKING_SPACE) {	//����Ŷ�
				g_fCameraMoveX += (g_matMoveDir._m[0][3] * 0.2f);
				g_fCameraMoveZ += (g_matMoveDir._m[2][3] * 0.2f);
			}
			else {	// �ץ��d��
				if (g_fCameraMoveX > WALKING_SPACE) g_fCameraMoveX = WALKING_SPACE;
				else if (g_fCameraMoveX < -WALKING_SPACE) g_fCameraMoveX = -WALKING_SPACE;
				if (g_fCameraMoveZ > WALKING_SPACE) g_fCameraMoveZ = WALKING_SPACE;
				else if (g_fCameraMoveZ < -WALKING_SPACE) g_fCameraMoveZ = -WALKING_SPACE;
			}
			break;
		case GLUT_KEY_DOWN:		// �ثe���U���O�V�U��V��
			g_MoveDir = vec4(g_fRadius*sin(g_fTheta)*sin(g_fPhi), 0.f, g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.f);
			g_MoveDir = normalize(g_MoveDir);
			g_matMoveDir = Translate(g_MoveDir.x, 0.f, g_MoveDir.z);
			if (g_fCameraMoveX <= WALKING_SPACE && g_fCameraMoveX >= -WALKING_SPACE && g_fCameraMoveZ <= WALKING_SPACE && g_fCameraMoveZ >= -WALKING_SPACE) {	//����Ŷ�
				g_fCameraMoveX -= (g_matMoveDir._m[0][3] * 0.2f);
				g_fCameraMoveZ -= (g_matMoveDir._m[2][3] * 0.2f);
			}
			else {	// �ץ��d��
				if (g_fCameraMoveX > WALKING_SPACE) g_fCameraMoveX = WALKING_SPACE;
				else if (g_fCameraMoveX < -WALKING_SPACE) g_fCameraMoveX = -WALKING_SPACE;
				if (g_fCameraMoveZ > WALKING_SPACE) g_fCameraMoveZ = WALKING_SPACE;
				else if (g_fCameraMoveZ < -WALKING_SPACE) g_fCameraMoveZ = -WALKING_SPACE;
			}
			break;
		case GLUT_KEY_LEFT:		// �ثe���U���O�V����V��
			g_MoveDir = vec4(g_fRadius*sin(g_fTheta)*sin(g_fPhi), 0.f, g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.f);
			g_MoveDir = normalize(g_MoveDir);
			g_matMoveDir = RotateY(90.f) * Translate(g_MoveDir.x, 0.f, g_MoveDir.z);
			if (g_fCameraMoveX <= WALKING_SPACE && g_fCameraMoveX >= -WALKING_SPACE && g_fCameraMoveZ <= WALKING_SPACE && g_fCameraMoveZ >= -WALKING_SPACE) {	//����Ŷ�
				g_fCameraMoveX += (g_matMoveDir._m[0][3] * 0.2f);
				g_fCameraMoveZ += (g_matMoveDir._m[2][3] * 0.2f);
			}
			else {	// �ץ��d��
				if (g_fCameraMoveX > WALKING_SPACE) g_fCameraMoveX = WALKING_SPACE;
				else if (g_fCameraMoveX < -WALKING_SPACE) g_fCameraMoveX = -WALKING_SPACE;
				if (g_fCameraMoveZ > WALKING_SPACE) g_fCameraMoveZ = WALKING_SPACE;
				else if (g_fCameraMoveZ < -WALKING_SPACE) g_fCameraMoveZ = -WALKING_SPACE;
			}
			break;
		case GLUT_KEY_RIGHT:	// �ثe���U���O�V�k��V��
			g_MoveDir = vec4(g_fRadius*sin(g_fTheta)*sin(g_fPhi), 0.f, g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.f);
			g_MoveDir = normalize(g_MoveDir);
			g_matMoveDir = RotateY(90.f) * Translate(g_MoveDir.x, 0.f, g_MoveDir.z);
			if (g_fCameraMoveX <= WALKING_SPACE && g_fCameraMoveX >= -WALKING_SPACE && g_fCameraMoveZ <= WALKING_SPACE && g_fCameraMoveZ >= -WALKING_SPACE) {	//����Ŷ�
				g_fCameraMoveX -= (g_matMoveDir._m[0][3] * 0.2f);
				g_fCameraMoveZ -= (g_matMoveDir._m[2][3] * 0.2f);
			}
			else {	// �ץ��d��
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
	g_fPhi = (float)M_PI*(x - HALF_SIZE) / (HALF_SIZE); // �ഫ�� g_fPhi ���� -PI �� PI ���� (-180 ~ 180 ����)
	g_fTheta = (float)-M_PI*(float)y / SCREEN_SIZE;
}

// The motion callback for a window is called when the mouse moves within the window while one or more mouse buttons are pressed.
void Win_MouseMotion(int x, int y) {
	g_fPhi = (float)M_PI*(x - HALF_SIZE) / (HALF_SIZE); // �ഫ�� g_fPhi ���� -PI �� PI ���� (-180 ~ 180 ����)
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
    glutKeyboardFunc( Win_Keyboard );	// �B�z ASCI ����p A�Ba�BESC ��...����
	glutSpecialFunc( Win_SpecialKeyboard);	// �B�z NON-ASCI ����p F1�BHome�B��V��...����
    glutDisplayFunc( GL_Display );
	glutReshapeFunc( GL_Reshape );
	glutIdleFunc( IdleProcess );
	
    glutMainLoop();
    return 0;
}