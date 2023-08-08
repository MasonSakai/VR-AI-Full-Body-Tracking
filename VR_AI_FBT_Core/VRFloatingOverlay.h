#pragma once

#include "openvr.h"
#include "vrUtil.h"
#include "OverlayManager.h"
#include "OverlayWidget.h"

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

struct FloatingOverlayText {
	QString text;
	float duration = .5f;
};

class VRFloatingOverlay : public QObject
{
	Q_OBJECT
		typedef QObject BaseClass;

public:
	static VRFloatingOverlay* SharedInstance();

public:
	VRFloatingOverlay();
	virtual ~VRFloatingOverlay();

	bool Init();
	void Shutdown();

	QString GetName() { return m_strName; }

	void SetWidget(QWidget* pWidget);


	void QueueText(QString text, float duration);

public slots:
	void OnSceneChanged(const QList<QRectF>&);
	void OnTimeoutPumpEvents();

protected:

private:

	void UpdateTextUI();

	vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	QString m_strVRDriver;
	QString m_strVRDisplay;
	QString m_strName;

	vr::HmdError m_eLastHmdError;

	vr::HmdError m_eCompositorError;
	vr::HmdError m_eOverlayError;
	vr::Compositor_OverlaySettings m_overlaySettings;
	vr::VROverlayHandle_t m_ulOverlayHandle;

	QOpenGLContext* m_pOpenGLContext;
	QGraphicsScene* m_pScene;
	QOpenGLFramebufferObject* m_pFbo;
	QOffscreenSurface* m_pOffscreenSurface;

	QTimer* m_pPumpEventsTimer;

	std::queue<FloatingOverlayText> textQueue;

	// the widget we're drawing into the texture
	QWidget* m_pWidget;

	QPointF m_ptLastMouse;
	Qt::MouseButtons m_lastMouseButtons;
	bool m_bManualMouseHandling;
};