#pragma once
#include <Aurie/shared.hpp>

struct ImGuiWrapperInterface : public Aurie::AurieInterfaceBase
{
    virtual void RegisterDrawCallback(std::function<void()> callback) = 0;
};
