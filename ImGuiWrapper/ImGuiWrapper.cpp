#include "ImGuiWrapper.h"

Aurie::AurieStatus ImGuiWrapper::Create()
{
    return Aurie::AURIE_SUCCESS;
}

void ImGuiWrapper::Destroy()
{
    this->draw_callbacks.clear();
}

void ImGuiWrapper::QueryVersion(
    OUT short& Major,
    OUT short& Minor,
    OUT short& Patch
)
{
    Major = 1;
    Minor = 0;
    Patch = 0;
}

void ImGuiWrapper::RegisterDrawCallback(std::function<void()> callback)
{
    this->draw_callbacks.push_back(callback);
}
