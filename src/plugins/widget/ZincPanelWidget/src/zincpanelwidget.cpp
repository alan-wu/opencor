/*******************************************************************************

Copyright (C) The University of Auckland

OpenCOR is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

OpenCOR is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*******************************************************************************/

//==============================================================================
// Zinc widget
//==============================================================================

#include "coreguiutils.h"
#include "zincpanelwidget.h"

#include <QMouseEvent>
#include <QOpenGLContext>
#include <QTimer>
#include <QHBoxLayout>


//==============================================================================

#include "opencmiss/zinc/context.hpp"
#include "opencmiss/zinc/result.hpp"

//==============================================================================

namespace OpenCOR {
namespace ZincPanelWidget {

//==============================================================================

ZincPanelWidgetSceneViewerCallback::ZincPanelWidgetSceneViewerCallback(ZincPanelWidget *pZincPanelWidget) :
    mZincPanelWidget(pZincPanelWidget)
{
}

//==============================================================================

void ZincPanelWidgetSceneViewerCallback::operator()(const OpenCMISS::Zinc::Sceneviewerevent &pSceneViewerEvent)
{
    // Ask our Zinc widget to update itself if repainting is required

    if (pSceneViewerEvent.getChangeFlags() & OpenCMISS::Zinc::Sceneviewerevent::CHANGE_FLAG_REPAINT_REQUIRED)
        QTimer::singleShot(0, mZincPanelWidget, SLOT(update()));
}

//==============================================================================

ZincPanelWidget::ZincPanelWidget(const ZincPanelWidgets &pNeighbors,
        QWidget *pParent) :
    QOpenGLWidget(pParent),
    Core::CommonWidget(this),
    mGraphicsInitialized(false),
    mDevicePixelRatio(-1),
    mContext(0),
    mSceneViewer(OpenCMISS::Zinc::Sceneviewer()),
    mSceneViewerNotifier(OpenCMISS::Zinc::Sceneviewernotifier()),
    mZincPanelWidgetSceneViewerCallback(this)
{
    // We are not active by default

    mActive = false;

    // Create and set our horizontal layout

    QHBoxLayout *layout = new QHBoxLayout(this);

    layout->setContentsMargins(QMargins());
    layout->setSpacing(0);

    setLayout(layout);

    // Create, customise and add an inactive marker to our layout

    enum {
        MarkerWidth = 3
    };

    mMarker = new QFrame(this);

    mMarker->setFrameShape(QFrame::VLine);
    mMarker->setLineWidth(MarkerWidth);
    mMarker->setMinimumWidth(MarkerWidth);

    setActive(false);

    layout->addWidget(mMarker);


    // Allow the graph panel to be of any vertical size

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);

    // Make sure that our marker's colour is properly intiialised

    updateMarkerColor();

    // Let our plot's neighbours know about our plot

    //foreach (ZincPanelPlotWidget *neighbor, neighbors)
     //   neighbor->addNeighbor(mPlot);
}

//==============================================================================

OpenCMISS::Zinc::Context * ZincPanelWidget::context() const
{
    // Return our context

    return mContext;
}

//==============================================================================

void ZincPanelWidget::setContext(OpenCMISS::Zinc::Context *pContext)
{
    // Set our context

    mContext = pContext;

    if (mGraphicsInitialized)
        createSceneViewer();
}

//==============================================================================

OpenCMISS::Zinc::Sceneviewer ZincPanelWidget::sceneViewer() const
{
    // Return our scene viewer

    return mSceneViewer;
}

//==============================================================================

ZincPanelWidget::ProjectionMode ZincPanelWidget::projectionMode()
{
    // Return our projection mode

    switch (mSceneViewer.getProjectionMode()) {
    case OpenCMISS::Zinc::Sceneviewer::PROJECTION_MODE_PARALLEL:
        return Parallel;
    case OpenCMISS::Zinc::Sceneviewer::PROJECTION_MODE_PERSPECTIVE:
        return Perspective;
    default:
        return Invalid;
    }
}

//==============================================================================

void ZincPanelWidget::setProjectionMode(const ProjectionMode &pProjectionMode)
{
    // Set our projection mode

    switch (pProjectionMode) {
    case Parallel:
        mSceneViewer.setProjectionMode(OpenCMISS::Zinc::Sceneviewer::PROJECTION_MODE_PARALLEL);

        break;
    case Perspective:
        mSceneViewer.setProjectionMode(OpenCMISS::Zinc::Sceneviewer::PROJECTION_MODE_PERSPECTIVE);

        break;
    default:
        // Invalid, so do nothing

        ;
    }
}

//==============================================================================

int ZincPanelWidget::viewParameters(double *pEye, double *pLookAt, double *pUp,
                               double &pAngle)
{
    // Return our view parameters

    int res = mSceneViewer.getLookatParameters(pEye, pLookAt, pUp);

    if (res == OpenCMISS::Zinc::Result::RESULT_OK)
        pAngle = mSceneViewer.getViewAngle();

    return res;
}

//==============================================================================

void ZincPanelWidget::setViewParameters(double *pEye, double *pLookAt, double *pUp,
                                   double &pAngle)
{
    // Set our view parameters

    mSceneViewer.beginChange();
        mSceneViewer.setLookatParametersNonSkew(pEye, pLookAt, pUp);
        mSceneViewer.setViewAngle(pAngle);
    mSceneViewer.endChange();
}

//==============================================================================

OpenCMISS::Zinc::Scenefilter ZincPanelWidget::sceneFilter()
{
    // Return our scene filter

    return mSceneViewer.getScenefilter();
}

//==============================================================================

void ZincPanelWidget::setSceneFilter(const OpenCMISS::Zinc::Scenefilter &pSceneFilter)
{
    // Set our scene filter

    mSceneViewer.setScenefilter(pSceneFilter);
}

//==============================================================================

double ZincPanelWidget::tumbleRate()
{
    // Return our tumble rate

    return mSceneViewer.getTumbleRate();
}

//==============================================================================

void ZincPanelWidget::setTumbleRate(const double &pTumbleRate)
{
    // Set our tumble rate

    mSceneViewer.setTumbleRate(pTumbleRate);
}

//==============================================================================

int ZincPanelWidget::project(double *pInCoordinates, double *pOutCoordinates)
{
    // Project the given point in global coordinates into window pixel
    // coordinates with the origin at the window's top left pixel
    // Note: the Z pixel coordinate is a depth, which is mapped so that -1 is on
    //       the far clipping plane and +1 is on the near clipping plane...

    return mSceneViewer.transformCoordinates(OpenCMISS::Zinc::SCENECOORDINATESYSTEM_WORLD,
                                             OpenCMISS::Zinc::SCENECOORDINATESYSTEM_WINDOW_PIXEL_TOP_LEFT,
                                             OpenCMISS::Zinc::Scene(), pInCoordinates, pOutCoordinates);
}

//==============================================================================

int ZincPanelWidget::unproject(double *pInCoordinates, double *pOutCoordinates)
{
    // Unproject the given point in window pixel coordinates where the origin is
    // at the window's top left pixel into global coordinates
    // Note: the Z pixel coordinate is a depth, which is mapped so that -1 is on
    //       the far clipping plane, and +1 is on the near clipping plane...

    return mSceneViewer.transformCoordinates(OpenCMISS::Zinc::SCENECOORDINATESYSTEM_WINDOW_PIXEL_TOP_LEFT,
                                             OpenCMISS::Zinc::SCENECOORDINATESYSTEM_WORLD,
                                             OpenCMISS::Zinc::Scene(), pInCoordinates, pOutCoordinates);
}

//==============================================================================

void ZincPanelWidget::viewAll()
{
    // View all of our scene viewer

    mSceneViewer.viewAll();
}

//==============================================================================

void ZincPanelWidget::createSceneViewer()
{
    // Create our scene viewer and have it have the same OpenGL properties as
    // QOpenGLWidget

    mSceneViewer = mContext->getSceneviewermodule().createSceneviewer(OpenCMISS::Zinc::Sceneviewer::BUFFERING_MODE_DOUBLE,
                                                                      OpenCMISS::Zinc::Sceneviewer::STEREO_MODE_DEFAULT);

    mSceneViewer.setProjectionMode(OpenCMISS::Zinc::Sceneviewer::PROJECTION_MODE_PERSPECTIVE);
    mSceneViewer.setViewportSize(width(), height());

    // Further customise our scene viewer

    mSceneViewer.setScene(mContext->getDefaultRegion().getScene());
    mSceneViewer.setScenefilter(mContext->getScenefiltermodule().getDefaultScenefilter());

    // Get ourselves a scene viewer notifier and set its callback to our
    // callback class

    mSceneViewerNotifier = mSceneViewer.createSceneviewernotifier();

    mSceneViewerNotifier.setCallback(mZincPanelWidgetSceneViewerCallback);

    // We are all set, so view all of our scene viewer

    mSceneViewer.viewAll();
}

//==============================================================================

void ZincPanelWidget::updateSceneViewerViewerportSize(const int &pWidth,
                                                 const int &pHeight,
                                                 const bool &pCheckDevicePixelRatio)
{
    // Update the viewport size of our scene viewer, keeping in mind our device
    // pixel ratio

    int newDevicePixelRatio = devicePixelRatio();

    if (pCheckDevicePixelRatio) {
        if (newDevicePixelRatio == mDevicePixelRatio)
            return;
        else
            emit devicePixelRatioChanged(newDevicePixelRatio);
    }

    mDevicePixelRatio = newDevicePixelRatio;

    mSceneViewer.setViewportSize(newDevicePixelRatio*pWidth, newDevicePixelRatio*pHeight);
}

//==============================================================================

void ZincPanelWidget::initializeGL()
{
    // Forward the fact that our context is going to be destroyed

    connect(QOpenGLWidget::context(), SIGNAL(aboutToBeDestroyed()),
            this, SIGNAL(contextAboutToBeDestroyed()));

    // Create our scene viewer if we have a context

    mGraphicsInitialized = true;

    if (mContext)
        createSceneViewer();

    emit graphicsInitialized();
}

//==============================================================================

void ZincPanelWidget::paintGL()
{
    // Have our scene viewer render its scene

    updateSceneViewerViewerportSize(width(), height(), true);

    mSceneViewer.renderScene();
}

//==============================================================================

void ZincPanelWidget::resizeGL(int pWidth, int pHeight)
{
    // Update the viewport size of our scene viewer

    updateSceneViewerViewerportSize(pWidth, pHeight);
}

//==============================================================================

OpenCMISS::Zinc::Sceneviewerinput::ButtonType ZincPanelWidget::buttonMap(const Qt::MouseButton &pButton) const
{
    // Map the given button to a Zinc button

    switch (pButton) {
    case Qt::LeftButton:
        return OpenCMISS::Zinc::Sceneviewerinput::BUTTON_TYPE_LEFT;
    case Qt::MiddleButton:
        return OpenCMISS::Zinc::Sceneviewerinput::BUTTON_TYPE_MIDDLE;
    case Qt::XButton1:
        return OpenCMISS::Zinc::Sceneviewerinput::BUTTON_TYPE_SCROLL_DOWN;
    case Qt::XButton2:
            return OpenCMISS::Zinc::Sceneviewerinput::BUTTON_TYPE_SCROLL_UP;
    case Qt::RightButton:
        return OpenCMISS::Zinc::Sceneviewerinput::BUTTON_TYPE_RIGHT;
    default:
        return OpenCMISS::Zinc::Sceneviewerinput::BUTTON_TYPE_INVALID;
    }
}

//==============================================================================

OpenCMISS::Zinc::Sceneviewerinput::ModifierFlags ZincPanelWidget::modifierMap(const Qt::KeyboardModifiers &pModifiers) const
{
    // Map the given modifiers to Zinc modifiers

    OpenCMISS::Zinc::Sceneviewerinput::ModifierFlags res = OpenCMISS::Zinc::Sceneviewerinput::MODIFIER_FLAG_NONE;

    if (pModifiers & Qt::ShiftModifier)
        res = res|OpenCMISS::Zinc::Sceneviewerinput::MODIFIER_FLAG_SHIFT;

    if (pModifiers & Qt::ControlModifier)
        res = res|OpenCMISS::Zinc::Sceneviewerinput::MODIFIER_FLAG_CONTROL;

    if (pModifiers & Qt::AltModifier)
        res = res|OpenCMISS::Zinc::Sceneviewerinput::MODIFIER_FLAG_ALT;

    return res;
}

//==============================================================================

void ZincPanelWidget::mouseMoveEvent(QMouseEvent *pEvent)
{
    // Get our scene viewer to handle the given mouse move event

    OpenCMISS::Zinc::Sceneviewerinput sceneInput = mSceneViewer.createSceneviewerinput();

    sceneInput.setEventType(OpenCMISS::Zinc::Sceneviewerinput::EVENT_TYPE_MOTION_NOTIFY);

    if (pEvent->type() == QEvent::Leave)
        sceneInput.setPosition(-1, -1);
    else
        sceneInput.setPosition(pEvent->x(), pEvent->y());

    mSceneViewer.processSceneviewerinput(sceneInput);
}

//==============================================================================

void ZincPanelWidget::mousePressEvent(QMouseEvent *pEvent)
{
    // Get our scene viewer to handle the given mouse press event

    OpenCMISS::Zinc::Sceneviewerinput sceneInput = mSceneViewer.createSceneviewerinput();

    sceneInput.setButtonType(buttonMap(pEvent->button()));
    sceneInput.setEventType(OpenCMISS::Zinc::Sceneviewerinput::EVENT_TYPE_BUTTON_PRESS);
    sceneInput.setModifierFlags(modifierMap(pEvent->modifiers()));
    sceneInput.setPosition(pEvent->x(), pEvent->y());

    mSceneViewer.processSceneviewerinput(sceneInput);
}

//==============================================================================

void ZincPanelWidget::mouseReleaseEvent(QMouseEvent *pEvent)
{
    // Get our scene viewer to handle the given mouse release event

    OpenCMISS::Zinc::Sceneviewerinput sceneInput = mSceneViewer.createSceneviewerinput();

    sceneInput.setButtonType(buttonMap(pEvent->button()));
    sceneInput.setEventType(OpenCMISS::Zinc::Sceneviewerinput::EVENT_TYPE_BUTTON_RELEASE);
    sceneInput.setPosition(pEvent->x(), pEvent->y());

    mSceneViewer.processSceneviewerinput(sceneInput);
}

//==============================================================================

void ZincPanelWidget::wheelEvent(QWheelEvent *pEvent)
{
    // Get our scene viewer to handle the given wheel event by first pretending
    // to press the right mouse button

    OpenCMISS::Zinc::Sceneviewerinput sceneInput = mSceneViewer.createSceneviewerinput();

    sceneInput.setButtonType(OpenCMISS::Zinc::Sceneviewerinput::BUTTON_TYPE_RIGHT);
    sceneInput.setEventType(OpenCMISS::Zinc::Sceneviewerinput::EVENT_TYPE_BUTTON_PRESS);
    sceneInput.setPosition(pEvent->x(), pEvent->y());

    mSceneViewer.processSceneviewerinput(sceneInput);

    // Next, pretend to move the right mouse button

    sceneInput = mSceneViewer.createSceneviewerinput();

    sceneInput.setEventType(OpenCMISS::Zinc::Sceneviewerinput::EVENT_TYPE_MOTION_NOTIFY);
    sceneInput.setPosition(pEvent->x(), pEvent->y()-0.1*pEvent->delta());

    mSceneViewer.processSceneviewerinput(sceneInput);

    // Finally, pretend to release the right mouse button

    sceneInput = mSceneViewer.createSceneviewerinput();

    sceneInput.setButtonType(OpenCMISS::Zinc::Sceneviewerinput::BUTTON_TYPE_RIGHT);
    sceneInput.setEventType(OpenCMISS::Zinc::Sceneviewerinput::EVENT_TYPE_BUTTON_RELEASE);
    sceneInput.setPosition(pEvent->x(), pEvent->y());

    mSceneViewer.processSceneviewerinput(sceneInput);
}

//==============================================================================

QSize ZincPanelWidget::sizeHint() const
{
    // Suggest a default size for ourselves
    // Note: this is critical if we want a docked widget, with ourselves on it,
    //       to have a decent size when docked to the main window...

    return defaultSize(0.15);
}

void ZincPanelWidget::updateMarkerColor()
{
    // Update the marker's colour based on whether the graph panel is active

    mMarker->setStyleSheet("QFrame {"
                           "    color: "+QString(mActive?Core::highlightColor().name():Core::windowColor().name())+";"
                           "}");
}


void ZincPanelWidget::setActive(const bool &pActive)
{
    if (pActive == mActive)
        return;

    // Set the zinc panel's active state

    mActive = pActive;

    // Update the marker's colour

    updateMarkerColor();

    // Let people know if the graph panel has been activated or inactivated

    if (pActive)
        emit activated(this);
    else
        emit inactivated(this);
}

//==============================================================================

}   // namespace WebViewerWidget
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
