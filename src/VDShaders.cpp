
#include "VDShaders.h"

#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;

VDShader::VDShader()
	: mOffset(0.0f)
	, mColor(1.0f, 1.0f, 1.0f)
	, mPosition(0.0f, 0.0f, 0.0f)
{
}

VDShader::VDShader(float x, float z)
	: mOffset(ci::Rand::randFloat(0.0f, 10.0f))
	, mColor(ci::CM_HSV, ci::Rand::randFloat(0.0f, 0.1f), ci::Rand::randFloat(0.0f, 1.0f), ci::Rand::randFloat(0.25f, 1.0f))
	, mPosition(vec3(x, 0.0f, z))
{
}

void VDShader::update()
{
	//mDistance = glm::distance2(camera.getEyePoint(), mPosition);
}

void VDShader::draw()
{
	float t = mOffset;
	float height = 55.0f + 45.0f *  ci::math<float>::sin(t);
	mPosition.y = 0.5f * height;

	gl::color(mColor);
	gl::drawCube(mPosition, vec3(10.0f, height, 10.0f));
}

//////////////////////////////////////////////////////////////////////

VDShaders::VDShaders()
{
	mVDShaders.clear();

	for (int x = -50; x <= 50; x += 10)
		for (int z = -50; z <= 50; z += 10)
			mVDShaders.push_back(VDShader(float(x), float(z)));

	// Load and compile our shaders and textures
	try {
		auto glsl = gl::GlslProg::Format().vertex(CI_GLSL(150,

			uniform mat4	ciModelView;
		uniform mat4	ciModelViewProjection;
		uniform mat3	ciNormalMatrix;

		in vec4			ciPosition;
		in vec3			ciNormal;
		in vec4			ciColor;
		in vec2			ciTexCoord0;

		out vec4		vVertex;
		out vec3		vNormal;
		out vec4		vColor;
		out vec2		vTexCoord0;

		void main()
		{
			vVertex = ciModelView * ciPosition;
			vNormal = ciNormalMatrix * ciNormal;
			vColor = ciColor;
			vTexCoord0 = ciTexCoord0;

			gl_Position = ciModelViewProjection * ciPosition;
		}

		)).fragment(CI_GLSL(150,

			in vec4			vVertex;
		in vec3			vNormal;
		in vec4			vColor;
		in vec2			vTexCoord0;

		out vec4		oColor;

		void main()
		{
			vec2 uv = vTexCoord0;
			vec3 L = normalize(-vVertex.xyz);
			vec3 E = normalize(-vVertex.xyz);
			vec3 R = normalize(-reflect(L, vNormal));

			// diffuse term with fake ambient occlusion
			float occlusion = 0.5 + 0.5*16.0*uv.x*uv.y*(1.0 - uv.x)*(1.0 - uv.y);
			vec4 diffuse = vColor * occlusion * max(dot(vNormal, L), 0.0);

			// specular term
			vec4 specular = vColor * pow(max(dot(R, E), 0.0), 50.0);

			// write gamma corrected final color
			oColor.rgb = sqrt(diffuse + specular).rgb;

			// write luminance in alpha channel (required for FXAA)
			const vec3 luminance = vec3(0.299, 0.587, 0.114);
			oColor.a = sqrt(dot(oColor.rgb, luminance));
		}
		));

		mShader = gl::GlslProg::create(glsl);
	}
	catch (const std::exception& e) {
		console() << e.what() << std::endl;
		app::App::get()->quit();
	}
}

void VDShaders::update()
{
	for (auto &VDShader : mVDShaders)
		VDShader.update();

	std::qsort(&mVDShaders.front(), mVDShaders.size(), sizeof(VDShader), &VDShader::CompareByDistanceToCamera);
}

void VDShaders::draw()
{
	gl::enableDepthRead();
	gl::enableDepthWrite();

	//gl::pushMatrices();
	//gl::setMatrices(camera);

	gl::ScopedGlslProg glslProg(mShader);

	for (auto &VDShader : mVDShaders)
		VDShader.draw();

	//gl::popMatrices();

	gl::disableDepthWrite();
	gl::disableDepthRead();
}