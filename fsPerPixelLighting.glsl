#version 130
// #define NPR
// iTexLayer ���]�w�P�P�_�覡�A���]���B�i�H�䴩�줻�i�K��

#define NONE_MAP    0
#define DIFFUSE_MAP 1
#define LIGHT_MAP   2
#define NORMAL_MAP  4

in vec3 v3N;
in vec3 v3E;
in vec3 v3L; // Light Direction �b���Y�y�ФU����V
in vec3 v3L2;
in vec3 v3L3;
in vec3 v3L4;

in vec2 DiffuseMapUV;   // ��J Diffuse Map �K�Ϯy��
in vec2 LightMapUV;     // ��J Light Map �K�Ϯy��

uniform int iTexLayer;
uniform float fElapsedTime;

// �H�U���s�W������
uniform float fShininess;
uniform int   iLighting;
uniform vec4  vObjectColor;    // �N���󪺳�@�C��

// Light 1
uniform vec4  LightInView;        // Light's position in View Space
uniform vec4  AmbientProduct;	// light's ambient  �P Object's ambient  �P ka �����n
uniform vec4  DiffuseProduct;	// light's diffuse  �P Object's diffuse  �P kd �����n
uniform vec4  SpecularProduct;	// light's specular �P Object's specular �P ks �����n

// Light 2
uniform vec4  LightInView2;        // Light's position in View Space
uniform vec4  AmbientProduct2;  // light's ambient  �P Object's ambient  �P ka �����n
uniform vec4  DiffuseProduct2;  // light's diffuse  �P Object's diffuse  �P kd �����n
uniform vec4  SpecularProduct2; // light's specular �P Object's specular �P ks �����n

// Light 3
uniform vec4  LightInView3;        // Light's position in View Space
uniform vec4  AmbientProduct3;  // light's ambient  �P Object's ambient  �P ka �����n
uniform vec4  DiffuseProduct3;  // light's diffuse  �P Object's diffuse  �P kd �����n
uniform vec4  SpecularProduct3; // light's specular �P Object's specular �P ks �����n

// Light 4
uniform vec4  LightInView4;        // Light's position in View Space
uniform vec4  AmbientProduct4;  // light's ambient  �P Object's ambient  �P ka �����n
uniform vec4  DiffuseProduct4;  // light's diffuse  �P Object's diffuse  �P kd �����n
uniform vec4  SpecularProduct4; // light's specular �P Object's specular �P ks �����n

// For Texture Sampler
uniform sampler2D diffuMap; // �K�Ϫ��ѼƳ]�w
uniform sampler2D lightMap; // �K�Ϫ��ѼƳ]�w

// Light Map �C��
uniform vec4 LightMapColor;

void main()
{
	// ���ŧi diffuse �P specular
	vec4 LightingColor = vec4(0.0,0.0,0.0,1.0);

	vec4 diffuse = vec4(0.0,0.0,0.0,1.0);
	vec4 specular = vec4(0.0,0.0,0.0,1.0);

	vec4 diffuse2 = vec4(0.0,0.0,0.0,1.0);
	vec4 specular2 = vec4(0.0,0.0,0.0,1.0);

	vec4 diffuse3 = vec4(0.0,0.0,0.0,1.0);
	vec4 specular3 = vec4(0.0,0.0,0.0,1.0);

	vec4 diffuse4 = vec4(0.0,0.0,0.0,1.0);
	vec4 specular4 = vec4(0.0,0.0,0.0,1.0);
	
	if( iLighting != 1 ) {
		if( iTexLayer == NONE_MAP ) gl_FragColor = vObjectColor;
		else gl_FragColor = vObjectColor * texture2D(diffuMap, DiffuseMapUV); 
	}
	else {	
		// 1. �p�� Ambient color : Ia = AmbientProduct = Ka * Material.ambient * La = 
		vec4 ambient = AmbientProduct; // m_sMaterial.ka * m_sMaterial.ambient * vLightI;

		// ���ƶǤJ�� Normal Dir
		vec3 vN = normalize(v3N); 

		// 2. ���ƶǤJ�� Light Dir
		vec3 vL = normalize(v3L); // normalize light vector
		vec3 vL2 = normalize(v3L2); // normalize light vector
		vec3 vL3 = normalize(v3L3); // normalize light vector
		vec3 vL4 = normalize(v3L4); // normalize light vector

		// 5. �p�� L dot N
		float fLdotN = vL.x*vN.x + vL.y*vN.y + vL.z*vN.z;				//------ Light 1 ------//
		if( fLdotN >= 0 ) { // ���I�Q�����Ө�~�ݭn�p��
#ifndef NPR
			// Diffuse Color : Id = Kd * Material.diffuse * Ld * (L dot N)
			diffuse = fLdotN * DiffuseProduct; 
#else
			if( fLdotN >= 0.85 ) diffuse = 1.0 * DiffuseProduct;
			else if( fLdotN >= 0.55 ) diffuse = 0.55 * DiffuseProduct;
			else diffuse = 0.35 * DiffuseProduct;
#endif

			// Specular color
			// Method 1: Phone Model
			// �p�� View Vector
			// ���ƶǤJ�� v3E , View Direction
			vec3 vV = normalize(v3E);

			//�p�� Light �� �Ϯg�� vRefL = 2 * fLdotN * vN - L
			// �P�ɧQ�� normalize �ন���V�q
			vec3 vRefL = normalize(2.0f * (fLdotN) * vN - vL);

			//   �p��  vReflectedL dot View
			float RdotV = vRefL.x*vV.x + vRefL.y*vV.y + vRefL.z*vV.z;

#ifndef NPR
			// Specular Color : Is = Ks * Material.specular * Ls * (R dot V)^Shininess;
			if( RdotV > 0 ) specular = SpecularProduct * pow(RdotV, fShininess); 
#else
			specular = vec4(0.0,0.0,0.0,1.0); 
#endif
		}

		float fLdotN2 = vL2.x*vN.x + vL2.y*vN.y + vL2.z*vN.z;				//------ Light 2 ------//
		if( fLdotN2 >= 0 ) { // ���I�Q�����Ө�~�ݭn�p��
#ifndef NPR
			// Diffuse Color : Id = Kd * Material.diffuse * Ld * (L dot N)
			diffuse2 = fLdotN2 * DiffuseProduct2; 
#else
			if( fLdotN2 >= 0.85 ) diffuse2 = 1.0 * DiffuseProduct2;
			else if( fLdotN2 >= 0.55 ) diffuse2 = 0.55 * DiffuseProduct2;
			else diffuse2 = 0.35 * DiffuseProduct2;
#endif

			// Specular color
			// Method 1: Phone Model
			// �p�� View Vector
			// ���ƶǤJ�� v3E , View Direction
			vec3 vV2 = normalize(v3E);

			//�p�� Light �� �Ϯg�� vRefL = 2 * fLdotN * vN - L
			// �P�ɧQ�� normalize �ন���V�q
			vec3 vRefL2 = normalize(2.0f * (fLdotN2) * vN - vL2);

			//   �p��  vReflectedL dot View
			float RdotV2 = vRefL2.x*vV2.x + vRefL2.y*vV2.y + vRefL2.z*vV2.z;

#ifndef NPR
			// Specular Color : Is = Ks * Material.specular * Ls * (R dot V)^Shininess;
			if( RdotV2 > 0 ) specular2 = SpecularProduct2 * pow(RdotV2, fShininess); 
#else
			specular2 = vec4(0.0,0.0,0.0,1.0); 
#endif
		}

		float fLdotN3 = vL3.x*vN.x + vL3.y*vN.y + vL3.z*vN.z;				//------ Light 3 ------//
		if( fLdotN3 >= 0 ) { // ���I�Q�����Ө�~�ݭn�p��
#ifndef NPR
			// Diffuse Color : Id = Kd * Material.diffuse * Ld * (L dot N)
			diffuse3 = fLdotN3 * DiffuseProduct3; 
#else
			if( fLdotN3 >= 0.85 ) diffuse3 = 1.0 * DiffuseProduct3;
			else if( fLdotN3 >= 0.55 ) diffuse3 = 0.55 * DiffuseProduct3;
			else diffuse3 = 0.35 * DiffuseProduct3;
#endif

			// Specular color
			// Method 1: Phone Model
			// �p�� View Vector
			// ���ƶǤJ�� v3E , View Direction
			vec3 vV3 = normalize(v3E);

			//�p�� Light �� �Ϯg�� vRefL = 2 * fLdotN * vN - L
			// �P�ɧQ�� normalize �ন���V�q
			vec3 vRefL3 = normalize(2.0f * (fLdotN3) * vN - vL3);

			//   �p��  vReflectedL dot View
			float RdotV3 = vRefL3.x*vV3.x + vRefL3.y*vV3.y + vRefL3.z*vV3.z;

#ifndef NPR
			// Specular Color : Is = Ks * Material.specular * Ls * (R dot V)^Shininess;
			if( RdotV3 > 0 ) specular3 = SpecularProduct3 * pow(RdotV3, fShininess); 
#else
			specular3 = vec4(0.0,0.0,0.0,1.0); 
#endif
		}

		float fLdotN4 = vL4.x*vN.x + vL4.y*vN.y + vL4.z*vN.z;				//------ Light 4 ------//
		if( fLdotN4 >= 0 ) { // ���I�Q�����Ө�~�ݭn�p��
#ifndef NPR
			// Diffuse Color : Id = Kd * Material.diffuse * Ld * (L dot N)
			diffuse4 = fLdotN4 * DiffuseProduct4; 
#else
			if( fLdotN4 >= 0.85 ) diffuse4 = 1.0 * DiffuseProduct4;
			else if( fLdotN4 >= 0.55 ) diffuse4 = 0.55 * DiffuseProduct4;
			else diffuse4 = 0.35 * DiffuseProduct4;
#endif

			// Specular color
			// Method 1: Phone Model
			// �p�� View Vector
			// ���ƶǤJ�� v3E , View Direction
			vec3 vV4 = normalize(v3E);

			//�p�� Light �� �Ϯg�� vRefL = 2 * fLdotN * vN - L
			// �P�ɧQ�� normalize �ন���V�q
			vec3 vRefL4 = normalize(2.0f * (fLdotN4) * vN - vL4);

			//   �p��  vReflectedL dot View
			float RdotV4 = vRefL4.x*vV4.x + vRefL4.y*vV4.y + vRefL4.z*vV4.z;

#ifndef NPR
			// Specular Color : Is = Ks * Material.specular * Ls * (R dot V)^Shininess;
			if( RdotV4 > 0 ) specular4 = SpecularProduct4 * pow(RdotV4, fShininess); 
#else
			specular4 = vec4(0.0,0.0,0.0,1.0); 
#endif
		}


		// ------------ �p���C�� ambient + diffuse + specular -------------
		LightingColor = ambient + diffuse + specular + diffuse2 + specular2 + diffuse3 + specular3 + diffuse4 + specular4;
		LightingColor.w = DiffuseProduct.w;	// �]�w���ǤJ���誺 alpha,	DiffuseProduct.w
		//LightingColor.w = 1.0;	// �]�w alpha �� 1.0


		if( iTexLayer == NONE_MAP ) gl_FragColor = LightingColor;
		else if( iTexLayer == DIFFUSE_MAP || iTexLayer == (DIFFUSE_MAP|NORMAL_MAP) ) gl_FragColor = LightingColor * texture2D(diffuMap, DiffuseMapUV);
		else if( iTexLayer == (DIFFUSE_MAP|LIGHT_MAP)) {
			gl_FragColor =  ( 0.65 * LightingColor * texture2D(diffuMap, DiffuseMapUV)  + texture2D(diffuMap, DiffuseMapUV) * 
						    texture2D(lightMap, LightMapUV) * LightMapColor );

//			float t = 0.15 + 0.75 * abs(sin(fElapsedTime * 3.1415926 * 0.125));
//			gl_FragColor =  ( 0.55 * LightingColor * texture2D(diffuMap, DiffuseMapUV)  + // texture2D(diffuMap, DiffuseMapUV) * 
//							texture2D(lightMap, LightMapUV) * LightMapColor * t);
		}
	}
}
