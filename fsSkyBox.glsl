#version 130

in vec3 N;

// For Texture Sampler
uniform samplerCube cubeMap; // �K�Ϫ��ѼƳ]�w

void main()
{
		gl_FragColor = textureCube(cubeMap, -N);
}
