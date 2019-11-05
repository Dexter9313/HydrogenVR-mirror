vec4 computeLightSpacePosition(mat4 lightspace, vec3 position, vec3 normal,
                               float boundingSphereRadius)
{
	return lightspace
	       * vec4(position + 0.00072 * normal * boundingSphereRadius, 1.0);
}

float computeShadow(vec4 lightspacepos, sampler2D shadowmap)
{
	vec3 projCoords    = lightspacepos.xyz / lightspacepos.w;
	projCoords         = projCoords * 0.5 + 0.5;
	float currentDepth = projCoords.z;

	float shadow = 0.0;
#ifdef SMOOTHSHADOWS
	const int samples    = 7;
	const float diskSize = 10.0;
	const float factor   = 3.0 * diskSize / samples;
	vec2 texelSize       = factor / textureSize(shadowmap, 0);

	for(int x = -1 * (samples / 2); x <= (samples / 2); ++x)
	{
		for(int y = -1 * (samples / 2); y <= (samples / 2); ++y)
		{
			float pcfDepth
			    = texture(shadowmap, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth > pcfDepth ? 0.0 : 1.0;
		}
	}
	shadow /= samples * samples;

#else
	float depth = texture(shadowmap, projCoords.xy).r;
	shadow      = currentDepth > depth ? 0.0 : 1.0;
#endif

	return shadow;
}
