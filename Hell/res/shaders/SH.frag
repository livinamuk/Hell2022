#version 330 core

layout (location = 0) out vec4 FragColor;

#define ENV_SMPL_NUM 64

const float PI = 3.1415926535897932384626433832795;
const float goldenAngle = 2.3999632297286533222315555066336; //PI * (3.0 - sqrt(5.0));

uniform samplerCube reflectTex;

out vec3 outData;

#define NORM2SNORM(value) (value * 2.0 - 1.0)
#define SNORM2NORM(value) (value * 0.5 + 0.5)

vec3 SpherePoints_GoldenAngle(float i, float numSamples) {
	float theta = i * goldenAngle;
	float z = (1.0 - 1.0 / numSamples) * (1.0 - 2.0 * i / (numSamples - 1.0));
	float radius = sqrt(1.0 - z * z);
	return vec3(radius * vec2(cos(theta), sin(theta)), z);
}

const vec3 LUMA = vec3(0.2126, 0.7152, 0.0722);

#define SRGBtoLINEAR(c) ( c )

#define FAKE_ENV_HDR 0.4
#define FAKE_ENV_THR 0.55

vec3 sampleReflectionMap(vec3 sp, float lodBias){
	vec3 color = SRGBtoLINEAR(textureLod(reflectTex, sp, lodBias).rgb);
	#if defined (FAKE_ENV_HDR)
		color *= 1.0 + FAKE_ENV_HDR * smoothstep(FAKE_ENV_THR, 1.0, dot(LUMA, color)); //HDR for poors
	#endif
	return color;
}

// Constants, see here: http://en.wikipedia.org/wiki/Table_of_spherical_harmonics
#define k01 0.2820947918 // sqrt(  1/PI)/2
#define k02 0.4886025119 // sqrt(  3/PI)/2
#define k03 1.0925484306 // sqrt( 15/PI)/2
#define k04 0.3153915652 // sqrt(  5/PI)/4
#define k05 0.5462742153 // sqrt( 15/PI)/4
#define k06 0.5900435860 // sqrt( 70/PI)/8
#define k07 2.8906114210 // sqrt(105/PI)/2
#define k08 0.4570214810 // sqrt( 42/PI)/8
#define k09 0.3731763300 // sqrt(  7/PI)/4
#define k10 1.4453057110 // sqrt(105/PI)/4

// Y_l_m(s), where l is the band and m the range in [-l..l]
float SphericalHarmonic( in int l, in int m, in vec3 n )
{
	//----------------------------------------------------------
	if( l==0 )          return   k01;

	//----------------------------------------------------------
	if( l==1 && m==-1 ) return  -k02*n.y;
	if( l==1 && m== 0 ) return   k02*n.z;
	if( l==1 && m== 1 ) return  -k02*n.x;

	//----------------------------------------------------------
	if( l==2 && m==-2 ) return   k03*n.x*n.y;
	if( l==2 && m==-1 ) return  -k03*n.y*n.z;
	if( l==2 && m== 0 ) return   k04*(3.0*n.z*n.z-1.0);
	if( l==2 && m== 1 ) return  -k03*n.x*n.z;
	if( l==2 && m== 2 ) return   k05*(n.x*n.x-n.y*n.y);
	//----------------------------------------------------------

	return 0.0;
}

float shEvaluate(vec3 n, ivec2 idx) {
		    if (idx.x == 0 && idx.y == 0) return  SphericalHarmonic(0,  0, n);
	else if (idx.x == 0 && idx.y == 1) return -SphericalHarmonic(1, -1, n);
	else if (idx.x == 0 && idx.y == 2) return  SphericalHarmonic(1,  0, n);
	else if (idx.x == 1 && idx.y == 0) return -SphericalHarmonic(1,  1, n);
	else if (idx.x == 1 && idx.y == 1) return  SphericalHarmonic(2, -2, n);
	else if (idx.x == 1 && idx.y == 2) return -SphericalHarmonic(2, -1, n);
	else if (idx.x == 2 && idx.y == 0) return  SphericalHarmonic(2,  0, n);
	else if (idx.x == 2 && idx.y == 1) return -SphericalHarmonic(2,  1, n);
	else if (idx.x == 2 && idx.y == 2) return  SphericalHarmonic(2,  2, n);
	else return 0.0;
}

const mat3 convCoeffMat = mat3(
	1.0/1.0, 2.0/3.0, 2.0/3.0,
	2.0/3.0, 1.0/4.0, 1.0/4.0,
	1.0/4.0, 1.0/4.0, 1.0/4.0
);

vec3 shDiffuseConvolution(vec3 shRGB, ivec2 idx) {
	return shRGB * convCoeffMat[idx.x][idx.y];
}

vec3 CubeMapToSH2(ivec2 idx) {
	// Initialise sh to 0
	vec3 shRGB = vec3(0.0);

	vec2 ts = vec2(textureSize(reflectTex, 0));
	float maxMipMap = log2(max(ts.x, ts.y));

	float lodBias = maxMipMap - 6.0;

	for (int i=0; i < ENV_SMPL_NUM; ++i) {
		vec3 sp = SpherePoints_GoldenAngle(float(i), float(ENV_SMPL_NUM));
		vec3 color = sampleReflectionMap(sp, lodBias);

		float sh = shEvaluate(sp, idx);

		shRGB += sh * color;
	}

	// integrating over a sphere so each sample has a weight of 4*PI/samplecount (uniform solid angle, for each sample)
	float shFactor = 4.0 * PI / float(ENV_SMPL_NUM);
	shRGB *= shFactor;

	return shRGB;
}


void main() {
	ivec2 idx = ivec2(floor(gl_FragCoord.xy));

	vec3 shRGB = CubeMapToSH2(idx);

	#if 0
		shRGB = shDiffuseConvolution(shRGB, idx);
	#endif

	outData = shRGB;
}