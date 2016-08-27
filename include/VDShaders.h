#pragma once

#include "cinder/gl/GlslProg.h"
#include "cinder/Cinder.h"
#include "cinder/Color.h"
#include "cinder/Vector.h"
#include <vector>


typedef std::shared_ptr<class VDShader> VDShaderRef;

class VDShader {
public:
	VDShader();
	VDShader(float x, float z);

	void update();
	void draw();

	// Our custom sorting comparator.
	static int CompareByDistanceToCamera(const void* a, const void* b)
	{
		const VDShader* pA = reinterpret_cast<const VDShader*>(a);
		const VDShader* pB = reinterpret_cast<const VDShader*>(b);
		if (pA->mDistance < pB->mDistance)
			return -1;
		if (pA->mDistance > pB->mDistance)
			return 1;
		return 0;
	}

private:
	float      mDistance;
	float      mOffset;
	ci::Colorf mColor;
	ci::vec3   mPosition;
};

class VDShaders {
public:
	VDShaders();

	void update();
	void draw();
private:
	std::vector<VDShader>    mVDShaders;
	ci::gl::GlslProgRef    mShader;
};

