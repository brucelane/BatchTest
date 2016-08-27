 #include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Timer.h"
#include "cinder/Utilities.h"
#include "cinder/Log.h"

#include "VDFbo.h"
#include "VDShaders.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class BatchTestApp : public App {
public:
	enum SMAAMode { SMAA_EDGE_DETECTION, SMAA_BLEND_WEIGHTS, SMAA_BLEND_NEIGHBORS };
	enum DividerMode { MODE_COMPARISON, MODE_ORIGINAL1, MODE_FXAA, MODE_SMAA, MODE_ORIGINAL2, MODE_COUNT };
public:
	static void prepare(Settings *settings);

	void setup() override;
	void update() override;
	void draw() override;

	void render();

	void resize() override;
	void mouseMove(MouseEvent event) override;
	void mouseDrag(MouseEvent event) override;
	void keyDown(KeyEvent event) override;

private:
	VDShaders            mVDShaders;        // Moving, colored pistons.

	Timer               mTimer;          // We can pause and resume our timer.
	double              mTime;
	double              mTimeOffset;

	double              mFrameTime;      // Used to calculate frame rate.
	double              mFrameRate;

	gl::FboRef          mFboScene;       // Non-anti-aliased frame buffer to render our scene to.
	gl::FboRef          mFboFinal;       // Frame buffer that will contain the anti-aliased result.

	VDFbo               mFXAA;           // Takes care of applying FXAA anti-aliasing to our scene.

	vec2                mDivider;        // Determines which part of our scene is anti-aliased.
	DividerMode         mDividerMode;    // Allows us to see each of the modes (Original, FXAA, SMAA) full screen.
	int                 mDividerWidth;
	int                 mPixelSize;      // Allows us to zoom in on the scene.

	vec2                mMouse;          // Keep track of the mouse cursor.
};

void BatchTestApp::prepare(App::Settings *settings)
{
	settings->disableFrameRate();
	settings->setWindowSize(1280, 720);
}

void BatchTestApp::setup()
{
	mTime = 0.0;
	mTimeOffset = 0.0;
	mTimer.start();

	mDivider = mMouse = getWindowSize() / 2;
	mDividerWidth = getWindowWidth() / 4;
	mDividerMode = MODE_COMPARISON;
	mPixelSize = 1;

	mFrameTime = getElapsedSeconds();
	mFrameRate = 0.0;

	// Disable vertical sync, so we can evaluate the frame rate.
	gl::enableVerticalSync(false);

	// Disable alpha blending (which is enabled by default) for SMAA to work correctly.
	gl::disableAlphaBlending();
}

void BatchTestApp::update()
{
	// Keep track of time.
	mTime = mTimer.getSeconds() + mTimeOffset;

	// Show frame rate in window title.
	double elapsed = getElapsedSeconds() - mFrameTime;
	mFrameTime += elapsed;
	mFrameRate = 0.95 * mFrameRate + 0.05 * (1.0 / elapsed);
	getWindow()->setTitle(std::string("BatchTestApp - Frame rate: ") + toString(int(mFrameRate)));

	// Animate our camera.
	double t = mTime / 10.0;

	float phi = (float)t;
	float theta = 3.14159265f * (0.25f + 0.2f * math<float>::sin(phi * 0.9f));
	float x = 150.0f * math<float>::cos(phi) * math<float>::cos(theta);
	float y = 150.0f * math<float>::sin(theta);
	float z = 150.0f * math<float>::sin(phi) * math<float>::cos(theta);

	// Update the pistons.
	mVDShaders.update();

	// Move the divider.
	mDivider += 0.25f * (mMouse - mDivider);
}

void BatchTestApp::draw()
{
	// Render our scene to an Fbo.
	render();

	// Clear the main buffer.
	gl::clear();
	gl::color(Color::white());

	// Draw non-anti-aliased scene.
	if (mDividerMode == MODE_COMPARISON || mDividerMode == MODE_ORIGINAL1 || mDividerMode == MODE_ORIGINAL2)
		gl::draw(mFboScene->getColorTexture(), getWindowBounds());

	// Draw FXAA-anti-aliased scene.
	if (mDividerMode == MODE_COMPARISON || mDividerMode == MODE_FXAA) {
		mFXAA.apply(mFboFinal, mFboScene);

		if (mDividerMode == MODE_COMPARISON) {
			gl::pushMatrices();
			gl::setMatricesWindow(mDividerWidth, getWindowHeight());
			gl::pushViewport((int)mDivider.x - mDividerWidth, 0, mDividerWidth, getWindowHeight());
			gl::draw(mFboFinal->getColorTexture(), getWindowBounds().getMoveULTo(vec2(-(mDivider.x - mDividerWidth), 0)));
			gl::popViewport();
			gl::popMatrices();
		}
		else {
			gl::draw(mFboFinal->getColorTexture(), getWindowBounds());
		}
	}

	// Draw divider.
	if (mDividerMode == MODE_COMPARISON) {
		gl::drawLine(vec2((float)mDivider.x, 0), vec2((float)mDivider.x, (float)getWindowHeight()));
		gl::drawLine(vec2((float)(mDivider.x - mDividerWidth), 0), vec2((float)(mDivider.x - mDividerWidth), (float)getWindowHeight()));
		gl::drawLine(vec2((float)(mDivider.x + mDividerWidth), 0), vec2((float)(mDivider.x + mDividerWidth), (float)getWindowHeight()));
	}


	gl::disableAlphaBlending();
}

void BatchTestApp::render()
{
	// Bind the Fbo. Automatically unbinds it at the end of this function.
	gl::ScopedFramebuffer fbo(mFboScene);

	// Clear the buffer.
	gl::clear(ColorA(0, 0, 0, 0));
	gl::color(Color::white());

	// Render our scene.
	gl::pushViewport(0, 0, mFboScene->getWidth(), mFboScene->getHeight());
	mVDShaders.draw();
	gl::popViewport();
}

void BatchTestApp::resize()
{
	gl::Texture2d::Format tfmt;
	tfmt.setMinFilter(GL_NEAREST);
	tfmt.setMagFilter(GL_NEAREST);
	tfmt.setInternalFormat(GL_RGBA8);

	gl::Fbo::Format fmt;
	fmt.setColorTextureFormat(tfmt);

	mFboScene = gl::Fbo::create(getWindowWidth() / mPixelSize, getWindowHeight() / mPixelSize, fmt);
	mFboFinal = gl::Fbo::create(getWindowWidth() / mPixelSize, getWindowHeight() / mPixelSize, fmt);

	// Resize the divider.
	mDividerWidth = getWindowWidth() / 4;
}

void BatchTestApp::mouseMove(MouseEvent event)
{

}

void BatchTestApp::mouseDrag(MouseEvent event)
{

}

void BatchTestApp::keyDown(KeyEvent event)
{
	switch (event.getCode()) {
	case KeyEvent::KEY_ESCAPE:
		// Quit the application.
		quit();
		break;
	case KeyEvent::KEY_v:
		// Toggle vertical sync.
		if (gl::isVerticalSyncEnabled())
			gl::enableVerticalSync(false);
		else
			gl::enableVerticalSync();
		break;

	}
}

CINDER_APP(BatchTestApp, RendererGl, &BatchTestApp::prepare)