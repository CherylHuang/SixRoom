#include "CBullet.h"

CBullet::CBullet(const GLfloat fRadius, const int iSlices, const  int iStacks)
{
	GLfloat drho = (GLfloat)(3.141592653589) / (GLfloat)iStacks;
	GLfloat dtheta = 2.0f * (GLfloat)(3.141592653589) / (GLfloat)iSlices;
	GLfloat ds = 1.0f / (GLfloat)iSlices;
	GLfloat dt = 1.0f / (GLfloat)iStacks;
	GLfloat t = 1.0f;
	GLfloat s = 0.0f;
	GLint i, j;     // Looping variables  
	int idx = 0; // 纗 vertex 抖ま

	m_fRadius = fRadius;
	m_iSlices = iSlices;
	m_iStacks = iStacks;
	m_iNumVtx = iStacks*(2 * (iSlices + 1));

	m_pPoints = NULL; m_pNormals = NULL; m_pTex1 = NULL;

	m_pPoints = new vec4[m_iNumVtx];
	m_pNormals = new vec3[m_iNumVtx];
	m_pColors = new vec4[m_iNumVtx];
	m_pTex1 = new vec2[m_iNumVtx];

#if MULTITEXTURE >= LIGHT_MAP
	m_pTex2 = new vec2[m_iNumVtx];  // 玻ネ light map ┮惠禟瓜畒夹
#endif
#if MULTITEXTURE >= NORMAL_MAP
	m_pTex3 = new vec2[m_iNumVtx];	// 玻ネ normal map ┮惠禟瓜畒夹
	m_pTangentV = new vec3[m_iNumVtx];
#endif

	for (i = 0; i < iStacks; i++) {
		GLfloat rho = (GLfloat)i * drho;
		GLfloat srho = (GLfloat)(sin(rho));
		GLfloat crho = (GLfloat)(cos(rho));
		GLfloat srhodrho = (GLfloat)(sin(rho + drho));
		GLfloat crhodrho = (GLfloat)(cos(rho + drho));

		// Many sources of OpenGL sphere drawing code uses a triangle fan  
		// for the caps of the sphere. This however introduces texturing   
		// artifacts at the poles on some OpenGL implementations  
		s = 0.0f;
		for (j = 0; j <= iSlices; j++) {
			GLfloat theta = (j == iSlices) ? 0.0f : j * dtheta;
			GLfloat stheta = (GLfloat)(-sin(theta));
			GLfloat ctheta = (GLfloat)(cos(theta));

			GLfloat x = stheta * srho;
			GLfloat y = ctheta * srho;
			GLfloat z = crho;

			m_pPoints[idx].x = x * m_fRadius;
			m_pPoints[idx].y = y * m_fRadius;
			m_pPoints[idx].z = z * m_fRadius;
			m_pPoints[idx].w = 1;

			m_pNormals[idx].x = x;
			m_pNormals[idx].y = y;
			m_pNormals[idx].z = z;

			m_pTex1[idx].x = s;
			m_pTex1[idx].y = t; // 砞﹚禟瓜畒夹
			idx++;

			x = stheta * srhodrho;
			y = ctheta * srhodrho;
			z = crhodrho;

			m_pPoints[idx].x = x * m_fRadius;
			m_pPoints[idx].y = y * m_fRadius;
			m_pPoints[idx].z = z * m_fRadius;
			m_pPoints[idx].w = 1;

			m_pNormals[idx].x = x;
			m_pNormals[idx].y = y;
			m_pNormals[idx].z = z;

			m_pTex1[idx].x = s;
			m_pTex1[idx].y = t - dt; // 砞﹚禟瓜畒夹
			idx++;
			s += ds;
		}
		t -= dt;
	}

	// 箇砞盢┮Τ常砞﹚Θη︹
	for (int i = 0; i < m_iNumVtx; i++) m_pColors[i] = vec4(-1.0f, -1.0f, -1.0f, 1.0f);

#ifdef PERVERTEX_LIGHTING
	SetShaderName("vsPerVtxLighting.glsl", "fsPerVtxLighting.glsl");
#else
	SetShaderName("vsPerPixelLighting.glsl", "fsPerPixelLighting.glsl");
#endif 

	// Create and initialize a buffer object 盢场だ砞﹚簿 SetShader い
	// CreateBufferObject();

	// 砞﹚借
	SetMaterials(vec4(0), vec4(0.5f, 0.5f, 0.5f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	SetKaKdKsShini(0, 0.8f, 0.2f, 1);
}

void CBullet::RenderWithGouraudShading(const LightSource &lights)
{
	// Method 1 : 癸– Vertex 常璸衡肅︹
	for (int i = 0; i < m_iStacks; i++) {
		for (int j = 0; j < 2 * (m_iSlices + 1); j++) {
			m_pColors[i * 2 * (m_iSlices + 1) + j] = PhongReflectionModel(m_pPoints[i * 2 * (m_iSlices + 1) + j], m_pNormals[i * 2 * (m_iSlices + 1) + j], lights);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_uiBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4)*m_iNumVtx + sizeof(vec3)*m_iNumVtx, sizeof(vec4)*m_iNumVtx, m_pColors); // vertcies' Color
}


void CBullet::Update(float dt, const LightSource &Lights)	//方
{
#ifdef LIGHTING_WITHCPU
	if (m_bViewUpdated || m_bTRSUpdated) { // Model View 闽痻皚ず甧Τ笆
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
	//砞﹚方 & 穝方计秖
	m_Light1 = Lights;
	UpdateMultiLight(1);

#endif
}

void CBullet::Update(float dt, const LightSource &Lights, const LightSource &Lights2)	//ㄢ方
{
	// Lighting With GPU
	if (m_bViewUpdated || m_bTRSUpdated) {
		m_mxMVFinal = m_mxView * m_mxTRS;
		m_bViewUpdated = m_bTRSUpdated = false;
	}
	//砞﹚方 & 穝方计秖
	m_Light1 = Lights;	m_Light2 = Lights2;
	UpdateMultiLight(2);
}

void CBullet::Update(float dt, const LightSource &Lights, const LightSource &Lights2, const LightSource &Lights3)	//方
{
	// Lighting With GPU
	if (m_bViewUpdated || m_bTRSUpdated) {
		m_mxMVFinal = m_mxView * m_mxTRS;
		m_bViewUpdated = m_bTRSUpdated = false;
	}
	//砞﹚方 & 穝方计秖
	m_Light1 = Lights;	m_Light2 = Lights2; m_Light3 = Lights3;
	UpdateMultiLight(3);
}

void CBullet::Update(float dt, const LightSource &Lights, const LightSource &Lights2, const LightSource &Lights3, const LightSource &Lights4)	//方
{
	// Lighting With GPU
	if (m_bViewUpdated || m_bTRSUpdated) {
		m_mxMVFinal = m_mxView * m_mxTRS;
		m_bViewUpdated = m_bTRSUpdated = false;
	}
	//砞﹚方 & 穝方计秖
	m_Light1 = Lights;	m_Light2 = Lights2; m_Light3 = Lights3; m_Light4 = Lights4;
	UpdateMultiLight(4);
}

void CBullet::Update(float dt)
{
	if (m_bViewUpdated || m_bTRSUpdated) { // Model View 闽痻皚ず甧Τ笆
		m_mxMVFinal = m_mxView * m_mxTRS;
		m_mxITMV = InverseTransposeMatrix(m_mxMVFinal);
		m_bViewUpdated = m_bTRSUpdated = false;
	}
}

void CBullet::Draw()
{
	DrawingSetShader();
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // Change to wireframe mode
	for (int i = 0; i < m_iStacks; i++) {
		glDrawArrays(GL_TRIANGLE_STRIP, i*(2 * (m_iSlices + 1)), 2 * (m_iSlices + 1));
	}
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Return to solid mode
}

void CBullet::DrawW()
{
	DrawingWithoutSetShader();
	for (int i = 0; i < m_iStacks; i++) {
		glDrawArrays(GL_TRIANGLE_STRIP, i*(2 * (m_iSlices + 1)), 2 * (m_iSlices + 1));
	}
}

CBullet::~CBullet()
{

}

mat4& CBullet::GetTRSMatrix()
{
	return(m_mxTRS);
}

void onFrameMoveBullet(float delta)
{

}