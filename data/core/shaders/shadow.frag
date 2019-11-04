#version 150 core

#define BIAS 0.01

// see third comment :
// https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
void main()
{
	gl_FragDepth = gl_FragCoord.z + float(gl_FrontFacing) * BIAS;
	// float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005); //  can
	// also be used here
}
