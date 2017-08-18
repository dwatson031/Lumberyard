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
#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/Math/Vector2.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//! Interface class that a tooltip display component needs to implement. A tooltip display
//! component is responsible for displaying an element as a tooltip given sizing and positioning
//! properties
class UiTooltipDisplayInterface
    : public AZ::ComponentBus
{
public: // types

    enum class AutoPositionMode
    {
        OffsetFromMouse,
        OffsetFromElement
    };

public: // member functions

    virtual ~UiTooltipDisplayInterface() {}

    //! Get whether the tooltip display element will be auto positioned
    virtual bool GetAutoPosition() = 0;

    //! Set whether the tooltip display element will be auto positioned
    virtual void SetAutoPosition(bool autoPosition) = 0;

    //! Get auto position mode
    virtual AutoPositionMode GetAutoPositionMode() = 0;

    //! Set auto position mode
    virtual void SetAutoPositionMode(AutoPositionMode autoPositionMode) = 0;

    //! Get the offset from the tooltip display element's pivot to the mouse position
    virtual const AZ::Vector2& GetOffset() = 0;

    //! Set the offset from the tooltip display element's pivot to the mouse position
    virtual void SetOffset(const AZ::Vector2& offset) = 0;

    //! Get whether the tooltip display element should be resized so that the text element
    //! size matches the size of the string
    virtual bool GetAutoSize() = 0;

    //! Set whether the tooltip display element should be resized so that the text element
    //! size matches the size of the string
    virtual void SetAutoSize(bool autoSize) = 0;

    //! Get the entity id of the text element that is used for resizing
    virtual AZ::EntityId GetTextEntity() = 0;

    //! Set the entity id of the text element that is used for resizing
    //! This must be a child of this entity
    virtual void SetTextEntity(AZ::EntityId textEntity) = 0;

    //! Get the amount of time to wait before showing the tooltip display element after hover start
    virtual float GetDelayTime() = 0;

    //! Set the amount of time to wait before showing the tooltip display element after hover start
    virtual void SetDelayTime(float delayTime) = 0;

    //! Get the amount of time the tooltip display element is to remain visible
    virtual float GetDisplayTime() = 0;

    //! Set the amount of time the tooltip display element is to remain visible
    virtual void SetDisplayTime(float displayTime) = 0;

    //! Show the tooltip display element
    virtual void PrepareToShow(AZ::EntityId tooltipElement) = 0;

    //! Hide the tooltip display element
    virtual void Hide() = 0;

    //! Update the tooltip display element
    virtual void Update() = 0;

public: // static member data

    //! Only one component on a entity can implement the events
    static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
};

typedef AZ::EBus<UiTooltipDisplayInterface> UiTooltipDisplayBus;
