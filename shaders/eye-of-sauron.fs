#define TAU 6.28318530718
#define MOD2 vec2(.16632,.17369)
#define MOD3 vec3(.16532,.17369,.15787)
float gTime;
float flareUp;

vec2 Rotate2axis(vec2 p, float a)
{
	float si = sin(a);
	float co = cos(a);
	return mat2(si, co, -co, si) * p;
}

float Hash(float p)
{
	vec2 p2 = fract(vec2(p) * MOD2);
    p2 += dot(p2.yx, p2.xy+19.19);
	return fract(p2.x * p2.y);
}
 
float EyeNoise( in float x )
{
    float p = floor(x);
    float f = fract(x);
	f = clamp(pow(f, 7.0), 0.0,1.0);
	//f = f*f*(3.0-2.0*f);
    return mix(Hash(p), Hash(p+1.0), f);
}

float Bump( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);
	//f = f*f*(3.0-2.0*f);
	
	vec2 uv = (p.xy + vec2(37.0, 17.0) * p.z) + f.xy;
	vec2 rg = textureLod( iChannel0, (uv+.5)/256., 0.).yx;
	return mix(rg.x, rg.y, f.z);
}

float Noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);
	f = f*f*(3.0-2.0*f);
	vec2 uv = (p.xy + vec2(37.0, 17.0) * p.z) + f.xy;
	vec2 rg = textureLod( iChannel0, (uv+.5)/256., 0.).yx;
	return mix(rg.x, rg.y, f.z);
}

float Pupil(vec3 p, float r)
{
	// It's just a stretched sphere but the mirrored
	// halves are push together to make a sharper top and bottom.
	p.xz = abs(p.xz)+.25;
	return length(p) - r;
}

//=================================================================================================
float DE_Fire(vec3 p)
{
	p *= vec3(1.0, 1.0, 1.5);
	float len = length(p);
	float ax = atan(p.y, p.x)*10.0;
	float ay = atan(p.y, p.z)*10.0;
	vec3 shape = vec3(len*.5-gTime*1.2, ax, ay) * 2.0;
	
	shape += 2.5 * (Noise(p * .25) -
				 	Noise(p * 0.5) * .5 +
					Noise(p * 2.0) * .25);
	float f = Noise(shape)*6.0;
	f += (smoothstep(7.30, 8.3+flareUp, len)*smoothstep(12.0+flareUp*2.0, 8.0, len)) * 3.0;
	p *= vec3(.75, 1.2, 1.0);
	len = length(p);
	f = mix(f, 0.0, smoothstep(12.5+flareUp, 16.5+flareUp, len));
	return f;
}

//=================================================================================================
float Sphere(vec3 p, float r)
{
	return length(p) - r;
}

//=================================================================================================
float DE_Pupil(vec3 p)
{
	float time = gTime * .5+sin(gTime*.3)*.5;
	float t = EyeNoise(time) * .125 +.125;
	p.yz = Rotate2axis(p.yz, t * TAU);
	p *= vec3(1.2-EyeNoise(time+32.5)*.5, .155, 1.0);
	t = EyeNoise(time-31.0) * .125 +.1875;
	p.xz = Rotate2axis(p.xz, t*TAU);
	p += vec3(.0, 0.0, 4.);
	
	float  d = Pupil(p, .78);
	return d * max(1.0, abs(p.y*2.5));
}

//=================================================================================================
vec4 Raymarch( in vec3 ro, in vec3 rd, in vec2 fragCoord, inout bool hit, out float pupil)
{
	float sum = 0.0;
	// Starting point plus dither to prevent edge banding...
	float t = 14.0 + .1 * texture(iChannel0, fragCoord.xy / iChannelResolution[0].xy).y;
	vec3 pos = vec3(0.0, 0.0, 0.0);
	float d = 100.0;
	pupil = 0.0;
	for(int i=0; i < 197; i++)
	{
        if (t > 37.0) break;
		pos = ro + t*rd;
		vec3 shape = pos * vec3(1.5, .4, 1.5);
	
		// Accumulate pixel denisity depending on the distance to the pupil
		d = DE_Pupil(pos);
		pupil += smoothstep(0.02 +Noise(pos*4.0+gTime)*.3, 0.0, d) * .17;

		// Add fire around pupil...
		sum += smoothstep(1.3, 0.0, d) * .014;
		
		sum += max(DE_Fire(pos), 0.0) * .00162;
    	t += max(.1, t*.0057);

	}
	
	return vec4(pos, clamp(sum*sum*sum, 0.0, 1.0 ));
}

vec3 FlameColour(float f)
{
	f = f*f*(3.0-2.0*f);
	return  min(vec3(f+.8, f*f*1.4+.05, f*f*f*.6) * f, 1.0);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    gTime = iTime + 44.29;
    flareUp = max(sin(gTime*.75+3.5), 0.0);
	vec2 uv = fragCoord.xy / iResolution.xy;
	vec2 p = -1.0 + 2.0 * uv;
	p.x *= iResolution.x/iResolution.y;

	vec3 origin = vec3(sin(gTime*.34)*5.0, -10.0 - sin(gTime*.415) * 6.0, -20.0+sin(gTime*.15) * 2.0);
	vec3 target = vec3( 0.0, 0.0, 0.0 );
	
	// Make camera ray using origin and target positions...
	vec3 cw = normalize( target-origin);
	vec3 cp = vec3(0.0, 1.0, 0.0);
	vec3 cu = normalize( cross(cw, cp) );
	vec3 cv = ( cross(cu,cw) );
	vec3 ray = normalize(p.x*cu + p.y*cv + 1.5 * cw );
	
	bool hit = false;
	float pupil = 0.0;
	vec4 ret = Raymarch(origin, ray, fragCoord, hit, pupil);
	vec3 col = vec3(0.0);

	vec3 light = vec3(0.0, 4.0, -4.0);
	
	col += FlameColour(ret.w);
	col = mix (col, vec3(0.0), min(pupil, 1.0));
	
	fragColor = vec4(min(col, 1.0),1.0);	
}