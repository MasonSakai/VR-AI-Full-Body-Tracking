#pragma once

#include "openvr.h"
#include "vrUtil.h"
#include "OverlayManager.h"
#include "DashboardWidget.h"

#include <queue>

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

enum CameraState;

class DashboardCameraState {
public:
	uint8_t index;
	CameraState state;
	DashboardCameraState(uint8_t index, CameraState state);
};

class VRDashboardOverlay : public QObject
{
	Q_OBJECT
		typedef QObject BaseClass;

public:
	static VRDashboardOverlay* SharedInstance();

public:
	VRDashboardOverlay();
	virtual ~VRDashboardOverlay();

	bool Init();
	void Shutdown();

	QString GetName() { return m_strName; }

	void SetWidget(QWidget* pWidget);


	void SetCameraState(uint8_t index, CameraState state);

	void UpdateTrackersSeen();

public slots:
	void OnSceneChanged(const QList<QRectF>&);
	void OnTimeoutPumpEvents();

protected:

private:

	void UpdateCameraStateUI();

	vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	QString m_strVRDriver;
	QString m_strVRDisplay;
	QString m_strName;

	vr::HmdError m_eLastHmdError;

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

	std::queue<DashboardCameraState> cameraStateQueue;

	// the widget we're drawing into the texture
	QWidget* m_pWidget;

	QPointF m_ptLastMouse;
	Qt::MouseButtons m_lastMouseButtons;
	bool m_bManualMouseHandling;
};