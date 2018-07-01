#ifndef CSOLIDCUBE_H
#define CSOLIDCUBE_H
#include "../header/Angel.h"
#include "CShape.h"

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

#define SOLIDCUBE_NUM 36  // 6 faces, 2 triangles/face , 3 vertices/triangle

class CSolidCube : public CShape
{
private:
	vec4 m_vertices[8];
	int  m_iIndex;

	void Quad( int a, int b, int c, int d );
public:
	CSolidCube();
	~CSolidCube(){};

	void Update(float dt); // ぃ璸衡方酚
	void Update(float dt, const LightSource &lights);	//方
	void Update(float dt, const LightSource &Lights, const LightSource &Lights2);	//ㄢ方
	void Update(float dt, const LightSource &Lights, const LightSource &Lights2, const LightSource &Lights3);	//方
	void Update(float dt, const LightSource &Lights, const LightSource &Lights2, const LightSource &Lights3, const LightSource &Lights4);	//方

	void RenderWithFlatShading(const LightSource &lights);
	void RenderWithGouraudShading(const LightSource &lights);
	void Draw();
	void DrawW();
};

#endif