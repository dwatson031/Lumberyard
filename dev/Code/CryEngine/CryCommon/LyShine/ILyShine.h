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

#include <AzCore/Math/Vector2.h>
#include <LyShine/UiBase.h>

class IDraw2d;
class ISprite;
struct IUiAnimationSystem;
class IUiRenderer;
class UiEntityContext;

// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the LYSHINE_EXPORTS
// symbol defined in the stdafx.h. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// LYSHINE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef LYSHINE_EXPORTS
    #define LYSHINE_API DLL_EXPORT
#else
    #define LYSHINE_API DLL_IMPORT
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
//! The ILyShine interface provides access to the other interfaces provided by the UI module
class ILyShine
{
public:
    virtual ~ILyShine(){}

    //! Delete this object
    virtual void Release() = 0;

    //! Gets the IDraw2d interface
    virtual IDraw2d* GetDraw2d() = 0;

    //! Gets the IUiRenderer interface
    virtual IUiRenderer* GetUiRenderer() = 0;

    //! Create an empty UI Canvas (in game)
    //
    //! The system keeps track of all the loaded canvases and unloads them on game exit.
    virtual AZ::EntityId CreateCanvas() = 0;

    //! Load a UI Canvas from in-game
    virtual AZ::EntityId LoadCanvas(const string& assetIdPathname) = 0;

    //! Create an empty UI Canvas (for the UI editor)
    //
    //! The editor keeps track of the canvases it has open.
    virtual AZ::EntityId CreateCanvasInEditor(UiEntityContext* entityContext) = 0;

    //! Load a UI Canvas from the UI editor
    virtual AZ::EntityId LoadCanvasInEditor(const string& assetIdPathname, const string& sourceAssetPathname, UiEntityContext* entityContext) = 0;

    //! Reload a UI Canvas from xml. For use in the editor for the undo system only
    virtual AZ::EntityId ReloadCanvasFromXml(const AZStd::string& xmlString, UiEntityContext* entityContext) = 0;

    //! Get a loaded canvas by CanvasId
    //! NOTE: this only searches canvases loaded in the game (not the editor)
    virtual AZ::EntityId FindCanvasById(LyShine::CanvasId id) = 0;

    //! Get a loaded canvas by path name
    //! NOTE: this only searches canvases loaded in the game (not the editor)
    virtual AZ::EntityId FindLoadedCanvasByPathName(const string& assetIdPathname) = 0;

    //! Release a canvas from use either in-game or in editor, destroy UI Canvas if no longer used in either
    virtual void ReleaseCanvas(AZ::EntityId canvas, bool forEditor = false) = 0;

    //! Load a sprite object.
    virtual ISprite* LoadSprite(const string& pathname) = 0;

    //! Create a sprite that references the specified render target
    virtual ISprite* CreateSprite(const string& renderTargetName) = 0;

    //! Perform post-initialization (script system will be available)
    virtual void PostInit() = 0;

    //! Set the current viewport size, this should be called before Update and Render are called
    virtual void SetViewportSize(AZ::Vector2 viewportSize) = 0;

    //! Update UI elements
    //! \param deltaTimeInSeconds the amount of time in seconds since the last call to this function
    virtual void Update(float deltaTimeInSeconds) = 0;

    //! Render 2D and UI elements that should be rendered at end of frame
    virtual void Render() = 0;

    //! Execute events that were queued during a canvas update or input event handler
    virtual void ExecuteQueuedEvents() = 0;

    //! Reset the system (this happens at end of running game in Editor for example)
    virtual void Reset() = 0;

    //! Unload canvases that should be unloaded when a level is unloaded
    virtual void OnLevelUnload() = 0;
};

#ifdef __cplusplus
extern "C" {
#endif

LYSHINE_API ILyShine* CreateLyShineInterface(ISystem* system);

#ifdef __cplusplus
};
#endif

