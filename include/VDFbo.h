#pragma once

#include "cinder/gl/gl.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"

class VDFbo {
public:
	VDFbo();

	void draw(const ci::gl::TextureRef &source, const ci::Area &bounds);
	void apply(const ci::gl::FboRef &destination, const ci::gl::FboRef &source);
private:
	ci::gl::GlslProgRef	mGlslProg;
};