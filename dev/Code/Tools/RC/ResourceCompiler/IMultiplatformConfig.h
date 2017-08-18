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

#ifndef CRYINCLUDE_TOOLS_RC_RESOURCECOMPILER_IMULTIPLATFORMCONFIG_H
#define CRYINCLUDE_TOOLS_RC_RESOURCECOMPILER_IMULTIPLATFORMCONFIG_H
#pragma once

#include "IConfig.h"          // IConfig, IConfigKeyRegistry, enum EConfigPriority

class IMultiplatformConfig
{
public:
    virtual ~IMultiplatformConfig()
    {
    }

    virtual void init(int platformCount, int activePlatform, IConfigKeyRegistry* pConfigKeyRegistry) = 0;

    virtual int getPlatformCount() const = 0;
    virtual int getActivePlatform() const = 0;

    virtual const IConfig& getConfig(int platform) const = 0;
    virtual IConfig& getConfig(int platform) = 0;
    virtual const IConfig& getConfig() const = 0;
    virtual IConfig& getConfig() = 0;

    virtual void setKeyValue(EConfigPriority ePri, const char* key, const char* value) = 0;
};

#endif // CRYINCLUDE_TOOLS_RC_RESOURCECOMPILER_IMULTIPLATFORMCONFIG_H
