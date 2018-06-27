#version 130

in vec3 N;

// For Texture Sampler
uniform samplerCube cubeMap; // 貼圖的參數設定

void main()
{
		gl_FragColor = textureCube(cubeMap, -N);
}
