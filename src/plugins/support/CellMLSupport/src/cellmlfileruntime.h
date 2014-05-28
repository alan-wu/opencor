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
// CellML file runtime class
//==============================================================================

#ifndef CELLMLFILERUNTIME_H
#define CELLMLFILERUNTIME_H

//==============================================================================

#include "cellmlfileissue.h"
#include "cellmlsupportglobal.h"

//==============================================================================

#include <QList>
#include <QObject>
#include <QStringList>

//==============================================================================

#include "cellml-api-cxx-support.hpp"

#include "IfaceCCGS.hxx"
#include "IfaceCellML_APISPEC.hxx"

//==============================================================================

namespace OpenCOR {

//==============================================================================

namespace Compiler {
    class CompilerEngine;
}   // namespace Compiler

namespace CellMLSupport {

//==============================================================================

class CellmlFile;

//==============================================================================

class CELLMLSUPPORT_EXPORT CellmlFileRuntimeParameter
{
public:
    enum ParameterType {
        Voi,
        Constant,
        ComputedConstant,
        Rate,
        State,
        Algebraic,
        Floating,
        LocallyBound
    };

    explicit CellmlFileRuntimeParameter(const QString &pName,
                                        const int &pDegree,
                                        const QString &pUnit,
                                        const QStringList &pComponentHierarchy,
                                        const ParameterType &pType,
                                        const int &pIndex);

    QString name() const;
    int degree() const;
    QString unit() const;
    QStringList componentHierarchy() const;
    ParameterType type() const;
    int index() const;

    QString formattedName() const;
    QString formattedComponentHierarchy() const;
    QString fullyFormattedName() const;

    QString formattedUnit(const QString &pVoiUnit) const;

private:
    QString mName;
    int mDegree;
    QString mUnit;
    QStringList mComponentHierarchy;
    ParameterType mType;
    int mIndex;
};

//==============================================================================

typedef QList<CellmlFileRuntimeParameter *> CellmlFileRuntimeParameters;

//==============================================================================

class CELLMLSUPPORT_EXPORT CellmlFileRuntime : public QObject
{
    Q_OBJECT

public:
    enum ModelType {
        Ode,
        Dae
    };

    typedef int (*InitializeConstantsFunction)(double *CONSTANTS, double *RATES, double *STATES);

    typedef int (*ComputeComputedConstantsFunction)(double *CONSTANTS, double *RATES, double *STATES);

    typedef int (*ComputeOdeRatesFunction)(double VOI, double *CONSTANTS, double *RATES, double *STATES, double *ALGEBRAIC);
    typedef int (*ComputeOdeVariablesFunction)(double VOI, double *CONSTANTS, double *RATES, double *STATES, double *ALGEBRAIC);

    typedef int (*ComputeDaeEssentialVariablesFunction)(double VOI, double *CONSTANTS, double *RATES, double *OLDRATES, double *STATES, double *OLDSTATES, double *ALGEBRAIC, double *CONDVAR);
    typedef int (*ComputeDaeResidualsFunction)(double VOI, double *CONSTANTS, double *RATES, double *OLDRATES, double *STATES, double *OLDSTATES, double *ALGEBRAIC, double *CONDVAR, double *resid);
    typedef int (*ComputeDaeRootInformationFunction)(double VOI, double *CONSTANTS, double *RATES, double *OLDRATES, double *STATES, double *OLDSTATES, double *ALGEBRAIC, double *CONDVAR);
    typedef int (*ComputeDaeStateInformationFunction)(double *SI);
    typedef int (*ComputeDaeVariablesFunction)(double VOI, double *CONSTANTS, double *RATES, double *STATES, double *ALGEBRAIC, double *CONDVAR);

    explicit CellmlFileRuntime();
    ~CellmlFileRuntime();

    QString address() const;

    bool isValid() const;

    ModelType modelType() const;

    bool needOdeSolver() const;
    bool needDaeSolver() const;
    bool needNlaSolver() const;

    int constantsCount() const;
    int statesCount() const;
    int ratesCount() const;
    int algebraicCount() const;
    int condVarCount() const;

    InitializeConstantsFunction initializeConstants() const;

    ComputeComputedConstantsFunction computeComputedConstants() const;

    ComputeOdeRatesFunction computeOdeRates() const;
    ComputeOdeVariablesFunction computeOdeVariables() const;

    ComputeDaeEssentialVariablesFunction computeDaeEssentialVariables() const;
    ComputeDaeResidualsFunction computeDaeResiduals() const;
    ComputeDaeRootInformationFunction computeDaeRootInformation() const;
    ComputeDaeStateInformationFunction computeDaeStateInformation() const;
    ComputeDaeVariablesFunction computeDaeVariables() const;

    CellmlFileIssues issues() const;

    CellmlFileRuntimeParameters parameters() const;

    CellmlFileRuntime * update(CellmlFile *pCellmlFile);

    CellmlFileRuntimeParameter * variableOfIntegration() const;

private:
    ModelType mModelType;
    bool mAtLeastOneNlaSystem;

    ObjRef<iface::cellml_services::CodeInformation> mOdeCodeInformation;
    ObjRef<iface::cellml_services::IDACodeInformation> mDaeCodeInformation;

    Compiler::CompilerEngine *mCompilerEngine;

    CellmlFileIssues mIssues;

    CellmlFileRuntimeParameter *mVariableOfIntegration;
    CellmlFileRuntimeParameters mParameters;

    InitializeConstantsFunction mInitializeConstants;

    ComputeComputedConstantsFunction mComputeComputedConstants;

    ComputeOdeRatesFunction mComputeOdeRates;
    ComputeOdeVariablesFunction mComputeOdeVariables;

    ComputeDaeEssentialVariablesFunction mComputeDaeEssentialVariables;
    ComputeDaeResidualsFunction mComputeDaeResiduals;
    ComputeDaeRootInformationFunction mComputeDaeRootInformation;
    ComputeDaeStateInformationFunction mComputeDaeStateInformation;
    ComputeDaeVariablesFunction mComputeDaeVariables;

    void resetOdeCodeInformation();
    void resetDaeCodeInformation();

    void resetFunctions();

    void reset(const bool &pRecreateCompilerEngine, const bool &pResetIssues);

    void couldNotGenerateModelCodeIssue();
    void unknownProblemDuringModelCodeGenerationIssue();

    void checkCodeInformation(iface::cellml_services::CodeInformation *pCodeInformation);

    void getOdeCodeInformation(CellmlFile *pCellmlFile);
    void getDaeCodeInformation(CellmlFile *pCellmlFile);

    QString functionCode(const QString &pFunctionSignature,
                         const QString &pFunctionBody,
                         const bool &pHasDefines = false);

    QStringList componentHierarchy(iface::cellml_api::CellMLElement *pElement);
};

//==============================================================================

}   // namespace CellMLSupport
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
