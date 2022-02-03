
uniform vec2 lightpos;
uniform vec3 lightColor;
uniform float screenHeight;
uniform vec3 lightAttenuation;
uniform float radius;

void main()
{		
	vec2 pixel = gl_FragCoord.xy;
	pixel.y = screenHeight-pixel.y;
	vec2 aux = lightpos-pixel;
	float distance=length(aux);
	float attenuation=1.0/(lightAttenuation.x+lightAttenuation.y*distance+lightAttenuation.z*distance*distance);
	vec4 color=vec4(attenuation,attenuation,attenuation,1.0)*vec4(lightColor,1.0);
	gl_FragColor = color;
}