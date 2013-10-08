/*******************************************************************************

Licensed to the OpenCOR team under one or more contributor license agreements.
See the NOTICE.txt file distributed with this work for additional information
regarding copyright ownership. The OpenCOR team licenses this file to you under
the Apache License, Version 2.0 (the "License"); you may not use this file
except in compliance with the License. You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.

*******************************************************************************/

//==============================================================================
// Single cell view graph panel plot widget
//==============================================================================

#include "cellmlfileruntime.h"
#include "singlecellviewgraphpanelplotwidget.h"

//==============================================================================

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QCursor>
#include <QDesktopWidget>
#include <QLocale>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QWidget>

//==============================================================================

#include <QtNumeric>

//==============================================================================

#include <qnamespace.h>

//==============================================================================

#include <float.h>

//==============================================================================

#include "qwt_painter.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_directpainter.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_renderer.h"
#include "qwt_scale_div.h"
#include "qwt_scale_engine.h"

//==============================================================================

#include "ui_singlecellviewgraphpanelplotwidget.h"

//==============================================================================

namespace OpenCOR {
namespace SingleCellView {

//==============================================================================

SingleCellViewGraphPanelPlotGraph::SingleCellViewGraphPanelPlotGraph(CellMLSupport::CellmlFileRuntimeParameter *pParameterX,
                                                                     CellMLSupport::CellmlFileRuntimeParameter *pParameterY) :
    QwtPlotCurve(),
    mSelected(true),
    mFileName(QString()),
    mParameterX(pParameterX),
    mParameterY(pParameterY)
{
    // Customise ourselves a bit

    setPen(QPen(Qt::darkBlue));
    setRenderHint(QwtPlotItem::RenderAntialiased);
}

//==============================================================================

bool SingleCellViewGraphPanelPlotGraph::isValid() const
{
    // Return whether we are valid

    return   !mFileName.isEmpty()
           && mParameterX && mParameterY;
}

//==============================================================================

bool SingleCellViewGraphPanelPlotGraph::isSelected() const
{
    // Return whether we are selected

    return mSelected;
}

//==============================================================================

void SingleCellViewGraphPanelPlotGraph::setSelected(const bool &pSelected)
{
    // Set our selected state

    mSelected = pSelected;
}

//==============================================================================

QString SingleCellViewGraphPanelPlotGraph::fileName() const
{
    // Return our file name

    return mFileName;
}

//==============================================================================

void SingleCellViewGraphPanelPlotGraph::setFileName(const QString &pFileName)
{
    // Set our file name

    mFileName = pFileName;
}

//==============================================================================

CellMLSupport::CellmlFileRuntimeParameter * SingleCellViewGraphPanelPlotGraph::parameterX() const
{
    // Return our parameter X

    return mParameterX;
}

//==============================================================================

void SingleCellViewGraphPanelPlotGraph::setParameterX(CellMLSupport::CellmlFileRuntimeParameter *pParameterX)
{
    // Set our parameter X

    mParameterX = pParameterX;
}

//==============================================================================

CellMLSupport::CellmlFileRuntimeParameter * SingleCellViewGraphPanelPlotGraph::parameterY() const
{
    // Return our parameter Y

    return mParameterY;
}

//==============================================================================

void SingleCellViewGraphPanelPlotGraph::setParameterY(CellMLSupport::CellmlFileRuntimeParameter *pParameterY)
{
    // Set our parameter Y

    mParameterY = pParameterY;
}

//==============================================================================

SingleCellViewGraphPanelPlotOverlayWidget::SingleCellViewGraphPanelPlotOverlayWidget(SingleCellViewGraphPanelPlotWidget *pParent) :
    QWidget(pParent),
    mOwner(pParent),
    mOriginPoint(QPoint()),
    mPoint(QPoint())
{
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);

    setFocusPolicy(Qt::NoFocus);
}

//==============================================================================

QPoint SingleCellViewGraphPanelPlotOverlayWidget::optimisedPoint(const QPoint &pPoint) const
{
    // Optimise the given point so that it fits within our owner's ranges

    QPoint realPoint = pPoint-mOwner->plotLayout()->canvasRect().topLeft().toPoint();

    QwtScaleMap canvasMapX = mOwner->canvasMap(QwtPlot::xBottom);
    QwtScaleMap canvasMapY = mOwner->canvasMap(QwtPlot::yLeft);

    return QPoint(qMin(qRound(canvasMapX.transform(mOwner->maxX())),
                       qMax(qRound(canvasMapX.transform(mOwner->minX())),
                            realPoint.x())),
                  qMin(qRound(canvasMapY.transform(mOwner->minY())),
                       qMax(qRound(canvasMapY.transform(mOwner->maxY())),
                            realPoint.y())));
}

//==============================================================================

void SingleCellViewGraphPanelPlotOverlayWidget::paintEvent(QPaintEvent *pEvent)
{
    // Accept the event

    pEvent->accept();

    // Check whether a mouse action is to be carried out

    if (mOwner->mouseAction() == SingleCellViewGraphPanelPlotWidget::None)
        return;

    // Paint the overlay, if any is needed

    QPainter painter(this);

    QRectF canvasRect = mOwner->plotLayout()->canvasRect();

    painter.translate(canvasRect.x(), canvasRect.y());

    switch (mOwner->mouseAction()) {
    case SingleCellViewGraphPanelPlotWidget::ShowCoordinates: {
        // Draw the two dashed lines that show the coordinates, using a dark
        // cyan pen

        QPen pen = painter.pen();
        QColor penColor = Qt::darkCyan;

        penColor.setAlphaF(0.69);

        pen.setColor(penColor);
        pen.setStyle(Qt::DashLine);

        painter.setPen(pen);

        QPoint point = optimisedPoint(mPoint);
        // Note: see drawCoordinates() as why we use QPoint rather than
        //       QPointF...

        painter.drawLine(0, point.y(), canvasRect.width(), point.y());
        painter.drawLine(point.x(), 0, point.x(), canvasRect.height());

        // Draw the coordinates of our point

        drawCoordinates(&painter, point, penColor, Qt::white, BottomRight);

        break;
    }
    case SingleCellViewGraphPanelPlotWidget::ZoomRegion: {
        // Draw the region to be zoomed

        QColor penColor = Qt::darkRed;
        QColor brushColor = Qt::yellow;

        penColor.setAlphaF(0.69);
        brushColor.setAlphaF(0.19);

        painter.setPen(penColor);

        QRect zoomRegionRect = zoomRegion();
        // Note: see drawCoordinates() as why we use QRect rather than QRectF...

        painter.fillRect(zoomRegionRect, brushColor);
        painter.drawRect(zoomRegionRect);

        // Draw the two sets of coordinates

        drawCoordinates(&painter, zoomRegionRect.topLeft(),
                        penColor, Qt::white, BottomRight, false);
        drawCoordinates(&painter, zoomRegionRect.topLeft()+QPoint(zoomRegionRect.width(), zoomRegionRect.height()),
                        penColor, Qt::white, TopLeft, false);

        break;
    }
    default:
        // Either no mouse action or not a mouse action we know how to handle

        ;
    }
}

//==============================================================================

void SingleCellViewGraphPanelPlotOverlayWidget::setOriginPoint(const QPoint &pOriginPoint)
{
    // Set our point of origin

    mOriginPoint = pOriginPoint;
    // Note: unlike for setPoint(), we must not repaint ourselves since a call
    //       to setOriginPoint() should always be followed by one to setPoint(),
    //       in which case we don't want to repaint ourselves twice, not least
    //       because the first time we may not have a correct mPoint value...
}

//==============================================================================

void SingleCellViewGraphPanelPlotOverlayWidget::setPoint(const QPoint &pPoint)
{
    // Set our point

    mPoint = pPoint;

    repaint();
}

//==============================================================================

QRect SingleCellViewGraphPanelPlotOverlayWidget::zoomRegion() const
{
    // Return the region to be zoomed based on mOriginPoint and mPoint
    // Note: by default, we assume that we are already fully zoomed in in both
    //       directions...

    QwtScaleMap canvasMapX = mOwner->canvasMap(QwtPlot::xBottom);
    QwtScaleMap canvasMapY = mOwner->canvasMap(QwtPlot::yLeft);

    double minX = canvasMapX.transform(mOwner->minX());
    double maxX = canvasMapX.transform(mOwner->maxX());
    double minY = canvasMapY.transform(mOwner->minY());
    double maxY = canvasMapY.transform(mOwner->maxY());

    if (mOwner->canZoomInX() || mOwner->canZoomInY()) {
        QPoint originPoint = optimisedPoint(mOriginPoint);
        QPoint point = optimisedPoint(mPoint);

        if (mOwner->canZoomInX()) {
            minX = qMin(originPoint.x(), point.x());
            maxX = qMax(originPoint.x(), point.x());
        }

        if (mOwner->canZoomInY()) {
            minY = qMin(originPoint.y(), point.y());
            maxY = qMax(originPoint.y(), point.y());
        }
    }

    return QRect(minX, minY, maxX-minX, maxY-minY);
}

//==============================================================================

void SingleCellViewGraphPanelPlotOverlayWidget::drawCoordinates(QPainter *pPainter,
                                                                const QPoint &pPoint,
                                                                const QColor &pBackgroundColor,
                                                                const QColor &pForegroundColor,
                                                                const Location &pLocation,
                                                                const bool &pCanMoveLocation)
{
    // Retrieve the size of coordinates as they will appear on the screen,
    // which means using the same font as the one used for the axes
    // Note: normally, pPoint would be a QPointF, but we want the coordinates to
    //       be drawn relative to something (see paintEvent()) and the only way
    //       to guarantee that everything will be painted as expected is to use
    //       QPoint. Indeed, if we were to use QPointF, then QPainter would have
    //       to do some rouding and though everything should be fine (since we
    //       always add/subtract a rounded number), it happens that it's not
    //       always the case. Indeed, we should always have a gap of one pixel
    //       between the coordinates and pPoint, but it could happen that we
    //       have either no gap or one of two pixels, so...

    pPainter->setFont(mOwner->axisFont(QwtPlot::xBottom));

    QPointF point = mOwner->canvasPoint(pPoint, false);
    QString coordinates = QString("X: %1\nY: %2").arg(QLocale().toString(point.x(), 'g', 15),
                                                      QLocale().toString(point.y(), 'g', 15));
    QRect coordinatesRect = pPainter->boundingRect(qApp->desktop()->availableGeometry(), 0, coordinates);

    // Determine where the coordinates and its background should be drawn

    switch (pLocation) {
    case TopLeft:
        coordinatesRect.moveTo(pPoint.x()-coordinatesRect.width()-1,
                               pPoint.y()-coordinatesRect.height()-1);

        break;
    case TopRight:
        coordinatesRect.moveTo(pPoint.x()+2,
                               pPoint.y()-coordinatesRect.height()-1);

        break;
    case BottomLeft:
        coordinatesRect.moveTo(pPoint.x()-coordinatesRect.width()-1,
                               pPoint.y()+2);

        break;
    case BottomRight:
        coordinatesRect.moveTo(pPoint.x()+2,
                               pPoint.y()+2);

        break;
    }

    if (pCanMoveLocation) {
        QwtScaleMap canvasMapX = mOwner->canvasMap(QwtPlot::xBottom);
        QwtScaleMap canvasMapY = mOwner->canvasMap(QwtPlot::yLeft);

        QPoint topLeftPoint = QPoint(canvasMapX.transform(mOwner->minX()),
                                     canvasMapY.transform(mOwner->maxY()));
        QPoint bottomRightPoint = QPoint(canvasMapX.transform(mOwner->maxX()),
                                         canvasMapY.transform(mOwner->minY()));

        if (coordinatesRect.top() < topLeftPoint.y())
            coordinatesRect.moveTop(pPoint.y()+2);
        else if (coordinatesRect.top()+coordinatesRect.height()-1 > bottomRightPoint.y())
            coordinatesRect.moveTop(pPoint.y()-coordinatesRect.height()-1);

        if (coordinatesRect.left() < topLeftPoint.x())
            coordinatesRect.moveLeft(pPoint.x()+2);
        else if (coordinatesRect.left()+coordinatesRect.width()-1 > bottomRightPoint.x())
            coordinatesRect.moveLeft(pPoint.x()-coordinatesRect.width()-1);

        // Note: the -1 for the else-if tests is because fillRect() below works
        //       on (0, 0; width-1, height-1)...
    }

    // Draw a filled rectangle to act as the background for the coordinates
    // we are to show

    pPainter->fillRect(coordinatesRect, pBackgroundColor);

    // Draw the text for the coordinates, using a white pen

    QPen pen = pPainter->pen();

    pen.setColor(pForegroundColor);

    pPainter->setPen(pen);

    pPainter->drawText(coordinatesRect, coordinates);
}

//==============================================================================

QwtText SingleCellViewGraphPanelPlotScaleDraw::label(double pValue) const
{
    if (qFuzzyCompare(pValue+1.0, 1.0))
        pValue = 0.0;

    return QLocale().toString(pValue, 'g', 15);
}

//==============================================================================

static const double DblMaxAxis = 0.3*DBL_MAX;
// Note: normally, we would use DBL_MAX, but this means that our maximum axis
//       range would be 2*DBL_MAX, which would cause problems with QwtPlot (e.g.
//       to create ticks), so instead we use a value that results in a range
//       that we know will work...

static const double MinAxis = -DblMaxAxis;
static const double MaxAxis =  DblMaxAxis;

static const double MaxAxisRange = MaxAxis-MinAxis;
static const double MinAxisRange = 1.0e-11;
// Note: to use a smaller minimum axis range would make the coordinates to be
//       shown as "0", so...

static const double DefMinAxis =    0.0;
static const double DefMaxAxis = 1000.0;

//==============================================================================

SingleCellViewGraphPanelPlotWidget::SingleCellViewGraphPanelPlotWidget(QWidget *pParent) :
    QwtPlot(pParent),
    CommonWidget(pParent),
    mGui(new Ui::SingleCellViewGraphPanelPlotWidget),
    mGraphs(QList<SingleCellViewGraphPanelPlotGraph *>()),
    mMouseAction(None),
    mOriginPoint(QPoint()),
    mPoint(QPoint()),
    mWantedMinX(0.0),
    mWantedMaxX(0.0),
    mWantedMinY(0.0),
    mWantedMaxY(0.0),
    mCanZoomInX(true),
    mCanZoomOutX(true),
    mCanZoomInY(true),
    mCanZoomOutY(true),
    mNeedContextMenu(false)
{
    // Set up the GUI

    mGui->setupUi(this);

    // Get ourselves a direct painter

    mDirectPainter = new QwtPlotDirectPainter(this);

    // Speedup painting on X11 systems
    // Note: this can only be done on X11 systems...

    if (QwtPainter::isX11GraphicsSystem())
        canvas()->setAttribute(Qt::WA_PaintOnScreen, true);

    // Customise ourselves a bit

    setCanvasBackground(Qt::white);

    setAxisScaleDraw(QwtPlot::xBottom, new SingleCellViewGraphPanelPlotScaleDraw());
    setAxisScaleDraw(QwtPlot::yLeft, new SingleCellViewGraphPanelPlotScaleDraw());

    // We don't want a frame around ourselves

    qobject_cast<QwtPlotCanvas *>(canvas())->setFrameShape(QFrame::NoFrame);

    // Set our axes' values

    doSetAxis(QwtPlot::xBottom, DefMinAxis, DefMaxAxis);
    doSetAxis(QwtPlot::yLeft, DefMinAxis, DefMaxAxis);

    // Attach a grid to ourselves

    QwtPlotGrid *grid = new QwtPlotGrid();

    grid->setMajorPen(Qt::gray, 0, Qt::DotLine);

    grid->attach(this);

    // Create our overlay widget

    mOverlayWidget = new SingleCellViewGraphPanelPlotOverlayWidget(this);

    // Create our context menu

    mContextMenu = new QMenu(this);

    mContextMenu->addAction(mGui->actionCopy);
    mContextMenu->addSeparator();
    mContextMenu->addAction(mGui->actionZoomIn);
    mContextMenu->addAction(mGui->actionZoomOut);
    mContextMenu->addSeparator();
    mContextMenu->addAction(mGui->actionResetZoom);
}

//==============================================================================

SingleCellViewGraphPanelPlotWidget::~SingleCellViewGraphPanelPlotWidget()
{
    // Delete some internal objects

    delete mDirectPainter;

    foreach (SingleCellViewGraphPanelPlotGraph *graph, mGraphs)
        delete graph;
}

//==============================================================================

void SingleCellViewGraphPanelPlotWidget::retranslateUi()
{
    // Retranslate our GUI

    mGui->retranslateUi(this);
}

//==============================================================================

bool SingleCellViewGraphPanelPlotWidget::eventFilter(QObject *pObject,
                                                     QEvent *pEvent)
{
    // Default handling of the event

    bool res = QwtPlot::eventFilter(pObject, pEvent);

    // We want to handle a double mouse click, but for some reasons to override
    // mouseDoubleClickEvent() doesn't work, so...

    if (pEvent->type() == QEvent::MouseButtonDblClick)
        handleMouseDoubleClickEvent(static_cast<QMouseEvent *>(pEvent));

    // We are all done, so...

    return res;
}

//==============================================================================

void SingleCellViewGraphPanelPlotWidget::handleMouseDoubleClickEvent(QMouseEvent *pEvent)
{
    // Reset the zoom level, in case we double-clicked using the left mouse
    // button with no modifiers

    if (   (pEvent->button() == Qt::LeftButton)
        && (pEvent->modifiers() == Qt::NoModifier))
        on_actionResetZoom_triggered();
}

//==============================================================================

void SingleCellViewGraphPanelPlotWidget::setWantedAxes(const double &pWantedMinX,
                                                       const double &pWantedMaxX,
                                                       const double &pWantedMinY,
                                                       const double &pWantedMaxY)
{
    // Set our axes' wanted values

    mWantedMinX = pWantedMinX;
    mWantedMaxX = pWantedMaxX;
    mWantedMinY = pWantedMinY;
    mWantedMaxY = pWantedMaxY;

    // Effectively update our axes by resetting them

    doSetAxes(Reset);
}

//==============================================================================

void SingleCellViewGraphPanelPlotWidget::updateActions()
{
    // Update our actions

    double rangeX = maxX()-minX();
    double rangeY = maxY()-minY();

    mCanZoomInX  = rangeX > MinAxisRange;
    mCanZoomOutX = rangeX < MaxAxisRange;

    mCanZoomInY  = rangeY > MinAxisRange;
    mCanZoomOutY = rangeY < MaxAxisRange;

    // Update the enabled status of our actions

    mGui->actionZoomIn->setEnabled(mCanZoomInX || mCanZoomInY);
    mGui->actionZoomOut->setEnabled(mCanZoomOutX || mCanZoomOutY);

    mGui->actionResetZoom->setEnabled(true);
//---GRY--- WE SHOULD 'PROPERLY' SET THE ENABLED STATE OF THE RESET ZOOM
//          ACTION...
}

//==============================================================================

void SingleCellViewGraphPanelPlotWidget::checkAxisValues(double &pMin,
                                                         double &pMax)
{
    // Make sure that our axis' values have finite values

    if (!qIsFinite(pMin))
        pMin = MinAxis;

    if (!qIsFinite(pMax))
        pMax = MaxAxis;

    // Make sure that our axis' values are valid

    double range = pMax-pMin;

    if (range > MaxAxisRange) {
        // The range is too big, so reset our values

        pMin = MinAxis;
        pMax = MaxAxis;
    } else if (range < MinAxisRange) {
        // The range is too small, so reset our values

        pMin = qMax(MinAxis, 0.5*(pMin+pMax-MinAxisRange));
        pMax = qMin(MaxAxis, pMin+MinAxisRange);
        pMin = pMax-MinAxisRange;
        // Note: the last statement is in case pMax was set to MaxAxis, in which
        //       case pMin has to be re-reset...
    } else if (pMin < MinAxis) {
        // The minimum value is too small, so reset it

        pMin = MinAxis;
        pMax = pMin+range;
    } else if (pMax > MaxAxis) {
        // The maximum value is too big, so reset it

        pMax = MaxAxis;
        pMin = pMax-range;
    }
}

//==============================================================================

void SingleCellViewGraphPanelPlotWidget::checkAxesValues(double &pMinX,
                                                         double &pMaxX,
                                                         double &pMinY,
                                                         double &pMaxY)
{
    // Make sure that our axes' values are fine

    checkAxisValues(pMinX, pMaxX);
    checkAxisValues(pMinY, pMaxY);
}

//==============================================================================

SingleCellViewGraphPanelPlotWidget::MouseAction SingleCellViewGraphPanelPlotWidget::mouseAction() const
{
    // Return our mouse action

    return mMouseAction;
}

//==============================================================================

void SingleCellViewGraphPanelPlotWidget::resetMouseAction()
{
    // Reset our mouse action and our overlay widget, by repainting it

    mMouseAction = None;

    mOverlayWidget->repaint();
}

//==============================================================================

double SingleCellViewGraphPanelPlotWidget::minX() const
{
    // Return our minimum X value

    return axisScaleDiv(QwtPlot::xBottom).lowerBound();
}

//==============================================================================

double SingleCellViewGraphPanelPlotWidget::maxX() const
{
    // Return our maximum X value

    return axisScaleDiv(QwtPlot::xBottom).upperBound();
}

//==============================================================================

double SingleCellViewGraphPanelPlotWidget::minY() const
{
    // Return our minimum Y value

    return axisScaleDiv(QwtPlot::yLeft).lowerBound();
}

//==============================================================================

double SingleCellViewGraphPanelPlotWidget::maxY() const
{
    // Return our maximum Y value

    return axisScaleDiv(QwtPlot::yLeft).upperBound();

}

//==============================================================================

bool SingleCellViewGraphPanelPlotWidget::canZoomInX() const
{
    // Return whether we can zoom in on the X axis

    return mCanZoomInX;
}

//==============================================================================

bool SingleCellViewGraphPanelPlotWidget::canZoomOutX() const
{
    // Return whether we can zoom out on the X axis

    return mCanZoomOutX;
}

//==============================================================================

bool SingleCellViewGraphPanelPlotWidget::canZoomInY() const
{
    // Return whether we can zoom in on the Y axis

    return mCanZoomInY;
}

//==============================================================================

bool SingleCellViewGraphPanelPlotWidget::canZoomOutY() const
{
    // Return whether we can zoom out on the Y axis

    return mCanZoomOutY;
}

//==============================================================================

QList<SingleCellViewGraphPanelPlotGraph *> SingleCellViewGraphPanelPlotWidget::graphs() const
{
    // Return all our graphs

    return mGraphs;
}

//==============================================================================

void SingleCellViewGraphPanelPlotWidget::optimiseAxisValues(const int &pAxisId,
                                                            double &pMin,
                                                            double &pMax)
{
    // Optimise the axis' values so that they fall onto a factor of the axis'
    // minor step

    uint base = axisScaleEngine(pAxisId)->base();
    double majorStep = QwtScaleArithmetic::divideInterval(pMax-pMin,
                                                          axisMaxMajor(pAxisId),
                                                          base);
    double minorStep = QwtScaleArithmetic::divideInterval(majorStep,
                                                          axisMaxMinor(pAxisId),
                                                          base);

    pMin = qFloor(pMin/minorStep)*minorStep;
    pMax = qCeil(pMax/minorStep)*minorStep;
}

//==============================================================================

void SingleCellViewGraphPanelPlotWidget::doSetAxis(const int &pAxis,
                                                   double pMin, double pMax)
{
    // Set our axis
    // Note: to use setAxisScale() on its own is not sufficient unless we were
    //       to replot ourselves immediately after, but we don't want to do
    //       that, so instead we also use setAxisScaleDiv() to make sure that
    //       our axis is indeed taken into account (i.e. we can retrieve them
    //       using minX(), maxX(), minY() and maxY()). Also, we must call
    //       setAxisScaleDiv() before setAxisScale() to make sure that the axis
    //       data is not considered as valid, which is important when it comes
    //       to plotting ourselves...

    setAxisScaleDiv(pAxis, QwtScaleDiv(pMin, pMax));
    setAxisScale(pAxis, pMin, pMax);

    // Make sure that our actions are up-to-date

    updateActions();
}

//==============================================================================

bool SingleCellViewGraphPanelPlotWidget::doSetAxes(const SettingAction &pSettingAction,
                                                   double pMinX, double pMaxX,
                                                   double pMinY,double pMaxY)
{
    // Keep track of our axes' old values

    double oldMinX = minX();
    double oldMaxX = maxX();
    double oldMinY = minY();
    double oldMaxY = maxY();

    // Make sure that the given axes' values are fine, if we want to set them,
    // or merge/reset our axes' values

    if (pSettingAction == Set) {
        checkAxesValues(pMinX, pMaxX, pMinY, pMaxY);
    } else {
        // Retrieve the bounding rectangle for all our graphs (but only for
        // those that actually have some data), if needed

        QRectF boundingRect = QRectF();
        bool wantMinMaxX = mWantedMinX != mWantedMaxX;
        bool wantMinMaxY = mWantedMinY != mWantedMaxY;

        if (!wantMinMaxX || !wantMinMaxY)
            foreach (SingleCellViewGraphPanelPlotGraph *graph, mGraphs)
                if (graph->isValid() && graph->isSelected() && graph->dataSize())
                    boundingRect |= graph->boundingRect();

        // Take into account our axes' needed values, if any, or use our default
        // axes' values, if our bounding rectangle has no width/height

        if (wantMinMaxX) {
            if (boundingRect.width()) {
                boundingRect.setLeft(qMin(boundingRect.left(), mWantedMinX));
                boundingRect.setWidth(qMax(boundingRect.width(), mWantedMaxX-mWantedMinX));
            } else {
                boundingRect.setLeft(mWantedMinX);
                boundingRect.setWidth(mWantedMaxX-mWantedMinX);
            }
        } else if (!boundingRect.width()) {
            boundingRect.setLeft(DefMinAxis);
            boundingRect.setWidth(DefMaxAxis-DefMinAxis);
        }

        if (wantMinMaxY) {
            if (boundingRect.height()) {
                boundingRect.setTop(qMin(boundingRect.top(), mWantedMinY));
                boundingRect.setHeight(qMax(boundingRect.height(), mWantedMaxY-mWantedMinY));
            } else {
                boundingRect.setTop(mWantedMinY);
                boundingRect.setHeight(mWantedMaxY-mWantedMinY);
            }
        } else if (!boundingRect.height()) {
            boundingRect.setTop(DefMinAxis);
            boundingRect.setHeight(DefMaxAxis-DefMinAxis);
        }

        // Update our axes' values, should we have retrieved a non-null bounding
        // rectangle and in case we need to reset or update the axes' values

        double realMinX = DefMinAxis;
        double realMaxX = DefMaxAxis;
        double realMinY = DefMinAxis;
        double realMaxY = DefMaxAxis;

        if (!boundingRect.isNull()) {
            // Retrieve our axes' values and make sure that they are fine

            double minX = boundingRect.left();
            double maxX = minX+boundingRect.width();
            double minY = boundingRect.top();
            double maxY = minY+boundingRect.height();

            checkAxesValues(minX, maxX, minY, maxY);

            // Optimise our axes' values by rounding them down/up, if needed,
            // and make sure that they are fine

            if (!wantMinMaxX || !wantMinMaxY) {
                if (!wantMinMaxX)
                    optimiseAxisValues(QwtPlot::xBottom, minX, maxX);

                if (!wantMinMaxY)
                    optimiseAxisValues(QwtPlot::yLeft, minY, maxY);

                checkAxesValues(minX, maxX, minY, maxY);
            }

            // Now, we can update our axes' values

            if (pSettingAction == Merge) {
                realMinX = qMin(realMinX, minX);
                realMaxX = qMax(realMaxX, maxX);
                realMinY = qMin(realMinY, minY);
                realMaxY = qMax(realMaxY, maxY);
            } else {
                // Reset

                realMinX = minX;
                realMaxX = maxX;
                realMinY = minY;
                realMaxY = maxY;
            }
        }

        // Effectively force our axes' new values

        pMinX = realMinX;
        pMaxX = realMaxX;
        pMinY = realMinY;
        pMaxY = realMaxY;
    }

    // Update our axes' values, if needed

    bool needReplot = false;

    if ((pMinX != oldMinX) || (pMaxX != oldMaxX)) {
        doSetAxis(QwtPlot::xBottom, pMinX, pMaxX);

        needReplot = true;
    }

    if ((pMinY != oldMinY) || (pMaxY != oldMaxY)) {
        doSetAxis(QwtPlot::yLeft, pMinY, pMaxY);

        needReplot = true;
    }

    // Replot ourselves, if needed

    if (needReplot) {
        replotNow();

        return true;
    } else {
        return false;
    }
}

//==============================================================================

void SingleCellViewGraphPanelPlotWidget::scaleAxes(const QPoint &pPoint,
                                                   const double &pScalingFactorX,
                                                   const double &pScalingFactorY)
{
    // Rescale our X axis, but only if zooming in/out is possible on that axis

    bool needRescaling = false;

    QPointF originPoint = canvasPoint(pPoint);

    double newMinX = minX();
    double newMaxX = maxX();

    if (   ((pScalingFactorX < 1.0) && mCanZoomInX)
        || ((pScalingFactorX > 1.0) && mCanZoomOutX)) {
        double oldMinX = minX();
        double oldRange = maxX()-oldMinX;
        double newRange = pScalingFactorX*oldRange;
        double factor = (originPoint.x()-oldMinX)/oldRange;
        // Note: we make sure that the factor is within the [0; 1] range...

        newMinX = qMax(MinAxis, originPoint.x()-factor*newRange);
        newMaxX = qMin(MaxAxis, newMinX+newRange);
        newMinX = newMaxX-newRange;
        // Note: the last statement is in case newMaxX has been set to mMaxX, in
        //       which case we need to update newMinX...

        needRescaling = true;
    }

    // Rescale our Y axis, but only if zooming in/out is possible on that axis

    double newMinY = minY();
    double newMaxY = maxY();

    if (   ((pScalingFactorY < 1.0) && mCanZoomInY)
        || ((pScalingFactorY > 1.0) && mCanZoomOutY)) {
        double oldMinY = minY();
        double oldRange = maxY()-oldMinY;
        double newRange = pScalingFactorY*(oldRange);
        double factor = (originPoint.y()-oldMinY)/oldRange;
        // Note: we make sure that the factor is within the [0; 1] range...

        newMinY = qMax(MinAxis, originPoint.y()-factor*newRange);
        newMaxY = qMin(MaxAxis, newMinY+newRange);
        newMinY = newMaxY-newRange;
        // Note: the last statement is in case newMaxY has been set to mMaxY, in
        //       which case we need to update newMinY...

        needRescaling = true;
    }

    // Rescale our axes, if needed

    if (needRescaling)
        doSetAxes(Set, newMinX, newMaxX, newMinY, newMaxY);
}

//==============================================================================

static const double NoScalingFactor     = 1.0;
static const double ScalingInFactor     = 0.9;
static const double ScalingOutFactor    = 1.0/ScalingInFactor;
static const double BigScalingInFactor  = 0.5*ScalingInFactor;
static const double BigScalingOutFactor = 1.0/BigScalingInFactor;

//==============================================================================

QPointF SingleCellViewGraphPanelPlotWidget::canvasPoint(const QPoint &pPoint,
                                                        const bool pNeedOffset) const
{
    // Return the mouse position using canvas coordinates, making sure that they
    // are within our ranges

    QPointF realPoint = pPoint;

    if (pNeedOffset)
        realPoint -= plotLayout()->canvasRect().topLeft();

    return QPointF(qMin(maxX(), qMax(minX(), canvasMap(QwtPlot::xBottom).invTransform(realPoint.x()))),
                   qMin(maxY(), qMax(minY(), canvasMap(QwtPlot::yLeft).invTransform(realPoint.y()))));
}

//==============================================================================

void SingleCellViewGraphPanelPlotWidget::mouseMoveEvent(QMouseEvent *pEvent)
{
    // Default handling of the event

    QwtPlot::mouseMoveEvent(pEvent);

    // Carry out the mouse action

    switch (mMouseAction) {
    case Pan: {
        // Determine the X/Y shifts for our panning

        QPointF originPoint = canvasPoint(mPoint);
        QPointF currentPoint = canvasPoint(pEvent->pos());

        double shiftX = currentPoint.x()-originPoint.x();
        double shiftY = currentPoint.y()-originPoint.y();

        mPoint = pEvent->pos();

        // Set our axes' new values

        doSetAxes(Set, minX()-shiftX, maxX()-shiftX, minY()-shiftY, maxY()-shiftY);

        break;
    }
    case ShowCoordinates:
        // Update the point of our overlay widget

        mOverlayWidget->setPoint(pEvent->pos());

        break;
    case Zoom: {
        // Determine our X/Y delta values

        QPointF originPoint = canvasPoint(mPoint);
        QPointF currentPoint = canvasPoint(pEvent->pos());

        double deltaX = currentPoint.x()-originPoint.x();
        double deltaY = currentPoint.y()-originPoint.y();

        mPoint = pEvent->pos();

        // Rescale ourselves
        // Note: this will automatically replot ourselves...

        scaleAxes(mOriginPoint,
                  deltaX?
                      (deltaX > 0)?
                          ScalingInFactor:
                          ScalingOutFactor:
                      NoScalingFactor,
                  deltaY?
                      (deltaY < 0)?
                          ScalingInFactor:
                          ScalingOutFactor:
                      NoScalingFactor);

        break;
    }
    case ZoomRegion:
        // Update our zoom region by updating the point of our overlay widget

        mOverlayWidget->setPoint(pEvent->pos());

        break;
    default:
        // None

        ;
    }

    // The mouse has moved, so we definitely won't need to show our context menu

    mNeedContextMenu = false;
}

//==============================================================================

void SingleCellViewGraphPanelPlotWidget::mousePressEvent(QMouseEvent *pEvent)
{
    // Default handling of the event

    QwtPlot::mousePressEvent(pEvent);

    // Check that the position of the mouse is over our canvas

    if (!plotLayout()->canvasRect().contains(pEvent->pos()))
        return;

    // Make sure that we are not already carrying out a mouse action (e.g. we
    // were zooming in/out and then pressed on the left mouse button) and if so,
    // then cancel it by resetting our mouse action

    if (mMouseAction != None) {
        resetMouseAction();

        return;
    }

    // Check which mouse action to can carry out

    if (   (pEvent->button() == Qt::LeftButton)
        && (pEvent->modifiers() == Qt::NoModifier)) {
        // We want to pan, but only do this if we are not completely zoomed out

        if (mCanZoomOutX || mCanZoomOutY) {
            mMouseAction = Pan;

            mPoint = pEvent->pos();
        }
    } else if (   (pEvent->button() == Qt::LeftButton)
               && (pEvent->modifiers() == Qt::ShiftModifier)) {
        // We want to show the coordinates

        mMouseAction = ShowCoordinates;

        mOverlayWidget->setPoint(pEvent->pos());
    } else if (   (pEvent->button() == Qt::RightButton)
               && (pEvent->modifiers() == Qt::NoModifier)) {
        // We want to zoom in/out

        mMouseAction = Zoom;

        mOriginPoint = pEvent->pos();
        mPoint = pEvent->pos();
    } else if (   (pEvent->button() == Qt::RightButton)
               && (pEvent->modifiers() == Qt::ControlModifier)) {
        // We want to zoom a region, but only allow it if we are not already
        // fully zoomed in

        if (mCanZoomInX || mCanZoomInY) {
            mMouseAction = ZoomRegion;

            mOverlayWidget->setOriginPoint(pEvent->pos());
            mOverlayWidget->setPoint(pEvent->pos());
        }
    }

    // Check whether we might need to show our context menu

    mNeedContextMenu = pEvent->button() == Qt::RightButton;
}

//==============================================================================

void SingleCellViewGraphPanelPlotWidget::mouseReleaseEvent(QMouseEvent *pEvent)
{
    // Default handling of the event

    QwtPlot::mouseReleaseEvent(pEvent);

    // Check whether we need to carry out a mouse action

    if (mMouseAction == None)
        return;

    // Finish carrying out the mouse action, if needed

    switch (mMouseAction) {
    case ZoomRegion: {
        // Retrieve our zoom region

        QRect zoomRegionRect = mOverlayWidget->zoomRegion();

        // Reset our mouse action

        resetMouseAction();

        // Effectively zoom our region, if possible, by updating our axes

        QRectF zoomRegion = QRectF(canvasPoint(zoomRegionRect.topLeft(), false),
                                   canvasPoint(zoomRegionRect.topLeft()+QPoint(zoomRegionRect.width(), zoomRegionRect.height()), false));

        if (zoomRegion.width() && zoomRegion.height())
            doSetAxes(Set,
                      zoomRegion.left(), zoomRegion.left()+zoomRegion.width(),
                      zoomRegion.top()+zoomRegion.height(), zoomRegion.top());

        break;
    }
    default:
        // A mouse action that doesn't require anything specific to be done,
        // except to reset our mouse action

        resetMouseAction();
    }

    // Show our context menu, if still needed

    if (mNeedContextMenu) {
        mOriginPoint = mapFromGlobal(QCursor::pos());

        mContextMenu->exec(QCursor::pos());
    }
}

//==============================================================================

void SingleCellViewGraphPanelPlotWidget::resizeEvent(QResizeEvent *pEvent)
{
    // Default handling of the event

    QwtPlot::resizeEvent(pEvent);

    // Update the size of our overlay widget

    mOverlayWidget->resize(pEvent->size());
}

//==============================================================================

void SingleCellViewGraphPanelPlotWidget::wheelEvent(QWheelEvent *pEvent)
{
    // Default handling of the event

    QwtPlot::wheelEvent(pEvent);

    // Make sure that we are not already carrying out a mouse action

    if (mMouseAction != None)
        return;

    // Make sure that we have actually got a delta that will tell us about the
    // kind of zooming we need to do

    if (!pEvent->delta())
        return;

    // The only mouse action we support using the wheel is zooming in/out, but
    // this requires no modifiers being used

    if (pEvent->modifiers() != Qt::NoModifier)
        return;

    // Zoom in/out by scaling our two axes

    double scalingFactor = (pEvent->delta() > 0)?ScalingInFactor:ScalingOutFactor;

    scaleAxes(pEvent->pos(), scalingFactor, scalingFactor);
}

//==============================================================================

void SingleCellViewGraphPanelPlotWidget::replotNow()
{
    // Replot ourselves

    replot();

    // Make sure that the replotting occurs immediately
    // Note: this is needed when running a simulation since, otherwise,
    //       replotting won't occur immediately (because of threading)...

    qApp->processEvents();
}

//==============================================================================

bool SingleCellViewGraphPanelPlotWidget::addGraph(SingleCellViewGraphPanelPlotGraph *pGraph)
{
    // Make sure that the given graph is not already attached to us

    if (mGraphs.contains(pGraph))
        return false;

    // Attach the given graph to ourselves and keep track of it

    pGraph->attach(this);

    mGraphs << pGraph;

    return true;
}

//==============================================================================

bool SingleCellViewGraphPanelPlotWidget::removeGraph(SingleCellViewGraphPanelPlotGraph *pGraph)
{
    // Check that the given graph is attached to us

    if (!mGraphs.contains(pGraph))
        return false;

    // Detach the given graph from ourselves, stop tracking it and delete it

    pGraph->detach();

    mGraphs.removeOne(pGraph);

    delete pGraph;

    return true;
}

//==============================================================================

void SingleCellViewGraphPanelPlotWidget::drawGraphSegment(SingleCellViewGraphPanelPlotGraph *pGraph,
                                                          const qulonglong &pFrom,
                                                          const qulonglong &pTo)
{
    // Make sure that we have a graph segment to draw

    if (pFrom == pTo)
        return;

    // Reset our axes and replot ourselves, if it is our first graph segment, or
    // carry on as normal

    if (!pFrom) {
        // It is our first graph segment, so reset our axes
        // Note: we always want to replot...

        if (!doSetAxes(Reset))
            replotNow();
    } else {
        // It's not our first graph segment, so determine the minimum/maximum
        // X/Y values for our new data

        double newMinX = 0.0;
        double newMaxX = 0.0;
        double newMinY = 0.0;
        double newMaxY = 0.0;

        for (qulonglong i = pFrom; i <= pTo; ++i)
            if (i == pFrom) {
                newMinX = newMaxX = pGraph->data()->sample(i).x();
                newMinY = newMaxY = pGraph->data()->sample(i).y();
            } else {
                double valX = pGraph->data()->sample(i).x();
                double valY = pGraph->data()->sample(i).y();

                newMinX = qMin(newMinX, valX);
                newMaxX = qMax(newMaxX, valX);

                newMinY = qMin(newMinY, valY);
                newMaxY = qMax(newMaxY, valY);
            }

        // Check whether our X/Y axis can handle the minimum/maximum X/Y values
        // for our new data

        if (   (newMinX < MinAxis) || (newMaxX > MaxAxis)
            || (newMinY < MinAxis) || (newMaxY > MaxAxis))
            // Our X/Y axis cannot handle the minimum/maximum X/Y values for our
            // new data, so check our axes by trying to set them

            doSetAxes(Merge, minX(), maxX(), minY(), maxY());
        else
            // Our X/Y axis can handle the X/Y min/max of our new data, so just
            // draw our new graph segment

            mDirectPainter->drawSeries(pGraph, pFrom, pTo);
    }
}

//==============================================================================

void SingleCellViewGraphPanelPlotWidget::on_actionCopy_triggered()
{
    // Copy our contents to the clipboard

    QApplication::clipboard()->setPixmap(grab());
}

//==============================================================================

void SingleCellViewGraphPanelPlotWidget::on_actionZoomIn_triggered()
{
    // Zoom in by scaling our two axes around the point where the context menu
    // was shown

    scaleAxes(mOriginPoint, BigScalingInFactor, BigScalingInFactor);
}

//==============================================================================

void SingleCellViewGraphPanelPlotWidget::on_actionZoomOut_triggered()
{
    // Zoom out by scaling our two axes around the point where the context menu
    // was shown

    scaleAxes(mOriginPoint, BigScalingOutFactor, BigScalingOutFactor);
}

//==============================================================================

void SingleCellViewGraphPanelPlotWidget::on_actionResetZoom_triggered()
{
    // Reset the zoom level by resetting our axes, but only if the reset zoom
    // mouse action is enabled
    // Note: we check for the reset zoom mouse action to be enabled since we may
    //       call this method directly...

    if (mGui->actionResetZoom->isEnabled())
        doSetAxes(Set, 0.0, 1000.0, 0.0, 1000.0);
//---GRY--- WE SHOULD 'PROPERLY' SET THE AXES...
}

//==============================================================================

}   // namespace SingleCellView
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
