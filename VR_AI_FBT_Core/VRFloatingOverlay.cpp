

#include "VRFloatingOverlay.h"

#include <QOpenGLFramebufferObjectFormat>
#include <QOpenGLPaintDevice>
#include <QPainter>
#include <QtWidgets/QWidget>
#include <QMouseEvent>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsEllipseItem>
#include <QCursor>
#include <QtWidgets/QApplication>

using namespace vr;

#include <iostream>

glm::vec2 headsetOffsets(.25f, -.375f);

VRFloatingOverlay* s_pSharedVRController = NULL;


VRFloatingOverlay* VRFloatingOverlay::SharedInstance()
{
	if (!s_pSharedVRController)
	{
		s_pSharedVRController = new VRFloatingOverlay();
	}
	return s_pSharedVRController;
}

void VRFloatingOverlay::QueueText(QString text, float duration) {
	FloatingOverlayText fot;
	fot.text = text;
	fot.duration = duration;
	textQueue.push(fot);
}
std::chrono::high_resolution_clock::time_point durationStart;
bool hasDuration = false;
void VRFloatingOverlay::UpdateTextUI() {
	if (!textQueue.empty()) {
		std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
		if (hasDuration) {
			std::chrono::duration<float> dt = now - durationStart;
			if (dt.count() > textQueue.front().duration) {
				textQueue.pop();
				if (textQueue.empty()) {
					hasDuration = false;
					vr::VROverlay()->HideOverlay(m_ulOverlayHandle);
					return;
				}
			}
			else return;
		}
		durationStart = now;
		((OverlayWidget*)m_pWidget)->SetText(textQueue.front().text);
		hasDuration = true;
		vr::VROverlay()->ShowOverlay(m_ulOverlayHandle);
		OnSceneChanged(QList<QRectF>());
	}
}
void VRFloatingOverlay::UpdateTransform() {
	if (!hasDuration) return;

	glm::vec3 headsetForward = headRot * glm::vec3(0, 0, -1);
	glm::vec3 headsetUp = headRot * glm::vec3(0, 1, 0);
	glm::vec3 headsetRight = headRot * glm::vec3(1, 0, 0);

	glm::vec3 posDelta = glm::vec3(0, 1, 0) * headsetOffsets.y;
	posDelta += glm::normalize(reject(headsetForward, glm::vec3(0, 1, 0))) * headsetOffsets.x;

	glm::mat4x4 matrix = glm::lookAt(glm::vec3(0, 0, 0), glm::normalize(posDelta), glm::vec3(0, 1, 0));

	vr::HmdMatrix34_t transform = ConvertMatrix(matrix, headPos + posDelta);

	vr::VROverlay()->SetOverlayTransformAbsolute(m_ulOverlayHandle, vr::ETrackingUniverseOrigin::TrackingUniverseStanding, &transform);
}

VRFloatingOverlay::VRFloatingOverlay()
	: BaseClass()
	, m_eLastHmdError(vr::VRInitError_None)
	, m_eCompositorError(vr::VRInitError_None)
	, m_eOverlayError(vr::VRInitError_None)
	, m_strVRDriver("No Driver")
	, m_strVRDisplay("No Display")
	, m_pOpenGLContext(NULL)
	, m_pScene(NULL)
	, m_pOffscreenSurface(NULL)
	, m_pFbo(NULL)
	, m_pWidget(NULL)
	, m_pPumpEventsTimer(NULL)
	, m_lastMouseButtons(0)
	, m_ulOverlayHandle(vr::k_ulOverlayHandleInvalid)
	, m_bManualMouseHandling(false)
{
	std::cout << "Starting\n";
	OverlayWidget* pOverlayWidget = new OverlayWidget();

	std::cout << "Init\n";
	Init();
	if (vr::VROverlay())
	{
		pOverlayWidget->OverlayHandle = m_ulOverlayHandle;
	}

	std::cout << "Set Widget\n";
	SetWidget(pOverlayWidget);

	std::cout << "Started\n";
}


VRFloatingOverlay::~VRFloatingOverlay()
{
}


bool VRFloatingOverlay::Init()
{
	bool bSuccess = true;

	m_strName = "systemoverlay";

	QStringList arguments = qApp->arguments();

	int nNameArg = arguments.indexOf("-name");
	if (nNameArg != -1 && nNameArg + 2 <= arguments.size())
	{
		m_strName = arguments.at(nNameArg + 1);
	}

	QSurfaceFormat format = QSurfaceFormat::defaultFormat();
	format.setMajorVersion(4);
	format.setMinorVersion(1);
	format.setSamples(4);
	format.setProfile(QSurfaceFormat::CompatibilityProfile);

	m_pOpenGLContext = new QOpenGLContext();
	m_pOpenGLContext->setFormat(format);
	bSuccess = m_pOpenGLContext->create();
	if (!bSuccess)
		return false;

	// create an offscreen surface to attach the context and FBO to
	m_pOffscreenSurface = new QOffscreenSurface();
	m_pOffscreenSurface->create();
	m_pOpenGLContext->makeCurrent(m_pOffscreenSurface);

	m_pScene = new QGraphicsScene();
	connect(m_pScene, SIGNAL(changed(const QList<QRectF>&)), this, SLOT(OnSceneChanged(const QList<QRectF>&)));

	bSuccess = vr::VRCompositor() != NULL;

	if (vr::VROverlay())
	{
		vr::VROverlayError overlayError = vr::VROverlay()->CreateOverlay(OverlayKey, OverlayName, &m_ulOverlayHandle);
		bSuccess = bSuccess && overlayError == vr::VROverlayError_None;
	}

	if (bSuccess)
	{

		m_pPumpEventsTimer = new QTimer(this);
		connect(m_pPumpEventsTimer, SIGNAL(timeout()), this, SLOT(OnTimeoutPumpEvents()));
		m_pPumpEventsTimer->setInterval(20);
		m_pPumpEventsTimer->start();

	}
	return true;
}
void VRFloatingOverlay::Shutdown()
{
	delete m_pScene;
	delete m_pFbo;
	delete m_pOffscreenSurface;

	if (m_pOpenGLContext)
	{
		//		m_pOpenGLContext->destroy();
		delete m_pOpenGLContext;
		m_pOpenGLContext = NULL;
	}
}


void VRFloatingOverlay::OnSceneChanged(const QList<QRectF>&)
{
	// skip rendering if the overlay isn't visible
	if (!vr::VROverlay() || !vr::VROverlay()->IsOverlayVisible(m_ulOverlayHandle))
		return;

	//Rendering with automatic resizing doesn't work well with a fixed buffer size,
	//but creating and deleting the buffer breaks everything

	m_pOpenGLContext->makeCurrent(m_pOffscreenSurface);
	//m_pFbo = new QOpenGLFramebufferObject(m_pWidget->width(), m_pWidget->height(), GL_TEXTURE_2D);
	m_pFbo->bind();

	QOpenGLPaintDevice device(m_pFbo->size());
	QPainter painter(&device);
	
	m_pScene->render(&painter);

	m_pFbo->release();

	GLuint unTexture = m_pFbo->texture();
	if (unTexture != 0)
	{
		vr::Texture_t texture = { (void*)(uintptr_t)unTexture, vr::TextureType_OpenGL, vr::ColorSpace_Auto };
		vr::VROverlay()->SetOverlayTexture(m_ulOverlayHandle, &texture);
	}
}

void VRFloatingOverlay::OnTimeoutPumpEvents()
{
	if (!vr::VRSystem())
		return;

	vr::VREvent_t vrEvent;
	while (vr::VROverlay()->PollNextOverlayEvent(m_ulOverlayHandle, &vrEvent, sizeof(vrEvent)))
	{
		switch (vrEvent.eventType)
		{
		case vr::VREvent_OverlayShown:
		{
			m_pWidget->repaint();
		}
		break;

		case vr::VREvent_Quit:
			//QApplication::exit();
			break;
		}
	}

	if (vr::VRCompositor() != NULL) {
		vr::Compositor_FrameTiming t;
		t.m_nSize = sizeof(vr::Compositor_FrameTiming);
		bool hasFrame = vr::VRCompositor()->GetFrameTiming(&t, 0);
		// If the frame has changed we update, if a frame was redisplayed we update.
		if ((hasFrame && currentFrame != t.m_nFrameIndex) || (hasFrame && t.m_nNumFramePresents != numFramePresents)) {
			currentFrame = t.m_nFrameIndex;
			numFramePresents = t.m_nNumFramePresents;
			UpdateTextUI();
			UpdateTransform();
		}
	}
	else if (vr::VROverlay()->IsOverlayVisible(m_ulOverlayHandle)) {
			{
		UpdateTextUI();
		UpdateTransform();
	}
}


void VRFloatingOverlay::SetWidget(QWidget* pWidget)
{
	if (m_pScene)
	{
		// all of the mouse handling stuff requires that the widget be at 0,0
		pWidget->move(0, 0);
		m_pScene->addWidget(pWidget);
	}
	m_pWidget = pWidget;

	m_pFbo = new QOpenGLFramebufferObject(m_pWidget->width(), m_pWidget->height(), GL_TEXTURE_2D);
}