#include "CObjReader.h"
// Example 4 �}�l
// ���P Example 3 �¤W(Y�b)
// �C�@�� Vertex �W�[ Normal �A�令�~���� CShape�A�@�ֳB�z�������]�w�ݨD


CObjReader::CObjReader(char *objfile)
{
	FILE *pfile;
	char pLineHead[20];
	int face[3][3]; //Ū����
	int ivec, ifaces, inormal;	//�I�B���B�k�V�q�Ƭ���
	ifaces = ivec = inormal = 0;

	if ((pfile = fopen(objfile, "r")) == NULL) {
		printf("obj file can't open."); system("pause");
	}
	while (!feof(pfile)) { //�O�_���ɮק�
		fscanf(pfile, "%s", pLineHead); //Ū���r��
		if (strcmp(pLineHead, "v") == 0) ivec++; //Ū��point
		else if (strcmp(pLineHead, "vn") == 0) inormal++; //Ū��normal
		else if (strcmp(pLineHead, "f") == 0) ifaces++; //Ū��face
	}

	m_iNumVtx = ifaces * 3;	//�I��
	m_pPoints = NULL; m_pNormals = NULL; m_pTex1 = NULL;

	m_pPoints = new vec4[m_iNumVtx];	//�ϥ��I
	m_pNormals = new vec3[m_iNumVtx];	//Normal
	m_pColors = new vec4[m_iNumVtx];	//�C��

	_vec4Points = new vec4[ivec];		//����I (vec4)
	_vec3Points = new vec3[inormal];	//�k�V�q (vec3)

	int pCount, vCount, nCount;
	pCount = vCount = nCount = 0;
	rewind(pfile);	//���s�����ɮ��Y

	while (!feof(pfile)) { //�O�_���ɮק�
		fscanf(pfile, "%s", pLineHead); //Ū���r��
		if (strcmp(pLineHead, "v") == 0) { //Ū��vertex
			fscanf(pfile, "%f %f %f", &_vec4Points[vCount].x, &_vec4Points[vCount].y, &_vec4Points[vCount].z); //Ū��3�I
			_vec4Points[vCount].w = 1;
			vCount++;
		}
		else if (strcmp(pLineHead, "vn") == 0) { //Ū��normal
			fscanf(pfile, "%f %f %f", &_vec3Points[nCount].x, &_vec3Points[nCount].y, &_vec3Points[nCount].z); //Ū��3�I
			nCount++;
		}
		else if (strcmp(pLineHead, "f") == 0) { //Ū��face
			fscanf(pfile, "%d/%d/%d %d/%d/%d %d/%d/%d", &face[0][0], &face[0][1], &face[0][2],
														&face[1][0], &face[1][1], &face[1][2],
														&face[2][0], &face[2][1], &face[2][2]); //Ū��face
			for (int i = 0; i < 3; i++) {
				m_pPoints[pCount + i] = _vec4Points[face[i][0] - 1];	// Ū�����I
				m_pNormals[pCount + i] = _vec3Points[face[i][2] - 1];	// Ū��Normal
			}
			pCount += 3;
		}
	}
	fclose(pfile); //�����ɮ�

	// Set shader's name
	SetShaderName("vsPerPixelLighting.glsl", "fsPerPixelLighting.glsl");

	// Create and initialize a buffer object 
	//CreateBufferObject();

	// ��l�C�� : -1
	//m_fColor[0] = -1.0f; m_fColor[1] = -1.0f; m_fColor[2] = -1.0f; m_fColor[3] = 1;
	// �w�]�N�Ҧ��������]�w���Ǧ�
	for (int i = 0; i < m_iNumVtx; i++) m_pColors[i] = vec4(-1.0f, -1.0f, -1.0f, 1.0f);

	// �]�w����
	SetMaterials(vec4(0), vec4(0.5f, 0.5f, 0.5f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));	//gray
	SetKaKdKsShini(0, 0.8f, 0.2f, 1);
}

CObjReader::~CObjReader()
{
	//�k�٪Ŷ�
	if (_vec4Points != NULL) delete[] _vec4Points;
	if (_vec3Points != NULL) delete[] _vec3Points;
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
	// �H�C�@�ӭ����T�ӳ��I�p��䭫�ߡA�H�ӭ��ߧ@���C��p�⪺�I��
	// �ھ� Phong lighting model �p��۹������C��A�ñN�C���x�s�즹�T���I��
	// �]���C�@�ӥ��������I�� Normal ���ۦP�A�ҥH���B�èS���p�⦹�T�ӳ��I������ Normal

	vec4 vCentroidP;
	for (int i = 0; i < m_iNumVtx; i += 3) {
		// �p��T���Ϊ�����
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
// ���B�ҵ��� vLightPos �����O�@�ɮy�Ъ��T�w�����m
void CObjReader::Update(float dt, point4 vLightPos, color4 vLightI)
{
#ifdef LIGHTING_WITHCPU
	if (m_bViewUpdated || m_bTRSUpdated) { // Model View �������x�}���e�����
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
	m_vLightInView = m_mxView * vLightPos;		// �N Light �ഫ�����Y�y�ЦA�ǤJ
												// ��X AmbientProduct DiffuseProduct �P SpecularProduct �����e
	m_AmbientProduct = m_Material.ka * m_Material.ambient  * vLightI;
	m_AmbientProduct.w = m_Material.ambient.w;
	m_DiffuseProduct = m_Material.kd * m_Material.diffuse  * vLightI;
	m_DiffuseProduct.w = m_Material.diffuse.w;
	// �O�d��l�� alpha �ȡA���������P��L�Ѽƪ��z�Z
	m_SpecularProduct = m_Material.ks * m_Material.specular * vLightI;
	m_SpecularProduct.w = m_Material.specular.w;
#endif
}


void CObjReader::Update(float dt, const LightSource &Lights)	//�@�ӥ���
{
#ifdef LIGHTING_WITHCPU
	if (m_bViewUpdated || m_bTRSUpdated) { // Model View �������x�}���e�����
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
	//�]�w���� & ��s�������ƶq
	m_Light1 = Lights;
	UpdateMultiLight(1);

#endif
}

void CObjReader::Update(float dt, const LightSource &Lights, const LightSource &Lights2)	//��ӥ���
{
 // Lighting With GPU
	if (m_bViewUpdated || m_bTRSUpdated) {
		m_mxMVFinal = m_mxView * m_mxTRS;
		m_bViewUpdated = m_bTRSUpdated = false;
	}
	//�]�w���� & ��s�������ƶq
	m_Light1 = Lights;	m_Light2 = Lights2;
	UpdateMultiLight(2);
}

void CObjReader::Update(float dt, const LightSource &Lights, const LightSource &Lights2, const LightSource &Lights3)	//�T�ӥ���
{
	// Lighting With GPU
	if (m_bViewUpdated || m_bTRSUpdated) {
		m_mxMVFinal = m_mxView * m_mxTRS;
		m_bViewUpdated = m_bTRSUpdated = false;
	}
	//�]�w���� & ��s�������ƶq
	m_Light1 = Lights;	m_Light2 = Lights2; m_Light3 = Lights3;
	UpdateMultiLight(3);
}

void CObjReader::Update(float dt, const LightSource &Lights, const LightSource &Lights2, const LightSource &Lights3, const LightSource &Lights4)	//�|�ӥ���
{
	// Lighting With GPU
	if (m_bViewUpdated || m_bTRSUpdated) {
		m_mxMVFinal = m_mxView * m_mxTRS;
		m_bViewUpdated = m_bTRSUpdated = false;
	}
	//�]�w���� & ��s�������ƶq
	m_Light1 = Lights;	m_Light2 = Lights2; m_Light3 = Lights3; m_Light4 = Lights4;
	UpdateMultiLight(4);
}

void CObjReader::Update(float dt)
{
	if (m_bViewUpdated || m_bTRSUpdated) { // Model View �������x�}���e�����
		m_mxMVFinal = m_mxView * m_mxTRS;
		m_mxITMV = InverseTransposeMatrix(m_mxMVFinal);
		m_bViewUpdated = m_bTRSUpdated = false;
	}
}