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
// Zinc panels widget
//==============================================================================

#include "corecliutils.h"
#include "coreguiutils.h"
#include "zincpanelswidget.h"

//==============================================================================

#include <QAction>
#include <QSettings>

//==============================================================================

namespace OpenCOR {
namespace ZincPanelWidget {

//==============================================================================

ZincPanelsWidget::ZincPanelsWidget(QWidget *pParent) :
    Core::SplitterWidget(pParent),
    mZincPanels(ZincPanelWidgets()),
    mActiveZincPanel(0)
{
    // Set our orientation

    setOrientation(Qt::Vertical);

}

//==============================================================================

void ZincPanelsWidget::retranslateUi()
{
    // Retranslate all our zinc panels

    foreach (ZincPanelWidget *zincPanel, mZincPanels)
        zincPanel->retranslateUi();
}

//==============================================================================

void ZincPanelsWidget::initialize()
{
    // Create a default zinc panel, if none exists

    if (mZincPanels.isEmpty())
        addZincPanel();
}

//==============================================================================

ZincPanelWidgets ZincPanelsWidget::zincPanels() const
{
    // Return our zinc panels

    return mZincPanels;
}

//==============================================================================

ZincPanelWidget * ZincPanelsWidget::activeZincPanel() const
{
    // Return our active zinc panel

    return mActiveZincPanel;
}

//==============================================================================

ZincPanelWidget * ZincPanelsWidget::addZincPanel(const bool &pActive)
{
    // Keep track of the zinc panels' original size

    QIntList origSizes = sizes();

    // Create a new zinc panel, add it to ourselves and keep track of it

    ZincPanelWidget *res = new ZincPanelWidget(mZincPanels,
                                                this);

    mZincPanels << res;

    // Resize the zinc panels, thus making sure that their size is what it
    // should be

    double scalingFactor = double(mZincPanels.count()-1)/mZincPanels.count();

    for (int i = 0, iMax = origSizes.count(); i < iMax; ++i)
        origSizes[i] *= scalingFactor;

    setSizes(origSizes << height()/mZincPanels.count());

    // Keep track of whenever a zinc panel gets activated

    connect(res, SIGNAL(activated(OpenCOR::ZincPanelWidget::ZincPanelWidget *)),
            this, SIGNAL(zincPanelActivated(OpenCOR::ZincPanelWidget::ZincPanelWidget *)));

    connect(res, SIGNAL(activated(OpenCOR::ZincPanelWidget::ZincPanelWidget *)),
            this, SLOT(updateZincPanels(OpenCOR::ZincPanelWidget::ZincPanelWidget *)));

    // Keep track of the addition and removal of a zinc

    //connect(res, SIGNAL(zincAdded(OpenCOR::ZincPanelWidget::ZincPanelWidget *, OpenCOR::ZincPanelWidget::ZincPanelPlotZinc *)),
    //        this, SIGNAL(zincAdded(OpenCOR::ZincPanelWidget::ZincPanelWidget *, OpenCOR::ZincPanelWidget::ZincPanelPlotZinc *)));
    //connect(res, SIGNAL(zincsRemoved(OpenCOR::ZincPanelWidget::ZincPanelWidget *, const OpenCOR::ZincPanelWidget::ZincPanelPlotZincs &)),
    //       this, SIGNAL(zincsRemoved(OpenCOR::ZincPanelWidget::ZincPanelWidget *, const OpenCOR::ZincPanelWidget::ZincPanelPlotZincs &)));

    // In/activate the zinc panel

    res->setActive(pActive);

    // Let people know that we have added a zinc panel

    emit zincPanelAdded(res, pActive);

    // Let people know whether zinc panels can be removed

    emit removeZincPanelsEnabled(mZincPanels.count() > 1);

    // Ask our first zinc panel's plot to align itself against its neighbours
    // Synchronise the axes of our zinc panels, if needed, and ensure that they
    // are all aligned with one another by forcing the setting of the axes of
    // our active zinc panel

    //ZincPanelPlotWidget *activeZincPanelPlot = mActiveZincPanel->plot();

    // Return our newly created zinc panel

    return res;
}

//==============================================================================

bool ZincPanelsWidget::removeZincPanel(ZincPanelWidget *pZincPanel)
{
    if (!pZincPanel)
        return false;

    // Retrieve the index of the given zinc panel

    int index = mZincPanels.indexOf(pZincPanel);

    // Let people know that we have removed it (or, rather, about to remove it)
    // Note: we let people know before we actually delete the zinc panel,
    //       because some people interested in that signal might have used the
    //       pointer to keep track of some information, as is done in
    //       SimulationExperimentViewInformationZincsWidget for example...

    emit zincPanelRemoved(pZincPanel);

    // Remove all tracks
    // Note: mActiveZincPanel will automatically get updated when another zinc
    //       panel gets selected...

    mZincPanels.removeOne(pZincPanel);

    // Now, we can delete our zinc panel

    delete pZincPanel;

    // Let people know whether zinc panels can be removed

    emit removeZincPanelsEnabled(mZincPanels.count() > 1);

    // Activate the next zinc panel or the last one available, if any

    if (index < mZincPanels.count()) {
        // There is a next zinc panel, so activate it

        mZincPanels[index]->setActive(true);
    } else {
        // We were dealing with the last zinc panel, so just activate the new
        // last zinc panel

        mZincPanels[mZincPanels.count()-1]->setActive(true);
    }

    if (!mZincPanels.isEmpty()) {
        return true;
    } else {
        return false;
    }
}

//==============================================================================

bool ZincPanelsWidget::removeCurrentZincPanel()
{
    // Make sure that we don't have only one zinc panel left

    if (mZincPanels.count() == 1)
        return false;

    // Remove the current zinc panel

    return removeZincPanel(mActiveZincPanel);
}

//==============================================================================

void ZincPanelsWidget::removeAllZincPanels()
{
    // Make sure that we don't have only one zinc panel left

    if (mZincPanels.count() == 1)
        return;

    // Remove all the zinc panels but one
    // Note: the one we keep is the very first one since it may be the user's
    //       most important zinc panel...

    while (mZincPanels.count() > 1)
        removeZincPanel(mZincPanels.last());
}

//==============================================================================

void ZincPanelsWidget::setActiveZincPanel(ZincPanelWidget *pZincPanel)
{
    // Make sure that we own the given zinc panel

    if (!mZincPanels.contains(pZincPanel))
        return;

    // Make the given zinc panel the active one

    pZincPanel->setActive(true);
}

//==============================================================================

void ZincPanelsWidget::updateZincPanels(OpenCOR::ZincPanelWidget::ZincPanelWidget *pZincPanel)
{
    // Keep track of the newly activated zinc panel

    mActiveZincPanel = pZincPanel;

    // Inactivate all the other zinc panels

    foreach (ZincPanelWidget *zincPanel, mZincPanels) {
        if (zincPanel != pZincPanel) {
            // We are not dealing with the zinc panel that just got activated,
            // so inactivate it

            zincPanel->setActive(false);
        }
    }
}

//==============================================================================

}   // namespace ZincPanelWidget
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
