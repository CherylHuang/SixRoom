#ifndef CQUAD_H
#define CQUAD_H
#include "../header/Angel.h"
#include "CShape.h"

#define QUAD_NUM 6		// 2 faces, 2 triangles/face 

class CQuad : public CShape
{
private:

	//vec4 m_Points[QUAD_NUM];
	//vec3 m_Normal[QUAD_NUM];	// ノ vec3 ㄓ琌竊璸衡, 狦璶琵祘Α糶癬ㄓよ獽эノ vec4 ㄓ
	//vec4 m_Colors[QUAD_NUM];

public:
	CQuad();

	void Update(float dt, const LightSource &lights);	//方
	void Update(float dt, const LightSource &Lights, const LightSource &Lights2);	//ㄢ方
	void Update(float dt, const LightSource &Lights, const LightSource &Lights2, const LightSource &Lights3);	//方
	void Update(float dt, const LightSource &Lights, const LightSource &Lights2, const LightSource &Lights3, const LightSource &Lights4);	//方
	void Update(float dt); // ぃ璸衡方酚

	GLuint GetShaderHandle() { return m_uiProgram;} 
	void RenderWithFlatShading(const LightSource &lights);//  vLightI: Light Intensity
	void RenderWithGouraudShading(const LightSource &lights);//  vLightI: Light Intensity
	void SetVtxColors(vec4 vLFColor, vec4 vLRColor, vec4 vTRColor, vec4 vTLColor); // four Vertices' Color
	void SetVtxColors(vec4 vFColor, vec4 vSColor);	// three Vertices' Color with idx as the first 

	void Draw();
	void DrawW();
};




#endif