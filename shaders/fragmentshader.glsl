#version 330 core

out vec4 FragColor;

uniform vec3 color;
uniform vec3 gscale;
uniform bool cctv;

void main()
{
	if(cctv){
		float gray = dot(color.rgb, gscale);
    	FragColor = vec4(gray,gray,gray,1.0);
	}
	else{
		FragColor = vec4(color.x*gscale.x,color.y*gscale.y,color.z*gscale.z, 1.0f);
	}
}