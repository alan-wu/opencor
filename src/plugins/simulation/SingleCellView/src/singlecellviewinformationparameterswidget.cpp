//==============================================================================
// Single cell view information parameters widget
//==============================================================================

#include "cellmlfileruntime.h"
#include "propertyeditorwidget.h"
#include "singlecellviewinformationparameterswidget.h"
#include "singlecellviewsimulation.h"

//==============================================================================

#include <QHeaderView>
#include <QMenu>
#include <QSettings>

//==============================================================================

namespace OpenCOR {
namespace SingleCellView {

//==============================================================================

SingleCellViewInformationParametersWidget::SingleCellViewInformationParametersWidget(QWidget *pParent) :
    QStackedWidget(pParent),
    mPropertyEditors(QMap<QString, Core::PropertyEditorWidget *>()),
    mContextMenus(QMap<Core::PropertyEditorWidget *, QMenu *>()),
    mModelParameters(QMap<Core::Property *, CellMLSupport::CellmlFileRuntimeModelParameter *>()),
    mModelParameterActions(QMap<QAction *, CellMLSupport::CellmlFileRuntimeModelParameter *>()),
    mColumnWidths(QList<int>()),
    mSimulationData(0)
{
    // Determine the default width of each column of our property editors

    Core::PropertyEditorWidget *tempPropertyEditor = new Core::PropertyEditorWidget(this);

    for (int i = 0, iMax = tempPropertyEditor->header()->count(); i < iMax; ++i)
        mColumnWidths.append(tempPropertyEditor->columnWidth(i));

    delete tempPropertyEditor;
}

//==============================================================================

void SingleCellViewInformationParametersWidget::retranslateContextMenu(QMenu *pContextMenu)
{
    // Retranslate our context menu

    pContextMenu->actions().at(0)->setText(tr("Plot Against Variable of Integration"));
    pContextMenu->actions().at(1)->setText(tr("Plot Against"));
}

//==============================================================================

void SingleCellViewInformationParametersWidget::retranslateUi()
{
    // Retranslate all our property editors

    foreach (Core::PropertyEditorWidget *propertyEditor, mPropertyEditors)
        propertyEditor->retranslateUi();

    // Retranslate all our context menus

    foreach (QMenu *contextMenu, mContextMenus)
        retranslateContextMenu(contextMenu);

    // Retranslate the tool tip of all our model parameters

    updateModelParametersToolTips();
}

//==============================================================================

static const QString SettingsColumnWidth = "ColumnWidth%1";

//==============================================================================

void SingleCellViewInformationParametersWidget::loadSettings(QSettings *pSettings)
{
    // Retrieve the width of each column of our property editors

    for (int i = 0, iMax = mColumnWidths.size(); i < iMax; ++i)
        mColumnWidths[i] = pSettings->value(SettingsColumnWidth.arg(i),
                                            mColumnWidths.at(i)).toInt();

}

//==============================================================================

void SingleCellViewInformationParametersWidget::saveSettings(QSettings *pSettings) const
{
    // Keep track of the width of each column of our current property editor

    for (int i = 0, iMax = mColumnWidths.size(); i < iMax; ++i)
        pSettings->setValue(SettingsColumnWidth.arg(i), mColumnWidths.at(i));
}

//==============================================================================

void SingleCellViewInformationParametersWidget::initialize(const QString &pFileName,
                                                           CellMLSupport::CellmlFileRuntime *pRuntime,
                                                           SingleCellViewSimulationData *pSimulationData)
{
    // Keep track of the simulation data

    mSimulationData = pSimulationData;

    // Retrieve the property editor for the given file name or create one, if
    // none exists

    Core::PropertyEditorWidget *propertyEditor = mPropertyEditors.value(pFileName);

    if (!propertyEditor) {
        // No property editor exists for the given file name, so create one

        propertyEditor = new Core::PropertyEditorWidget(this);

        // Also create its corresponding context menu

        QMenu *contextMenu = new QMenu(this);

        // Initialise our property editor's columns' width

        for (int i = 0, iMax = mColumnWidths.size(); i < iMax; ++i)
            propertyEditor->setColumnWidth(i, mColumnWidths.at(i));

        // Populate our property editor

        populateModel(propertyEditor, pRuntime);

        // Create our context menu

        populateContextMenu(contextMenu, pRuntime);

        // We want our own context menu for our property editor

        propertyEditor->setContextMenuPolicy(Qt::CustomContextMenu);

        connect(propertyEditor, SIGNAL(customContextMenuRequested(const QPoint &)),
                this, SLOT(propertyEditorContextMenu(const QPoint &)));

        // Keep track of changes to columns' width

        connect(propertyEditor->header(), SIGNAL(sectionResized(int,int,int)),
                this, SLOT(propertyEditorSectionResized(const int &, const int &, const int &)));

        // Keep track of when some of the model's data has changed

        connect(pSimulationData, SIGNAL(updated()),
                this, SLOT(updateParameters()));

        // Keep track of when the user changes a property value

        connect(propertyEditor, SIGNAL(propertyChanged(Core::Property *)),
                this, SLOT(propertyChanged(Core::Property *)));

        // Add our new property editor to ourselves

        addWidget(propertyEditor);

        // Keep track of our new property editor and its context menu

        mPropertyEditors.insert(pFileName, propertyEditor);
        mContextMenus.insert(propertyEditor, contextMenu);
    }

    // Set our retrieved property editor as our widget

    setCurrentWidget(propertyEditor);

    // Update the tool tip of all our model parameters
    // Note: this is in case the user changed the locale and then switched to a
    //       different file...

    updateModelParametersToolTips();
}

//==============================================================================

void SingleCellViewInformationParametersWidget::finalize(const QString &pFileName)
{
    // Remove any track of our property editor and its context menu

    mContextMenus.remove(mPropertyEditors.value(pFileName));
    mPropertyEditors.remove(pFileName);
}

//==============================================================================

void SingleCellViewInformationParametersWidget::updateParameters()
{
    // Retrieve our current property editor, if any

    Core::PropertyEditorWidget *propertyEditor = qobject_cast<Core::PropertyEditorWidget *>(currentWidget());

    if (!propertyEditor)
        return;

    // Update our property editor's data

    foreach (Core::Property *property, propertyEditor->properties()) {
        CellMLSupport::CellmlFileRuntimeModelParameter *modelParameter = mModelParameters.value(property);

        if (modelParameter)
            switch (modelParameter->type()) {
            case CellMLSupport::CellmlFileRuntimeModelParameter::Constant:
            case CellMLSupport::CellmlFileRuntimeModelParameter::ComputedConstant:
                propertyEditor->setDoublePropertyItem(property->value(), mSimulationData->constants()[modelParameter->index()]);

                break;
            case CellMLSupport::CellmlFileRuntimeModelParameter::Rate:
                propertyEditor->setDoublePropertyItem(property->value(), mSimulationData->rates()[modelParameter->index()]);

                break;
            case CellMLSupport::CellmlFileRuntimeModelParameter::State:
                propertyEditor->setDoublePropertyItem(property->value(), mSimulationData->states()[modelParameter->index()]);

                break;
            case CellMLSupport::CellmlFileRuntimeModelParameter::Algebraic:
                propertyEditor->setDoublePropertyItem(property->value(), mSimulationData->algebraic()[modelParameter->index()]);

                break;
            default:
                // Either Voi or Undefined, so...

                ;
            }
    }

    // Check whether any of our properties has actually been modified

    mSimulationData->checkForModifications();

    // Update the tool tip of all our model parameters

    updateModelParametersToolTips();
}

//==============================================================================

void SingleCellViewInformationParametersWidget::propertyChanged(Core::Property *pProperty)
{
    // Retrieve our current property editor, if any

    Core::PropertyEditorWidget *propertyEditor = qobject_cast<Core::PropertyEditorWidget *>(currentWidget());

    if (!propertyEditor)
        return;

    // Update our simulation data

    CellMLSupport::CellmlFileRuntimeModelParameter *modelParameter = mModelParameters.value(pProperty);

    if (modelParameter)
        switch (modelParameter->type()) {
        case CellMLSupport::CellmlFileRuntimeModelParameter::Constant:
            mSimulationData->constants()[modelParameter->index()] = propertyEditor->doublePropertyItem(pProperty->value());

            break;
        case CellMLSupport::CellmlFileRuntimeModelParameter::State:
            mSimulationData->states()[modelParameter->index()] = propertyEditor->doublePropertyItem(pProperty->value());

            break;
        default:
            // Either Voi, ComputedConstant, Rate, Algebraic or Undefined, so...

            ;
        }

    // Recompute our 'computed constants' and 'variables'
    // Note: we would normally call mSimulationData->checkForModifications()
    //       after recomputing our 'computed constants' and 'variables, but the
    //       recomputation will eventually result in updateParameters() abvove
    //       to be called and it will check for modifications, so...

    mSimulationData->recomputeComputedConstantsAndVariables();
}

//==============================================================================

void SingleCellViewInformationParametersWidget::finishPropertyEditing()
{
    // Retrieve our current property editor, if any

    Core::PropertyEditorWidget *propertyEditor = qobject_cast<Core::PropertyEditorWidget *>(currentWidget());

    if (!propertyEditor)
        return;

    // Finish the editing of our current property editor

    propertyEditor->finishPropertyEditing();
}

//==============================================================================

QIcon SingleCellViewInformationParametersWidget::modelParameterIcon(const CellMLSupport::CellmlFileRuntimeModelParameter::ModelParameterType &pModelParameterType)
{
    // Return an icon that illustrates the type of a model parameter

    switch (pModelParameterType) {
    case CellMLSupport::CellmlFileRuntimeModelParameter::Constant:
        return QIcon(":SingleCellView_constant");
    case CellMLSupport::CellmlFileRuntimeModelParameter::ComputedConstant:
        return QIcon(":SingleCellView_computedConstant");
    case CellMLSupport::CellmlFileRuntimeModelParameter::Rate:
        return QIcon(":SingleCellView_rate");
    case CellMLSupport::CellmlFileRuntimeModelParameter::State:
        return QIcon(":SingleCellView_state");
    case CellMLSupport::CellmlFileRuntimeModelParameter::Algebraic:
        return QIcon(":SingleCellView_algebraic");
    default:
        // We are dealing with a type of model parameter which is of no
        // interest to us
        // Note: we should never reach this point...

        return QIcon(":CellMLSupport_errorNode");
    }
}

//==============================================================================

void SingleCellViewInformationParametersWidget::populateModel(Core::PropertyEditorWidget *pPropertyEditor,
                                                              CellMLSupport::CellmlFileRuntime *pRuntime)
{
    // Prevent ourselves from being updated (to avoid any flickering)

    pPropertyEditor->setUpdatesEnabled(false);

    // Populate our property editor with the model parameters

    Core::Property *sectionProperty = 0;

    foreach (CellMLSupport::CellmlFileRuntimeModelParameter *modelParameter, pRuntime->modelParameters()) {
        // Check whether the current model parameter is in the same component as
        // the previous one

        QString currentComponent = modelParameter->component();

        if (   !sectionProperty
            ||  currentComponent.compare(sectionProperty->name()->text())) {
            // The current model parameter is in a different component, so
            // create a new section for the 'new' component

            sectionProperty = pPropertyEditor->addSectionProperty();

            pPropertyEditor->setStringPropertyItem(sectionProperty->name(), currentComponent);
        }

        // Add the current model parameter to the 'current' component section
        // Note: in case of an algebraic variable, if its degree is equal to
        //       zero, then we are dealing with a 'proper' algebraic variable
        //       otherwise a rate variable. Now, there may be several rate
        //       variables with the same name (but different degrees) and a
        //       state variable with the same name will also exist. So, to
        //       distinguish between all of them, we 'customise' our variable's
        //       name by appending n single quotes with n the degree of our
        //       variable (that degree is zero for all but rate variables; e.g.
        //       for a state variable which name is V, the corresponding rate
        //       variables of degree 1, 2 and 3 will be V', V'' and V''',
        //       respectively)...

        bool modelParameterEditable =    (modelParameter->type() == CellMLSupport::CellmlFileRuntimeModelParameter::Constant)
                                      || (modelParameter->type() == CellMLSupport::CellmlFileRuntimeModelParameter::State);

        Core::Property *property = pPropertyEditor->addDoubleProperty(QString(), modelParameterEditable, sectionProperty);

        property->name()->setIcon(modelParameterIcon(modelParameter->type()));

        pPropertyEditor->setStringPropertyItem(property->name(), modelParameter->name()+QString(modelParameter->degree(), '\''));

        QString perVoiUnitDegree = QString();

        if (modelParameter->degree()) {
            perVoiUnitDegree += "/"+pRuntime->variableOfIntegration()->unit();

            if (modelParameter->degree() > 1)
                perVoiUnitDegree += modelParameter->degree();
        }

        pPropertyEditor->setStringPropertyItem(property->unit(), modelParameter->unit()+perVoiUnitDegree);

        // Keep track of the link between our property value and model parameter

        mModelParameters.insert(property, modelParameter);
    }

    // Update the tool tip of all our model parameters

    updateModelParametersToolTips();

    // Expand all our properties

    pPropertyEditor->expandAll();

    // Allow ourselves to be updated again

    pPropertyEditor->setUpdatesEnabled(true);
}

//==============================================================================

void SingleCellViewInformationParametersWidget::populateContextMenu(QMenu *pContextMenu,
                                                                    CellMLSupport::CellmlFileRuntime *pRuntime)
{
    Q_UNUSED(pRuntime);

    // Create our two main menu items

    QAction *voiAction = pContextMenu->addAction(QString());
    QMenu *plotAgainstMenu = new QMenu(pContextMenu);

    pContextMenu->addAction(plotAgainstMenu->menuAction());

    // Initialise our two main menu items

    retranslateContextMenu(pContextMenu);

    // Create a connection to handle the plotting requirement against our
    // variable of integration

    connect(voiAction, SIGNAL(triggered()),
            this, SLOT(emitPlottingRequired()));

    // Keep track of the model parameter associated with our first main menu
    // item

    mModelParameterActions.insert(voiAction, pRuntime->variableOfIntegration());

    // Populate our property editor with the model parameters

    QMenu *componentMenu = 0;

    foreach (CellMLSupport::CellmlFileRuntimeModelParameter *modelParameter, pRuntime->modelParameters()) {
        // Check whether the current model parameter is in the same component as
        // the previous one

        QString currentComponent = modelParameter->component();

        if (   !componentMenu
            ||  currentComponent.compare(componentMenu->menuAction()->text())) {
            // The current model parameter is in a different component, so
            // create a new menu for the 'new' component

            componentMenu = new QMenu(currentComponent, pContextMenu);

            plotAgainstMenu->addMenu(componentMenu);
        }

        // Add the current model parameter to the 'current' component menu
        // Note: in case of an algebraic variable, see the corresponding note in
        //       populateModel() above...

        QAction *modelParameterAction = componentMenu->addAction(modelParameterIcon(modelParameter->type()),
                                                                 modelParameter->name()+QString(modelParameter->degree(), '\''));

        // Create a connection to handle the plotting requirement against our
        // model parameter

        connect(modelParameterAction, SIGNAL(triggered()),
                this, SLOT(emitPlottingRequired()));

        // Keep track of the model parameter associated with our model parameter
        // action

        mModelParameterActions.insert(modelParameterAction, modelParameter);
    }
}

//==============================================================================

void SingleCellViewInformationParametersWidget::updateModelParametersToolTips()
{
    // Retrieve our current property editor, if any

    Core::PropertyEditorWidget *propertyEditor = qobject_cast<Core::PropertyEditorWidget *>(currentWidget());

    if (!propertyEditor)
        return;

    // Update the tool tip of all our property editor's properties

    foreach (Core::Property *property, propertyEditor->properties()) {
        CellMLSupport::CellmlFileRuntimeModelParameter *modelParameter = mModelParameters.value(property);

        if (modelParameter) {
            QString modelParameterType = QString();

            switch (modelParameter->type()) {
            case CellMLSupport::CellmlFileRuntimeModelParameter::Constant:
                modelParameterType = tr("constant");

                break;
            case CellMLSupport::CellmlFileRuntimeModelParameter::ComputedConstant:
                modelParameterType = tr("computed constant");

                break;
            case CellMLSupport::CellmlFileRuntimeModelParameter::Rate:
                modelParameterType = tr("rate");

                break;
            case CellMLSupport::CellmlFileRuntimeModelParameter::State:
                modelParameterType = tr("state");

                break;
            case CellMLSupport::CellmlFileRuntimeModelParameter::Algebraic:
                modelParameterType = tr("algebraic");

                break;
            default:
                // We are dealing with a type of model parameter which is of no
                // interest to us, so do nothing...
                // Note: we should never reach this point...

                ;
            }

            QString modelParameterToolTip = property->name()->text()+tr(": ")+property->value()->text()+" "+property->unit()->text()+" ("+modelParameterType+")";

            property->name()->setToolTip(modelParameterToolTip);
            property->value()->setToolTip(modelParameterToolTip);
            property->unit()->setToolTip(modelParameterToolTip);
        }
    }
}

//==============================================================================

void SingleCellViewInformationParametersWidget::propertyEditorContextMenu(const QPoint &pPosition) const
{
    Q_UNUSED(pPosition);

    // Create a custom context menu for our property editor, based on what is
    // underneath our mouse pointer

    // Retrieve our current property editor, if any

    Core::PropertyEditorWidget *propertyEditor = qobject_cast<Core::PropertyEditorWidget *>(currentWidget());

    if (!propertyEditor)
        return;

    // Retrieve our current property, if any

    Core::Property *currentProperty = propertyEditor->currentProperty();

    if (!currentProperty)
        return;

    // Make sure that our current property is not a section

    if (currentProperty->name()->type() == Core::PropertyItem::Section)
        return;

    // Generate and show the context menu

    mContextMenus.value(propertyEditor)->exec(QCursor::pos());
}

//==============================================================================

void SingleCellViewInformationParametersWidget::propertyEditorSectionResized(const int &pLogicalIndex,
                                                                             const int &pOldSize,
                                                                             const int &pNewSize)
{
    Q_UNUSED(pOldSize);

    // Prevent all our property editors from responding to an updating of their
    // columns' width

    foreach (Core::PropertyEditorWidget *propertyEditor, mPropertyEditors)
        disconnect(propertyEditor->header(), SIGNAL(sectionResized(int,int,int)),
                   this, SLOT(propertyEditorSectionResized(const int &, const int &, const int &)));

    // Update the column width of all our property editors

    foreach (Core::PropertyEditorWidget *propertyEditor, mPropertyEditors)
        propertyEditor->header()->resizeSection(pLogicalIndex, pNewSize);

    // Keep track of the new column width

    mColumnWidths[pLogicalIndex] = pNewSize;

    // Re-allow all our property editors to respond to an updating of their
    // columns' width

    foreach (Core::PropertyEditorWidget *propertyEditor, mPropertyEditors)
        connect(propertyEditor->header(), SIGNAL(sectionResized(int,int,int)),
                this, SLOT(propertyEditorSectionResized(const int &, const int &, const int &)));
}

//==============================================================================

void SingleCellViewInformationParametersWidget::emitPlottingRequired()
{
    // Let people know that we want to plot the current model parameter against
    // another

    Core::PropertyEditorWidget *propertyEditor = qobject_cast<Core::PropertyEditorWidget *>(currentWidget());

    emit plottingRequired(mModelParameterActions.value(qobject_cast<QAction *>(sender())),
                          mModelParameters.value(propertyEditor->currentProperty()));
}

//==============================================================================

}   // namespace SingleCellView
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
