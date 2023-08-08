

#include "VRDashboardOverlay.h"

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


VRDashboardOverlay* s_pSharedVRController = NULL;


VRDashboardOverlay* VRDashboardOverlay::SharedInstance()
{
	if (!s_pSharedVRController)
	{
		s_pSharedVRController = new VRDashboardOverlay();
	}
	return s_pSharedVRController;
}


void VRDashboardOverlay::SetCameraState(uint8_t index, CameraState state) {
	cameraStateQueue.push(DashboardCameraState(index, state));
}
void VRDashboardOverlay::UpdateCameraStateUI() {
	uint8_t index;
	CameraState state;
	while (!cameraStateQueue.empty()) {
		index = cameraStateQueue.front().index;
		state = cameraStateQueue.front().state;
		cameraStateQueue.pop();
		((DashboardWidget*)m_pWidget)->SetCameraState(index, state);
	}
}

void VRDashboardOverlay::UpdateTrackersSeen() {
	DashboardWidget* widget = (DashboardWidget*)m_pWidget;
	for (int i = 5; i < 17; i++) {
		QString name = "lbl_";
		name.append(PoseNames[i]);
		widget->SetLabel(name, QString::number(trackers[i].getNumberOfCams()));
	}
}


VRDashboardOverlay::VRDashboardOverlay()
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
	DashboardWidget* pOverlayWidget = new DashboardWidget();

	std::cout << "Init\n";
	Init();

	std::cout << "Set Widget\n";
	SetWidget(pOverlayWidget);

	std::cout << "Started\n";
}


VRDashboardOverlay::~VRDashboardOverlay()
{
}


//-----------------------------------------------------------------------------
// Purpose: Helper to get a string from a tracked device property and turn it
//			into a QString
//-----------------------------------------------------------------------------
QString GetTrackedDeviceString(vr::IVRSystem* pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop)
{
	char buf[128];
	vr::TrackedPropertyError err;
	pHmd->GetStringTrackedDeviceProperty(unDevice, prop, buf, sizeof(buf), &err);
	if (err != vr::TrackedProp_Success)
	{
		return QString("Error Getting String: ") + pHmd->GetPropErrorNameFromEnum(err);
	}
	else
	{
		return buf;
	}
}


bool VRDashboardOverlay::Init()
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

	// Loading the OpenVR Runtime
	m_strVRDriver = GetTrackedDeviceString(m_VRSystem, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
	m_strVRDisplay = GetTrackedDeviceString(m_VRSystem, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);

	bSuccess = vr::VRCompositor() != NULL;

	if (vr::VROverlay())
	{
		vr::VROverlayError overlayError = vr::VROverlay()->CreateDashboardOverlay(DashboardKey, OverlayName, &m_ulOverlayHandle, &m_ulOverlayThumbnailHandle);
		bSuccess = bSuccess && overlayError == vr::VROverlayError_None;
	}

	if (bSuccess)
	{
		vr::VROverlay()->SetOverlayWidthInMeters(m_ulOverlayHandle, 2.5f);// 1.5f);
		vr::VROverlay()->SetOverlayInputMethod(m_ulOverlayHandle, vr::VROverlayInputMethod_Mouse);

		m_pPumpEventsTimer = new QTimer(this);
		connect(m_pPumpEventsTimer, SIGNAL(timeout()), this, SLOT(OnTimeoutPumpEvents()));
		m_pPumpEventsTimer->setInterval(20);
		m_pPumpEventsTimer->start();

	}
	return true;
}
void VRDashboardOverlay::Shutdown()
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


void VRDashboardOverlay::OnSceneChanged(const QList<QRectF>&)
{
	// skip rendering if the overlay isn't visible
	if (!vr::VROverlay() ||
		(!vr::VROverlay()->IsOverlayVisible(m_ulOverlayHandle) && !vr::VROverlay()->IsOverlayVisible(m_ulOverlayThumbnailHandle)))
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

void VRDashboardOverlay::OnTimeoutPumpEvents()
{
	if (!vr::VRSystem())
		return;


	if (m_bManualMouseHandling)
	{
		// tell OpenVR to make some events for us
		for (vr::TrackedDeviceIndex_t unDeviceId = 1; unDeviceId < vr::k_unControllerStateAxisCount; unDeviceId++)
		{
			if (vr::VROverlay()->HandleControllerOverlayInteractionAsMouse(m_ulOverlayHandle, unDeviceId))
			{
				break;
			}
		}
	}

	vr::VREvent_t vrEvent;
	while (vr::VROverlay()->PollNextOverlayEvent(m_ulOverlayHandle, &vrEvent, sizeof(vrEvent)))
	{
		switch (vrEvent.eventType)
		{
		case vr::VREvent_MouseMove:
		{
			QPointF ptNewMouse(vrEvent.data.mouse.x, vrEvent.data.mouse.y);
			QPoint ptGlobal = ptNewMouse.toPoint();
			QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseMove);
			mouseEvent.setWidget(NULL);
			mouseEvent.setPos(ptNewMouse);
			mouseEvent.setScenePos(ptGlobal);
			mouseEvent.setScreenPos(ptGlobal);
			mouseEvent.setLastPos(m_ptLastMouse);
			mouseEvent.setLastScenePos(m_pWidget->mapToGlobal(m_ptLastMouse.toPoint()));
			mouseEvent.setLastScreenPos(m_pWidget->mapToGlobal(m_ptLastMouse.toPoint()));
			mouseEvent.setButtons(m_lastMouseButtons);
			mouseEvent.setButton(Qt::NoButton);
			mouseEvent.setModifiers(Qt::KeyboardModifiers::fromInt(0));
			mouseEvent.setAccepted(false);

			m_ptLastMouse = ptNewMouse;
			QApplication::sendEvent(m_pScene, &mouseEvent);

			OnSceneChanged(QList<QRectF>());
		}
		break;

		case vr::VREvent_MouseButtonDown:
		{
			Qt::MouseButton button = vrEvent.data.mouse.button == vr::VRMouseButton_Right ? Qt::RightButton : Qt::LeftButton;

			m_lastMouseButtons |= button;

			QPoint ptGlobal = m_ptLastMouse.toPoint();
			QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMousePress);
			mouseEvent.setWidget(NULL);
			mouseEvent.setPos(m_ptLastMouse);
			mouseEvent.setButtonDownPos(button, m_ptLastMouse);
			mouseEvent.setButtonDownScenePos(button, ptGlobal);
			mouseEvent.setButtonDownScreenPos(button, ptGlobal);
			mouseEvent.setScenePos(ptGlobal);
			mouseEvent.setScreenPos(ptGlobal);
			mouseEvent.setLastPos(m_ptLastMouse);
			mouseEvent.setLastScenePos(ptGlobal);
			mouseEvent.setLastScreenPos(ptGlobal);
			mouseEvent.setButtons(m_lastMouseButtons);
			mouseEvent.setButton(button);
			mouseEvent.setModifiers(Qt::KeyboardModifiers::fromInt(0));
			mouseEvent.setAccepted(false);

			QApplication::sendEvent(m_pScene, &mouseEvent);
		}
		break;

		case vr::VREvent_MouseButtonUp:
		{
			Qt::MouseButton button = vrEvent.data.mouse.button == vr::VRMouseButton_Right ? Qt::RightButton : Qt::LeftButton;
			m_lastMouseButtons &= ~button;

			QPoint ptGlobal = m_ptLastMouse.toPoint();
			QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseRelease);
			mouseEvent.setWidget(NULL);
			mouseEvent.setPos(m_ptLastMouse);
			mouseEvent.setScenePos(ptGlobal);
			mouseEvent.setScreenPos(ptGlobal);
			mouseEvent.setLastPos(m_ptLastMouse);
			mouseEvent.setLastScenePos(ptGlobal);
			mouseEvent.setLastScreenPos(ptGlobal);
			mouseEvent.setButtons(m_lastMouseButtons);
			mouseEvent.setButton(button);
			mouseEvent.setModifiers(Qt::KeyboardModifiers::fromInt(0));
			mouseEvent.setAccepted(false);

			QApplication::sendEvent(m_pScene, &mouseEvent);
		}
		break;
		
		case vr::VREvent_OverlayShown:
		{
			m_pWidget->repaint();
		}
		break;

		case vr::VREvent_Quit:
			QApplication::exit();
			break;
		}
	}
	if (vr::VROverlay()->IsOverlayVisible(m_ulOverlayHandle)) {
		UpdateCameraStateUI();
		OnSceneChanged(QList<QRectF>());
	}

	if (m_ulOverlayThumbnailHandle != vr::k_ulOverlayHandleInvalid)
	{
		while (vr::VROverlay()->PollNextOverlayEvent(m_ulOverlayThumbnailHandle, &vrEvent, sizeof(vrEvent)))
		{
			switch (vrEvent.eventType)
			{
			case vr::VREvent_OverlayShown:
			{
				m_pWidget->repaint();
			}
			break;
			}
		}
	}

}


void VRDashboardOverlay::SetWidget(QWidget* pWidget)
{
	if (m_pScene)
	{
		// all of the mouse handling stuff requires that the widget be at 0,0
		pWidget->move(0, 0);
		m_pScene->addWidget(pWidget);
	}
	m_pWidget = pWidget;

	m_pFbo = new QOpenGLFramebufferObject(pWidget->width(), pWidget->height(), GL_TEXTURE_2D); // Error Here
	//Exception thrown: read access violation. QGuiApplicationPrivate::platform_integration was nullptr.


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


DashboardCameraState::DashboardCameraState(uint8_t index, CameraState state) {
	this->index = index;
	this->state = state;
}