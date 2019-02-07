attribute vec3 Position;

varying vec4 FragColor;

uniform mat4 gWorld;
uniform mat4 trans1;
uniform mat4 scale1;
uniform mat4 theta;
uniform mat4 worldTheta;
uniform mat4 surfaceScale;
uniform mat4 worldTrans;
uniform mat4 trackball;
uniform float flag1;

void main()
{
	vec4 normal;

	if(flag1==1.0 || flag1==2.0)	
		normal = trackball * theta * vec4(Position.xy, 0.0, 0.0);
	else if(flag1==3.0 || flag1==3.5)
		normal = trackball * vec4(Position, 0.0);
	else if(flag1==4.0 || flag1==6.0)
		normal = trackball * vec4(Position, 0.0);
	else if(flag1==5.0)
		normal = vec4(0.0, 1.0, 0.0, 0.0);


	if(flag1==4.0)
	    gl_Position = trackball * surfaceScale * vec4(Position, 1.0);		
	else if(flag1==5.0)
		gl_Position = vec4(Position, 1.0);		
	else if(flag1==6.0)
		gl_Position = trackball * scale1 * trans1 * vec4(Position, 1.0);
	else
	    gl_Position = trackball * trans1 * theta * scale1 * vec4(Position, 1.0);

	//Light calculations
	vec3 LightPosition = vec3(0.5, 0.5, -1.0);
	
	vec3 L = normalize( LightPosition - gl_Position.xyz );
	vec3 E = normalize( -gl_Position.xyz );
	vec3 H = normalize( L + E );

	vec3 N = normalize( normal ).xyz;

	vec3 ambient = vec3(0.4, 0.4, 0.4);

	float Kd = max( dot(L, N), 0.0 );
	vec3 diffuse = Kd * vec3(0.5, 0.5, 0.5);

	float Ks = pow( max(dot(N, H), 0.0), 100 );
	vec3 specular = Ks * vec3(0.5, 0.5, 0.5);
	if( dot(L, N) < 0.0 ) specular = vec3(0.0, 0.0, 0.0);


	float temp=(Position.z+1.0)/2.0;

	if(flag1==1.0)								//Grid
 		FragColor = vec4(0.0, 0.0, 1.0, 1.0);
	else if(flag1==2.0)							//Box
		FragColor = vec4(0.0, 1.0, 1.0, 1.0);
	else if(flag1==3.0)							//Not selected vertices
		FragColor = vec4(0.0, 1.0, 0.0, 1.0);	
	else if(flag1==3.5)							//Selected vertices
		FragColor = vec4(1.0, 1.0, 1.0, 1.0);
	else if(flag1==4.0)							//Surface
		FragColor = vec4(temp, 1.0-temp, 0.0, 0.1);
	else if(flag1==5.0)							//Floor
		FragColor = vec4(1.0, 1.0, 1.0, 1.0);
	else if(flag1==6.0)							//Output
		FragColor = vec4(1.0, 1.0, 1.0, 1.0);

    FragColor = vec4(ambient + diffuse + specular, 1.0) * FragColor;

		
	if(flag1!=5.0)
		gl_Position = worldTrans * gl_Position;
}
