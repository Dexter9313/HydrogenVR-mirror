#version 150 core

uniform sampler2D diffuse;
uniform sampler2D specular;
uniform sampler2D ambient;
uniform sampler2D emissive;
uniform sampler2D normals;
uniform sampler2D shininess;
uniform sampler2D opacity;
uniform sampler2D lightmap;

in vec3 f_position;
in vec3 f_tangent;
in vec3 f_normal;
in vec2 f_texcoord;

uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform float lightAmbiantFactor;

out vec4 outColor;

void main()
{
	mat3 fromtangentspace;
	fromtangentspace[0] = normalize(f_tangent);
	fromtangentspace[2] = normalize(f_normal);

	// Gram-Schmidt re-orthogonalize T with respect to N
	fromtangentspace[0] = normalize(
	    fromtangentspace[0]
	    - dot(fromtangentspace[0], fromtangentspace[2]) * fromtangentspace[2]);

	fromtangentspace[1] = cross(fromtangentspace[2], fromtangentspace[0]);

	vec4 diffuseColor   = texture(diffuse, f_texcoord);
	vec4 specularColor  = texture(specular, f_texcoord);
	vec4 ambientColor   = texture(ambient, f_texcoord);
	vec4 emissiveColor  = texture(emissive, f_texcoord);
	vec4 normalColor    = texture(normals, f_texcoord);
	vec4 shininessColor = texture(shininess, f_texcoord);
	vec4 opacityColor   = texture(opacity, f_texcoord);
	vec4 lightmapColor  = texture(lightmap, f_texcoord);

	vec3 normal = fromtangentspace * (normalColor.rgb * 2.0 - 1.0);

	// todo use normalmap
	float lightcoeff
	    = max(lightAmbiantFactor,
	          dot(normalize(normal), normalize(f_position - lightPosition)));

	// diffuse
	outColor.rgb
	    = lightcoeff * diffuseColor.rgb * lightColor + emissiveColor.rgb;
	// transparency
	outColor.a = 1.0;

	// lightmap
	outColor.rgb *= lightmapColor.r;
}
