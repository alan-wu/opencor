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

#pragma once

//==============================================================================

#include "commonwidget.h"
#include "zincpanelwidgetglobal.h"
#include "widget.h"

//==============================================================================

#include <QFrame>
#include <QList>
#include <QOpenGLWidget>

//==============================================================================

#include "opencmiss/zinc/sceneviewer.hpp"
#include "opencmiss/zinc/sceneviewerinput.hpp"

//==============================================================================

namespace OpenCMISS {
namespace Zinc {
    class Context;
}   // namespace Zinc
}   // namespace OpenCMISS

//==============================================================================

namespace OpenCOR {
namespace ZincPanelWidget {

//==============================================================================

class ZincPanelWidget;

//==============================================================================

typedef QList<ZincPanelWidget *> ZincPanelWidgets;

//==============================================================================

class ZincPanelWidgetSceneViewerCallback : public OpenCMISS::Zinc::Sceneviewercallback
{
public:
    explicit ZincPanelWidgetSceneViewerCallback(ZincPanelWidget *pZincPanelWidget);

private:
    ZincPanelWidget *mZincPanelWidget;

    virtual void operator()(const OpenCMISS::Zinc::Sceneviewerevent &pSceneViewerEvent);
};

//==============================================================================

class ZINCPANELWIDGET_EXPORT ZincPanelWidget : public QOpenGLWidget,
                                     public Core::CommonWidget
{
    Q_OBJECT

public:
    enum ProjectionMode {
        Invalid,
        Parallel,
        Perspective
    };

    explicit ZincPanelWidget(const ZincPanelWidgets &pNeighbors,
                              QWidget *pParent);

    OpenCMISS::Zinc::Context * context() const;
    void setContext(OpenCMISS::Zinc::Context *pContext);

    OpenCMISS::Zinc::Sceneviewer sceneViewer() const;

    ProjectionMode projectionMode();
    void setProjectionMode(const ProjectionMode &pProjectionMode);

    int viewParameters(double *pEye, double *pLookAt, double *pUp,
                       double &pAngle);
    void setViewParameters(double *pEye, double *pLookAt, double *pUp,
                           double &pAngle);

    OpenCMISS::Zinc::Scenefilter sceneFilter();
    void setSceneFilter(const OpenCMISS::Zinc::Scenefilter &pSceneFilter);

    double tumbleRate();
    void setTumbleRate(const double &pTumbleRate);

    int project(double *pInCoordinates, double *pOutCoordinates);
    int unproject(double *pInCoordinates, double *pOutCoordinates);

    void viewAll();

    void setActive(const bool &pActive);

protected:
    virtual void initializeGL();
    virtual void paintGL();
    virtual void resizeGL(int pWidth, int pHeight);

    virtual void mouseMoveEvent(QMouseEvent *pEvent);
    virtual void mousePressEvent(QMouseEvent *pEvent);
    virtual void mouseReleaseEvent(QMouseEvent *pEvent);
    virtual void wheelEvent(QWheelEvent *pEvent);

    virtual QSize sizeHint() const;

private:
    bool mGraphicsInitialized;

    int mDevicePixelRatio;

    OpenCMISS::Zinc::Context *mContext;
    OpenCMISS::Zinc::Sceneviewer mSceneViewer;
    OpenCMISS::Zinc::Sceneviewernotifier mSceneViewerNotifier;

    ZincPanelWidgetSceneViewerCallback mZincPanelWidgetSceneViewerCallback;

    QFrame *mMarker;

    bool mActive;

    void updateMarkerColor();

    void createSceneViewer();

    void updateSceneViewerViewerportSize(const int &pWidth, const int &pHeight,
                                         const bool &pCheckDevicePixelRatio = false);

    OpenCMISS::Zinc::Sceneviewerinput::ButtonType buttonMap(const Qt::MouseButton &pButton) const;
    OpenCMISS::Zinc::Sceneviewerinput::ModifierFlags modifierMap(const Qt::KeyboardModifiers &pModifiers) const;

signals:
	void activated(OpenCOR::ZincPanelWidget::ZincPanelWidget *pZincPanel);
	void inactivated(OpenCOR::ZincPanelWidget::ZincPanelWidget *pZincPanel);
    void contextAboutToBeDestroyed();
    void graphicsInitialized();
    void devicePixelRatioChanged(const int &pDevicePixelRatio);
};

//==============================================================================

}   // namespace ZincPanelWidget
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
