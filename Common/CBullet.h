#ifndef CBULLET_H
#define CBULLET_H
#include "../header/Angel.h"
#include "CShape.h"

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

class CBullet : public CShape
{
private:
	GLfloat m_fRadius;
	GLint m_iSlices, m_iStacks;

public:
	CBullet(const GLfloat fRadius = 1.0f, const int iSlices = 12, const  int iStacks = 6);
	~CBullet();
	CBullet *link;		//for link list
	mat4& GetTRSMatrix();

	void Update(float dt, const LightSource &lights);	//方
	void Update(float dt, const LightSource &Lights, const LightSource &Lights2);	//ㄢ方
	void Update(float dt, const LightSource &Lights, const LightSource &Lights2, const LightSource &Lights3);	//方
	void Update(float dt, const LightSource &Lights, const LightSource &Lights2, const LightSource &Lights3, const LightSource &Lights4);	//方
	void Update(float dt); // ぃ璸衡方酚

	void RenderWithGouraudShading(const LightSource &lights);
	void Draw();
	void DrawW(); // ㊣ぃΩ砞﹚ Shader 磞酶よΑ
};

#endif