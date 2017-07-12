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
// Graph panels widget
//==============================================================================

#pragma once

//==============================================================================

#include "zincpanelwidget.h"
#include "zincpanelwidgetglobal.h"
#include "splitterwidget.h"

//==============================================================================

namespace OpenCOR {
namespace ZincPanelWidget {

//==============================================================================

class ZINCPANELWIDGET_EXPORT ZincPanelsWidget : public Core::SplitterWidget
{
    Q_OBJECT

public:
    explicit ZincPanelsWidget(QWidget *pParent);

    virtual void retranslateUi();

    void initialize();

    ZincPanelWidgets zincPanels() const;
    ZincPanelWidget * activeZincPanel() const;

    ZincPanelWidget * addZincPanel(const bool &pActive = true);

    bool removeCurrentZincPanel();
    void removeAllZincPanels();

    void setActiveZincPanel(ZincPanelWidget *pZincPanel);

private:
    ZincPanelWidgets mZincPanels;

    ZincPanelWidget *mActiveZincPanel;

    bool removeZincPanel(ZincPanelWidget *pZincPanel);

signals:
    void zincPanelAdded(OpenCOR::ZincPanelWidget::ZincPanelWidget *pZincPanel,
                         const bool &pActive);
    void zincPanelRemoved(OpenCOR::ZincPanelWidget::ZincPanelWidget *pZincPanel);

    void removeZincPanelsEnabled(const bool &pEnabled);

    void zincPanelActivated(OpenCOR::ZincPanelWidget::ZincPanelWidget *pZincPanel);

    //void zincAdded(OpenCOR::ZincPanelWidget::ZincPanelWidget *pZincPanel,
    //                OpenCOR::ZincPanelWidget::ZincPanelPlotZinc *pZinc);
    //void zincsRemoved(OpenCOR::ZincPanelWidget::ZincPanelWidget *pZincPanel,
    //                   const OpenCOR::ZincPanelWidget::ZincPanelPlotZincs &pZincs);

private slots:
    void updateZincPanels(OpenCOR::ZincPanelWidget::ZincPanelWidget *pZincPanel);

};

//==============================================================================

}   // namespace ZincPanelWidget
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
