/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/
// Original file Copyright Crytek GMBH or its affiliates, used under license.

#include "StdAfx.h"
#include "UiAVSequenceProps.h"
#include <LyShine/Animation/IUiAnimation.h>
#include "UiEditorAnimationBus.h"
#include "UiAnimViewUndo.h"
#include "UiAnimViewSequence.h"
#include "Animation/AnimationContext.h"

#include "Objects/BaseObject.h"
//#include "Objects/SequenceObject.h"

#include "QtUtilWin.h"
#include <Animation/ui_UiAVSequenceProps.h>
#include <QMessageBox>


CUiAVSequenceProps::CUiAVSequenceProps(CUiAnimViewSequence* pSequence, float fps, QWidget* pParent)
    : QDialog(pParent)
    , m_FPS(fps)
    , m_outOfRange(0)
    , m_timeUnit(0)
    , ui(new Ui::CUiAVSequenceProps)
{
    ui->setupUi(this);
    assert(pSequence);
    m_pSequence = pSequence;
    connect(ui->BTNOK, &QPushButton::clicked, this, &CUiAVSequenceProps::OnOK);
    connect(ui->TU_SECONDS, &QRadioButton::toggled, this, &CUiAVSequenceProps::OnBnClickedTuSeconds);
    connect(ui->TU_FRAMES, &QRadioButton::toggled, this, &CUiAVSequenceProps::OnBnClickedTuFrames);

    OnInitDialog();
}

CUiAVSequenceProps::~CUiAVSequenceProps()
{
}

// CUiAVSequenceProps message handlers
BOOL CUiAVSequenceProps::OnInitDialog()
{
    QString name = m_pSequence->GetName();
    ui->NAME->setText(name);
    int seqFlags = m_pSequence->GetFlags();

    ui->MOVE_SCALE_KEYS->setChecked(false);

    ui->START_TIME->setRange(0.0, (1e+5));
    ui->END_TIME->setRange(0.0, (1e+5));

    Range timeRange = m_pSequence->GetTimeRange();

    m_timeUnit = 1;
    ui->START_TIME->setValue(timeRange.start);
    ui->END_TIME->setValue(timeRange.end);

    m_outOfRange = 0;
    if (m_pSequence->GetFlags() & IUiAnimSequence::eSeqFlags_OutOfRangeConstant)
    {
        m_outOfRange = 1;
        ui->ORT_CONSTANT->setChecked(true);
    }
    else if (m_pSequence->GetFlags() & IUiAnimSequence::eSeqFlags_OutOfRangeLoop)
    {
        m_outOfRange = 2;
        ui->ORT_LOOP->setChecked(true);
    }
    else
    {
        ui->ORT_ONCE->setChecked(true);
    }

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CUiAVSequenceProps::MoveScaleKeys()
{
    // Move/Rescale the sequence to a new time range.
    Range timeRangeOld = m_pSequence->GetTimeRange();
    Range timeRangeNew;
    timeRangeNew.start = ui->START_TIME->value();
    timeRangeNew.end = ui->END_TIME->value();

    if (!(timeRangeNew == timeRangeOld))
    {
        m_pSequence->AdjustKeysToTimeRange(timeRangeNew);
    }
}

void CUiAVSequenceProps::OnOK()
{
    QString name = ui->NAME->text();
    if (name.isEmpty())
    {
        QMessageBox::warning(this, "Sequence Properties", "A sequence name cannot be empty!");
        return;
    }
    else if (name.contains('/'))
    {
        QMessageBox::warning(this, "Sequence Properties", "A sequence name cannot contain a '/' character!");
        return;
    }

    UiAnimUndo undo("Change Animation Sequence Settings");
    UiAnimUndo::Record(new CUndoSequenceSettings(m_pSequence));

    if (ui->MOVE_SCALE_KEYS->isChecked())
    {
        MoveScaleKeys();
    }

    Range timeRange;
    timeRange.start = ui->START_TIME->value();
    timeRange.end = ui->END_TIME->value();

    if (m_timeUnit == 0)
    {
        float invFPS = 1.0f / m_FPS;
        timeRange.start = ui->START_TIME->value() * invFPS;
        timeRange.end = ui->END_TIME->value() * invFPS;
    }

    m_pSequence->SetTimeRange(timeRange);

    CUiAnimationContext* ac = nullptr;
    EBUS_EVENT_RESULT(ac, UiEditorAnimationBus, GetAnimationContext);
    if (ac)
    {
        ac->UpdateTimeRange();
    }

    QString seqName = m_pSequence->GetName();
    if (name != seqName)
    {
        // Rename sequence.
        m_pSequence->SetName(name.toLatin1().data());
    }

    int seqFlags = m_pSequence->GetFlags();
    seqFlags &= ~(IUiAnimSequence::eSeqFlags_OutOfRangeConstant | IUiAnimSequence::eSeqFlags_OutOfRangeLoop);

    if (ui->ORT_CONSTANT->isChecked())
    {
        seqFlags |= IUiAnimSequence::eSeqFlags_OutOfRangeConstant;
    }
    else if (ui->ORT_LOOP->isChecked())
    {
        seqFlags |= IUiAnimSequence::eSeqFlags_OutOfRangeLoop;
    }

    m_pSequence->SetFlags((IUiAnimSequence::EUiAnimSequenceFlags)seqFlags);
    accept();
}

void CUiAVSequenceProps::OnBnClickedTuFrames(bool v)
{
    if (!v)
    {
        return;
    }

    ui->START_TIME->setSingleStep(1.0f);
    ui->END_TIME->setSingleStep(1.0f);

    ui->START_TIME->setValue(double(int(ui->START_TIME->value() * m_FPS)));
    ui->END_TIME->setValue(double(int(ui->END_TIME->value() * m_FPS)));

    m_timeUnit = 1;
}


void CUiAVSequenceProps::OnBnClickedTuSeconds(bool v)
{
    if (!v)
    {
        return;
    }

    ui->START_TIME->setSingleStep(0.01f);
    ui->END_TIME->setSingleStep(0.01f);

    float fInvFPS = 1.0f / m_FPS;
    ui->START_TIME->setValue(ui->START_TIME->value() * fInvFPS);
    ui->END_TIME->setValue(ui->END_TIME->value() * fInvFPS);

    m_timeUnit = 0;
}

#include <Animation/UiAVSequenceProps.moc>