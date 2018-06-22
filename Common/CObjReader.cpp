#include "CObjReader.h"
// Example 4 開始
// 面同 Example 3 朝上(Y軸)
// 每一個 Vertex 增加 Normal ，改成繼曾自 CShape，一併處理相關的設定需求


CObjReader::CObjReader(char *objfile)
{
	FILE *pfile;
	char pLineHead[20];
	int face[3][3];					//讀取用
	int ivec, inormal, iuv, ifaces;	//點、法向量、貼圖、面數紀錄
	ifaces = ivec = inormal = iuv = 0;

	if ((pfile = fopen(objfile, "r")) == NULL) {
		printf("obj file can't open."); system("pause");
	}
	while (!feof(pfile)) { //是否到檔案尾									//--------第一次讀檔，紀錄欲開空間大小--------//
		fscanf(pfile, "%s", pLineHead);						//讀取字串
		if (strcmp(pLineHead, "v") == 0) ivec++;			//讀取point
		else if (strcmp(pLineHead, "vn") == 0) inormal++;	//讀取normal
		else if (strcmp(pLineHead, "vt") == 0) iuv++;		//讀取texture
		else if (strcmp(pLineHead, "f") == 0) ifaces++;		//讀取face
	}

	m_iNumVtx = ifaces * 3;				//點數
	m_pPoints = NULL; m_pNormals = NULL;
	m_pTex1 = NULL; m_pTex2 = NULL; m_pTex3 = NULL;

	m_pPoints = new vec4[m_iNumVtx];	//使用點
	m_pNormals = new vec3[m_iNumVtx];	//Normal
	m_pColors = new vec4[m_iNumVtx];	//顏色
	m_pTex1 = new vec2[m_iNumVtx];		//diffuse map
#if MULTITEXTURE >= LIGHT_MAP
	m_pTex2 = new vec2[m_iNumVtx];		// 產生 light map 所需的貼圖座標
#endif
#if MULTITEXTURE >= NORMAL_MAP
	m_pTex3 = new vec2[m_iNumVtx];		// 產生 normal map 所需的貼圖座標
	m_pTangentV = new vec3[m_iNumVtx];	// 儲存 Tangent Space Normal Map 的 Tangent vector
#endif

	_vec4Points = new vec4[ivec];		//資料點 (vec4)
	_vec3Points_n = new vec3[inormal];	//法向量 (vec3)
	_vec3Points_uv = new vec3[iuv];		//貼圖點 (vec2)

	int pCount, vCount, nCount, uvCount;
	pCount = vCount = nCount = uvCount = 0;
	rewind(pfile);	//重新指到檔案頭

	while (!feof(pfile)) { //是否到檔案尾									//--------第二次讀檔，資料紀錄--------//
		fscanf(pfile, "%s", pLineHead);			 //讀取字串
		if (strcmp(pLineHead, "v") == 0) {		 //讀取vertex
			fscanf(pfile, "%f %f %f", &_vec4Points[vCount].x, &_vec4Points[vCount].y, &_vec4Points[vCount].z); //讀取3點
			_vec4Points[vCount].w = 1;
			vCount++;
		}
		else if (strcmp(pLineHead, "vn") == 0) { //讀取normal
			fscanf(pfile, "%f %f %f", &_vec3Points_n[nCount].x, &_vec3Points_n[nCount].y, &_vec3Points_n[nCount].z); //讀取3點
			nCount++;
		}
		else if (strcmp(pLineHead, "vt") == 0) { //讀取texture
			fscanf(pfile, "%f %f %f", &_vec3Points_uv[uvCount].x, &_vec3Points_uv[uvCount].y, &_vec3Points_uv[uvCount].z); //讀取3點
			uvCount++;
		}
		else if (strcmp(pLineHead, "f") == 0) {	//讀取face
			fscanf(pfile, "%d/%d/%d %d/%d/%d %d/%d/%d", &face[0][0], &face[0][1], &face[0][2],
														&face[1][0], &face[1][1], &face[1][2],
														&face[2][0], &face[2][1], &face[2][2]); //頂點v/貼圖vt(uv)/法線vn
			for (int i = 0; i < 3; i++) {
				m_pPoints[pCount + i] = _vec4Points[face[i][0] - 1];		// 讀取頂點
				m_pNormals[pCount + i] = _vec3Points_n[face[i][2] - 1];		// 讀取Normal
				m_pTex1[pCount + i].x = _vec3Points_uv[face[i][1] - 1].x;	// Texture x
				m_pTex1[pCount + i].y = _vec3Points_uv[face[i][1] - 1].y;	// Texture y
			}
			pCount += 3;
		}
	}
	fclose(pfile); //關閉檔案

	for (int i = 0; i < iuv; i++) {
		printf("(%f, %f)\n", _vec3Points_uv[i].x, _vec3Points_uv[i].y);
	}

	//-----------------------Multitexturing--------------------------
	for (int i = 0; i < m_iNumVtx; i++) {
#if MULTITEXTURE >= LIGHT_MAP
		m_pTex2[i] = m_pTex1[i];  // 產生 light map 所需的貼圖座標
#endif
#if MULTITEXTURE >= NORMAL_MAP
		m_pTex3[i] = m_pTex1[i];;	// 產生 normal map 所需的貼圖座標
#endif
	}
#if MULTITEXTURE >= NORMAL_MAP
		// 計算 tangent vector
	for (int i = 0; i < m_iNumVtx; i += 3) { // 三個 vertex 一組
		float dU1 = m_pTex3[i + 1].x - m_pTex3[i].x;
		float dV1 = m_pTex3[i + 1].y - m_pTex3[i].y;
		float dU2 = m_pTex3[i + 2].x - m_pTex3[i].x;
		float dV2 = m_pTex3[i + 2].y - m_pTex3[i].y;
		float f = 1.0f / (dU1 * dV2 - dU2*dV1);
		vec4 E1 = m_pPoints[i + 1] - m_pPoints[i];
		vec4 E2 = m_pPoints[i + 2] - m_pPoints[i];

		vec3 tangent;
		tangent.x = f*(dV2 * E1.x + E2.x * (-dV1));
		tangent.y = f*(dV2 * E1.y + E2.y * (-dV1));
		tangent.z = f*(dV2 * E1.z + E2.z * (-dV1));

		m_pTangentV[i] += tangent;
		m_pTangentV[i + 1] += tangent;
		m_pTangentV[i + 2] = tangent;
	}
	for (int i = 0; i < m_iNumVtx; i++)
		m_pTangentV[i] = normalize(m_pTangentV[i]);
#endif


	// Set shader's name
	SetShaderName("vsPerPixelLighting.glsl", "fsPerPixelLighting.glsl");

	// Create and initialize a buffer object 
	//CreateBufferObject();

	// 預設將所有的面都設定成灰色
	for (int i = 0; i < m_iNumVtx; i++) m_pColors[i] = vec4(-1.0f, -1.0f, -1.0f, 1.0f);

	// 設定材質
	SetMaterials(vec4(0), vec4(0.5f, 0.5f, 0.5f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));	//gray
	SetKaKdKsShini(0, 0.8f, 0.2f, 1);
}

CObjReader::~CObjReader()
{
	//歸還空間
	if (_vec4Points != NULL) delete[] _vec4Points;
	if (_vec3Points_n != NULL) delete[] _vec3Points_n;
	if (_vec3Points_uv != NULL) delete[] _vec3Points_uv;
}

void CObjReader::Draw()
{
	DrawingSetShader();
	glDrawArrays( GL_TRIANGLES, 0, m_iNumVtx);
}

void CObjReader::DrawW()
{
	DrawingWithoutSetShader();
	glDrawArrays( GL_TRIANGLES, 0, m_iNumVtx);
}

//----------------------FLAT SHADING--------------------------
void CObjReader::RenderWithFlatShading(const LightSource &lights)
{
	// 以每一個面的三個頂點計算其重心，以該重心作為顏色計算的點頂
	// 根據 Phong lighting model 計算相對應的顏色，並將顏色儲存到此三個點頂
	// 因為每一個平面的頂點的 Normal 都相同，所以此處並沒有計算此三個頂點的平均 Normal

	vec4 vCentroidP;
	for (int i = 0; i < m_iNumVtx; i += 3) {
		// 計算三角形的重心
		vCentroidP = (m_pPoints[i] + m_pPoints[i + 1] + m_pPoints[i + 2]) / 3.0f;
		m_pColors[i] = m_pColors[i + 1] = m_pColors[i + 2] = PhongReflectionModel(vCentroidP, m_pNormals[i], lights);
	}
	glBindBuffer(GL_ARRAY_BUFFER, m_uiBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4)*m_iNumVtx + sizeof(vec3)*m_iNumVtx, sizeof(vec4)*m_iNumVtx, m_pColors); // vertcies' Color
}

//----------------------GROUND SHADING--------------------------
void CObjReader::RenderWithGouraudShading(const LightSource &Lights)
{
	for (int i = 0; i < m_iNumVtx; i++) {
		m_pColors[i] = PhongReflectionModel(m_pPoints[i], m_pNormals[i], Lights);
	}
	glBindBuffer(GL_ARRAY_BUFFER, m_uiBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4)*m_iNumVtx + sizeof(vec3)*m_iNumVtx, sizeof(vec4)*m_iNumVtx, m_pColors); // vertcies' Color
}

//----------------------UPDATE--------------------------
// 此處所給的 vLightPos 必須是世界座標的確定絕對位置
void CObjReader::Update(float dt, point4 vLightPos, color4 vLightI)
{
#ifdef LIGHTING_WITHCPU
	if (m_bViewUpdated || m_bTRSUpdated) { // Model View 的相關矩陣內容有更動
		m_mxMVFinal = m_mxView * m_mxTRS;
		m_mxMV3X3Final = mat3(
			m_mxMVFinal._m[0].x, m_mxMVFinal._m[1].x, m_mxMVFinal._m[2].x,
			m_mxMVFinal._m[0].y, m_mxMVFinal._m[1].y, m_mxMVFinal._m[2].y,
			m_mxMVFinal._m[0].z, m_mxMVFinal._m[1].z, m_mxMVFinal._m[2].z);

#ifdef GENERAL_CASE
		m_mxITMV = InverseTransposeMatrix(m_mxMVFinal);
#endif

		m_bViewUpdated = m_bTRSUpdated = false;
	}
	if (m_iMode == FLAT_SHADING) RenderWithFlatShading(vLightPos, vLightI);
	else RenderWithGouraudShading(vLightPos, vLightI);

#else // Lighting With GPU
	if (m_bViewUpdated || m_bTRSUpdated) {
		m_mxMVFinal = m_mxView * m_mxTRS;
		m_bViewUpdated = m_bTRSUpdated = false;
	}
	m_vLightInView = m_mxView * vLightPos;		// 將 Light 轉換到鏡頭座標再傳入
												// 算出 AmbientProduct DiffuseProduct 與 SpecularProduct 的內容
	m_AmbientProduct = m_Material.ka * m_Material.ambient  * vLightI;
	m_AmbientProduct.w = m_Material.ambient.w;
	m_DiffuseProduct = m_Material.kd * m_Material.diffuse  * vLightI;
	m_DiffuseProduct.w = m_Material.diffuse.w;
	// 保留原始的 alpha 值，不受光源與其他參數的干擾
	m_SpecularProduct = m_Material.ks * m_Material.specular * vLightI;
	m_SpecularProduct.w = m_Material.specular.w;
#endif
}


void CObjReader::Update(float dt, const LightSource &Lights)	//一個光源
{
#ifdef LIGHTING_WITHCPU
	if (m_bViewUpdated || m_bTRSUpdated) { // Model View 的相關矩陣內容有更動
		m_mxMVFinal = m_mxView * m_mxTRS;
		m_mxMV3X3Final = mat3(
			m_mxMVFinal._m[0].x, m_mxMVFinal._m[1].x, m_mxMVFinal._m[2].x,
			m_mxMVFinal._m[0].y, m_mxMVFinal._m[1].y, m_mxMVFinal._m[2].y,
			m_mxMVFinal._m[0].z, m_mxMVFinal._m[1].z, m_mxMVFinal._m[2].z);

#ifdef GENERAL_CASE
		m_mxITMV = InverseTransposeMatrix(m_mxMVFinal);
#endif

		m_bViewUpdated = m_bTRSUpdated = false;
	}
	if (m_iMode == FLAT_SHADING) RenderWithFlatShading(Lights);
	else RenderWithGouraudShading(Lights);

#else // Lighting With GPU
	if (m_bViewUpdated || m_bTRSUpdated) {
		m_mxMVFinal = m_mxView * m_mxTRS;
		m_bViewUpdated = m_bTRSUpdated = false;
	}
	//設定光源 & 更新的光源數量
	m_Light1 = Lights;
	UpdateMultiLight(1);

#endif
}

void CObjReader::Update(float dt, const LightSource &Lights, const LightSource &Lights2)	//兩個光源
{
 // Lighting With GPU
	if (m_bViewUpdated || m_bTRSUpdated) {
		m_mxMVFinal = m_mxView * m_mxTRS;
		m_bViewUpdated = m_bTRSUpdated = false;
	}
	//設定光源 & 更新的光源數量
	m_Light1 = Lights;	m_Light2 = Lights2;
	UpdateMultiLight(2);
}

void CObjReader::Update(float dt, const LightSource &Lights, const LightSource &Lights2, const LightSource &Lights3)	//三個光源
{
	// Lighting With GPU
	if (m_bViewUpdated || m_bTRSUpdated) {
		m_mxMVFinal = m_mxView * m_mxTRS;
		m_bViewUpdated = m_bTRSUpdated = false;
	}
	//設定光源 & 更新的光源數量
	m_Light1 = Lights;	m_Light2 = Lights2; m_Light3 = Lights3;
	UpdateMultiLight(3);
}

void CObjReader::Update(float dt, const LightSource &Lights, const LightSource &Lights2, const LightSource &Lights3, const LightSource &Lights4)	//四個光源
{
	// Lighting With GPU
	if (m_bViewUpdated || m_bTRSUpdated) {
		m_mxMVFinal = m_mxView * m_mxTRS;
		m_bViewUpdated = m_bTRSUpdated = false;
	}
	//設定光源 & 更新的光源數量
	m_Light1 = Lights;	m_Light2 = Lights2; m_Light3 = Lights3; m_Light4 = Lights4;
	UpdateMultiLight(4);
}

void CObjReader::Update(float dt)
{
	if (m_bViewUpdated || m_bTRSUpdated) { // Model View 的相關矩陣內容有更動
		m_mxMVFinal = m_mxView * m_mxTRS;
		m_mxITMV = InverseTransposeMatrix(m_mxMVFinal);
		m_bViewUpdated = m_bTRSUpdated = false;
	}
}