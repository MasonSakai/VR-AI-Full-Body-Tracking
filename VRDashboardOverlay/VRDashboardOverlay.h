#pragma once

#include "openvr.h"
#include "DashboardListener.h"

#include <QtCore/QtCore>
// because of incompatibilities with QtOpenGL and GLEW we need to cherry pick includes
#include <QVector2D>
#include <QMatrix4x4>
#include <QVector>
#include <QVector2D>
#include <QVector3D>
#include <QtGui/QOpenGLContext>
#include <QtWidgets/QGraphicsScene>
#include <QOffscreenSurface>
class QOpenGLFramebufferObject;

class VRDashboardOverlay : public QObject
{
	Q_OBJECT
		typedef QObject BaseClass;

public:
	static VRDashboardOverlay* SharedInstance();
	static VRDashboardOverlay* CreateSharedInstance(IDashboardListener* dashboardListener, int argc = 0, char* argv[] = NULL);

public:
	VRDashboardOverlay(int argc = 0, char* argv[] = NULL);
	virtual ~VRDashboardOverlay();

	bool Init();
	void Shutdown();
	void EnableRestart();

	bool BHMDAvailable();
	vr::IVRSystem* GetVRSystem();
	vr::HmdError GetLastHmdError();

	QString GetVRDriverString();
	QString GetVRDisplayString();
	QString GetName() { return m_strName; }

	void SetWidget(QWidget* pWidget);


	void SetCameraState(uint8_t index, uint8_t state);
	void ReturnCameraScreenshot(uint8_t index, uint8_t* data[]);

public slots:
	void OnSceneChanged(const QList<QRectF>&);
	void OnTimeoutPumpEvents();

protected:

private:
	bool ConnectToVRRuntime();
	void DisconnectFromVRRuntime();

	vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	QString m_strVRDriver;
	QString m_strVRDisplay;
	QString m_strName;

	vr::HmdError m_eLastHmdError;

private:
	vr::HmdError m_eCompositorError;
	vr::HmdError m_eOverlayError;
	vr::Compositor_OverlaySettings m_overlaySettings;
	vr::VROverlayHandle_t m_ulOverlayHandle;
	vr::VROverlayHandle_t m_ulOverlayThumbnailHandle;

	QOpenGLContext* m_pOpenGLContext;
	QGraphicsScene* m_pScene;
	QOpenGLFramebufferObject* m_pFbo;
	QOffscreenSurface* m_pOffscreenSurface;

	QTimer* m_pPumpEventsTimer;

	// the widget we're drawing into the texture
	QWidget* m_pWidget;

	QPointF m_ptLastMouse;
	Qt::MouseButtons m_lastMouseButtons;
	bool m_bManualMouseHandling;
};