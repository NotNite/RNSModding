#include <Aurie/shared.hpp>
#include <YYToolkit/Shared.hpp>
#include <YYToolkit/Shared.cpp>  // NOLINT(bugprone-suspicious-include) lmao
#include <fstream>

#define CIMGUI_API __declspec(dllimport)
#include "cimgui.h"
#include "ImGuiWrapperInterface.h"

using namespace Aurie;
using namespace YYTK;

typedef RValue& ScriptFunction(CInstance* self, CInstance* other, RValue& return_value,
                               int num_args, RValue** args);

static AurieModule* g_module = nullptr;
static YYTKInterface* g_module_interface = nullptr;
static ScriptFunction* scrdt_encounter_original = nullptr;

// God I hate cimgui
static char encounter_raw[256] = {0};

RValue& scrdt_encounter(YYTK::CInstance* self, CInstance* other, RValue& return_value,
                        int num_args, RValue** args)
{
    if (num_args >= 1)
    {
        auto str = args[0]->AsString();
        g_module_interface->Print(CM_GRAY, "Starting encounter with %s", str.data());

        auto encounter = std::string(encounter_raw);
        if (!encounter.empty())
        {
            g_module_interface->Print(CM_GRAY, "Redirecting encounter to %s", encounter.c_str());
            g_module_interface->GetRunnerInterface().YYCreateString(args[0], encounter.c_str());
        }
    }

    return_value = scrdt_encounter_original(self, other, return_value, num_args, args);
    return return_value;
}

void draw()
{
    if (ImGui_Begin("SteelYourself", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui_InputText("Encounter", encounter_raw, sizeof(encounter_raw), ImGuiInputTextFlags_None);
    }
    ImGui_End();
}

EXPORTED AurieStatus ModuleInitialize(
    IN AurieModule* module,
    IN const fs::path& module_path
)
{
    g_module = module;
    auto status = ObGetInterface(
        "YYTK_Main",
        reinterpret_cast<AurieInterfaceBase*&>(g_module_interface)
    );
    if (!AurieSuccess(status)) return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

    CInstance* global_instance;
    status = g_module_interface->GetGlobalInstance(&global_instance);
    if (!AurieSuccess(status) || global_instance == nullptr) return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

    RValue* online_version;
    status = g_module_interface->GetInstanceMember(global_instance, "onlineVersion", online_version);
    if (!AurieSuccess(status) || online_version == nullptr) return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
    if (online_version->m_Real <= 10000) online_version->m_Real += 10000;

    auto runner = g_module_interface->GetRunnerInterface();
    auto id = runner.Script_Find_Id("scrdt_encounter") - 100000;

    CScript* script = nullptr;
    status = g_module_interface->GetScriptData(id, script);
    if (!AurieSuccess(status) || script == nullptr) return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

    void* orig = nullptr;
    status = MmCreateHook(
        g_module,
        "SteelYourself_scr_encounter",
        script->m_Functions->m_ScriptFunction,
        scrdt_encounter,
        &orig
    );
    if (!AurieSuccess(status)) return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
    scrdt_encounter_original = static_cast<ScriptFunction*>(orig);

    ImGuiWrapperInterface* imgui_wrapper = nullptr;
    ObGetInterface(
        "ImGuiWrapper",
        reinterpret_cast<AurieInterfaceBase*&>(imgui_wrapper)
    );
    if (imgui_wrapper != nullptr)
    {
        imgui_wrapper->RegisterDrawCallback(draw);
    }

    return AURIE_SUCCESS;
}
