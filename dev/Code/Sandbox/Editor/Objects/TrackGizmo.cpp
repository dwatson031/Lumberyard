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
#include "TrackGizmo.h"
#include "DisplayContext.h"
#include "TrackView/TrackViewAnimNode.h"
#include "AnimationContext.h"

#include "../Viewport.h"
#include "../DisplaySettings.h"

#include <IMovieSystem.h>
#include <ISplines.h>

#define TRACK_DRAW_Z_OFFSET (0.01f)

//////////////////////////////////////////////////////////////////////////
// CTrackGizmo implementation.
//////////////////////////////////////////////////////////////////////////
#define AXIS_SIZE 0.1f

namespace {
    int s_highlightAxis = 0;
}

//////////////////////////////////////////////////////////////////////////
CTrackGizmo::CTrackGizmo()
{
    m_pAnimNode = 0;

    m_worldBbox.min = Vec3(-10000, -10000, -10000);
    m_worldBbox.max = Vec3(10000, 10000, 10000);
    m_keysSelected = false;
}

//////////////////////////////////////////////////////////////////////////
CTrackGizmo::~CTrackGizmo()
{
}

//////////////////////////////////////////////////////////////////////////
void CTrackGizmo::SetMatrix(const Matrix34& tm)
{
    CGizmo::SetMatrix(tm);
    m_worldBbox.min = Vec3(-10000, -10000, -10000);
    m_worldBbox.max = Vec3(10000, 10000, 10000);
}

//////////////////////////////////////////////////////////////////////////
void CTrackGizmo::Display(DisplayContext& dc)
{
    if (!(dc.flags & DISPLAY_TRACKS))
    {
        return;
    }

    if (!m_pAnimNode)
    {
        return;
    }

    uint32 hideMask = gSettings.objectHideMask;

    CAnimationContext* ac = GetIEditor()->GetAnimation();

    // Should have animation sequence.
    if (!ac->GetSequence())
    {
        return;
    }

    m_keysSelected = false;

    // Must have non empty position track.
    CTrackViewTrack* pTrack = m_pAnimNode->GetTrackForParameter(eAnimParamType_Position);
    if (!pTrack)
    {
        return;
    }

    int nkeys = pTrack->GetKeyCount();
    if (nkeys < 2)
    {
        return;
    }

    Range range = ac->GetTimeRange();
    //range.start = __min(range.end,track->GetKeyTime(0));
    //range.end = __min(range.end,track->GetKeyTime(nkeys-1));
    //float step = range.Length() / 100.0f;
    //step = min(step,0.01f);
    float step = 0.1f;

    bool bTicks = (dc.flags & DISPLAY_TRACKTICKS) == DISPLAY_TRACKTICKS;

    // Get Spline color.
    ColorF splineCol(0.5f, 0.3f, 1, 1);
    ColorF timeCol(0, 1, 0, 1);

    m_worldBbox.Reset();

    float zOffset = TRACK_DRAW_Z_OFFSET;
    Vec3 p0(0, 0, 0), p1(0, 0, 0);
    Vec3 tick(0, 0, 0.05f);
    pTrack->GetValue(range.start, p0);
    p0 = m_matrix * p0;
    p0.z += zOffset;

    // Update bounding box.
    m_worldBbox.Add(p0);

    for (float t = range.start + step; t < range.end; t += step)
    {
        p1 = Vec3(0, 0, 0);
        pTrack->GetValue(t, p1);
        p1 = m_matrix * p1;
        p1.z += zOffset;

        // Update bounding box.
        m_worldBbox.Add(p1);

        if (bTicks)
        {
            dc.DrawLine(p0 - tick, p0 + tick, timeCol, timeCol);
        }
        dc.DrawLine(p0, p1, splineCol, splineCol);
        p0 = p1;
    }

    int nSubTracks = pTrack->GetChildCount();
    if (nSubTracks > 0)
    {
        for (int i = 0; i < nSubTracks; i++)
        {
            DrawKeys(dc, pTrack, static_cast<CTrackViewTrack*>(pTrack->GetChild(i)));
        }
    }
    else
    {
        DrawKeys(dc, pTrack, pTrack);
    }
}

//////////////////////////////////////////////////////////////////////////
void CTrackGizmo::SetAnimNode(CTrackViewAnimNode* pNode)
{
    m_pAnimNode = pNode;
}

//////////////////////////////////////////////////////////////////////////
void CTrackGizmo::GetWorldBounds(AABB& bbox)
{
    bbox = m_worldBbox;
}

//////////////////////////////////////////////////////////////////////////
void CTrackGizmo::DrawAxis(DisplayContext& dc, const Vec3& org)
{
    float size = AXIS_SIZE;

    dc.DepthTestOff();

    Vec3 x(size, 0, 0);
    Vec3 y(0, size, 0);
    Vec3 z(0, 0, size);

    float fScreenScale = dc.view->GetScreenScaleFactor(org);
    x = x * fScreenScale;
    y = y * fScreenScale;
    z = z * fScreenScale;

    float col[4] = { 1, 1, 1, 1 };
    float hcol[4] = { 1, 0, 0, 1 };
    dc.renderer->DrawLabelEx(org + x, 1.2f, col, true, true, "X");
    dc.renderer->DrawLabelEx(org + y, 1.2f, col, true, true, "Y");
    dc.renderer->DrawLabelEx(org + z, 1.2f, col, true, true, "Z");

    Vec3 colX(1, 0, 0), colY(0, 1, 0), colZ(0, 0, 1);
    if (s_highlightAxis)
    {
        float col[4] = { 1, 0, 0, 1 };
        if (s_highlightAxis == 1)
        {
            colX(1, 1, 0);
            dc.renderer->DrawLabelEx(org + x, 1.2f, col, true, true, "X");
        }
        if (s_highlightAxis == 2)
        {
            colY(1, 1, 0);
            dc.renderer->DrawLabelEx(org + y, 1.2f, col, true, true, "Y");
        }
        if (s_highlightAxis == 3)
        {
            colZ(1, 1, 0);
            dc.renderer->DrawLabelEx(org + z, 1.2f, col, true, true, "Z");
        }
    }

    x = x * 0.8f;
    y = y * 0.8f;
    z = z * 0.8f;
    float fArrowScale = fScreenScale * 0.07f;
    dc.SetColor(colX);
    dc.DrawArrow(org, org + x, fArrowScale);
    dc.SetColor(colY);
    dc.DrawArrow(org, org + y, fArrowScale);
    dc.SetColor(colZ);
    dc.DrawArrow(org, org + z, fArrowScale);

    dc.DepthTestOn();
}

//////////////////////////////////////////////////////////////////////////
bool CTrackGizmo::HitTest(HitContext& hc)
{
    return false;
}

//////////////////////////////////////////////////////////////////////////
void CTrackGizmo::DrawKeys(DisplayContext& dc, CTrackViewTrack* pTrack, CTrackViewTrack* pKeysTrack)
{
    // Get Key color.
    dc.SetColor(1, 0, 0, 1);

    float zOffset = TRACK_DRAW_Z_OFFSET;

    float sz = 0.2f;
    int nkeys = pKeysTrack->GetKeyCount();
    for (int i = 0; i < nkeys; i++)
    {
        const CTrackViewKeyHandle& keyHandle = pKeysTrack->GetKey(i);

        const float t = keyHandle.GetTime();
        Vec3 p0(0, 0, 0);
        pTrack->GetValue(t, p0);
        p0 = m_matrix * p0;
        p0.z += zOffset;

        //float sz = 0.01f * dc.view->GetScreenScaleFactor(p0);
        float sz = 0.005f * dc.view->GetScreenScaleFactor(p0);

        // Draw quad.
        //dc.DrawBall( p0,sz );
        dc.DrawWireBox(p0 - Vec3(sz, sz, sz), p0 + Vec3(sz, sz, sz));

        if (keyHandle.IsSelected())
        {
            m_keysSelected = true;
            DrawAxis(dc, p0);
            dc.SetColor(1, 0, 0, 1);
        }
    }
}
