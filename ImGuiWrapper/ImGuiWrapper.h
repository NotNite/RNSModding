#pragma once
#include <map>
#include <Aurie/shared.hpp>
#include "ImGuiWrapperInterface.h"

struct ImGuiWrapper : public ImGuiWrapperInterface
{
    std::list<std::function<void()>> draw_callbacks;

    virtual Aurie::AurieStatus Create();
    virtual void Destroy();
    virtual void QueryVersion(
        OUT short& Major,
        OUT short& Minor,
        OUT short& Patch
    );
    
    virtual void RegisterDrawCallback(std::function<void()> callback) override;
};
