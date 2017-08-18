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
#include "CaptureTrack.h"

//////////////////////////////////////////////////////////////////////////
void CCaptureTrack::SerializeKey(ICaptureKey& key, XmlNodeRef& keyNode, bool bLoading)
{
    if (bLoading)
    {
        const char* desc;

        keyNode->getAttr("time", key.time);
        keyNode->getAttr("flags", key.flags);
        keyNode->getAttr("duration", key.duration);
        keyNode->getAttr("timeStep", key.timeStep);
        desc = keyNode->getAttr("format");
        if (strcmp(desc, "jpg") == 0)
        {
            key.FormatJPG();
        }
        else if (strcmp(desc, "bmp") == 0)
        {
            key.FormatBMP();
        }
        else if (strcmp(desc, "hdr") == 0)
        {
            key.FormatHDR();
        }
        else
        {
            key.FormatTGA();
        }
        desc = keyNode->getAttr("folder");
        cry_strcpy(key.folder, desc);
        keyNode->getAttr("once", key.once);
        desc = keyNode->getAttr("prefix");
        if (desc)
        {
            cry_strcpy(key.prefix, desc);
        }
        int intAttr;
        keyNode->getAttr("bufferToCapture", intAttr);
        key.captureBufferIndex = (intAttr < ICaptureKey::NumCaptureBufferTypes) ? static_cast<ICaptureKey::CaptureBufferType>(intAttr) : ICaptureKey::Color;
    }
    else
    {
        keyNode->setAttr("time", key.time);
        keyNode->setAttr("flags", key.flags);
        keyNode->setAttr("duration", key.duration);
        keyNode->setAttr("timeStep", key.timeStep);
        keyNode->setAttr("format", key.GetFormat());
        keyNode->setAttr("folder", key.folder);
        keyNode->setAttr("once", key.once);
        keyNode->setAttr("prefix", key.prefix);
        keyNode->setAttr("bufferToCapture", key.captureBufferIndex);
    }
}

//////////////////////////////////////////////////////////////////////////
void CCaptureTrack::GetKeyInfo(int key, const char*& description, float& duration)
{
    assert(key >= 0 && key < (int)m_keys.size());
    static char buffer[256];
    CheckValid();
    duration = m_keys[key].once ? 0 : m_keys[key].duration;
    char prefix[64] = "Frame";
    if (strlen(m_keys[key].prefix) > 0)
    {
        cry_strcpy(prefix, m_keys[key].prefix);
    }
    description = buffer;
    if (strlen(m_keys[key].folder) > 0)
    {
        sprintf_s(buffer, "[%s] %s, %.3f, %s", prefix, m_keys[key].GetFormat(), m_keys[key].timeStep, m_keys[key].folder);
    }
    else
    {
        sprintf_s(buffer, "[%s] %s, %.3f", prefix, m_keys[key].GetFormat(), m_keys[key].timeStep);
    }
}