

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


//Idea, can I define functions for this class in the main program?
//If so, I can use that to make callbacks
//If not, interfaces (that's also a good/better idea)


VRFloatingOverlay* s_pSharedVRController = NULL;


VRFloatingOverlay* VRFloatingOverlay::SharedInstance()
{
	if (!s_pSharedVRController)
	{
		s_pSharedVRController = new VRFloatingOverlay();
	}
	return s_pSharedVRController;
}


void VRFloatingOverlay::SetCameraState(QString text) {
	cameraStateQueue.push(text);
}
void VRFloatingOverlay::UpdateCameraStateUI() { //turn into timeout based system
	while (!cameraStateQueue.empty()) {
		QString text = cameraStateQueue.front();
		cameraStateQueue.pop();
		((OverlayWidget*)m_pWidget)->SetText(text);
	}
}
void VRFloatingOverlay::ReturnCameraScreenshot(uint8_t index, uint8_t* data[]) {

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

	std::cout << "Set Widget\n";
	SetWidget(pOverlayWidget);

	//vr::VROverlay()->ShowOverlay(m_ulOverlayHandle);

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

	QSurfaceFormat format;
	format.setMajorVersion(4);
	format.setMinorVersion(1);
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
		vr::VROverlay()->SetOverlayWidthInMeters(m_ulOverlayHandle, .25f);
		vr::HmdMatrix34_t transform;
		glm::vec3 offset(0, -.125f, -.25f);
		glm::vec3 front = -glm::normalize(offset);
		glm::vec3 left(1, 0, 0);
		glm::vec3 up = glm::cross(front, left);
		transform.m[0][0] = left.x;
		transform.m[1][0] = left.y;
		transform.m[2][0] = left.z;
		transform.m[0][1] = up.x;
		transform.m[1][1] = up.y;
		transform.m[2][1] = up.z;
		transform.m[0][2] = front.x;
		transform.m[1][2] = front.y;
		transform.m[2][2] = front.z;
		transform.m[0][3] = offset.x;
		transform.m[1][3] = offset.y;
		transform.m[2][3] = offset.z;

		vr::VROverlay()->SetOverlayTransformTrackedDeviceRelative(m_ulOverlayHandle, vr::k_unTrackedDeviceIndex_Hmd, &transform);

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

	m_pOpenGLContext->makeCurrent(m_pOffscreenSurface);
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
			//UpdateCameraStateUI();
			m_pWidget->repaint();
		}
		break;

		case vr::VREvent_Quit:
			//QApplication::exit();
			break;
		}
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

	m_pFbo = new QOpenGLFramebufferObject(pWidget->width(), pWidget->height(), GL_TEXTURE_2D);


	if (vr::VROverlay())
	{
		vr::HmdVector2_t vecWindowSize =
		{
			(float)pWidget->width(),
			(float)pWidget->height()
		};
		vr::VROverlay()->SetOverlayMouseScale(m_ulOverlayHandle, &vecWindowSize);
	}

}