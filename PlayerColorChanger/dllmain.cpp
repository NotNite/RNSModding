#include <Aurie/shared.hpp>
#include <YYToolkit/Shared.hpp>
#include <YYToolkit/Shared.cpp>  // NOLINT(bugprone-suspicious-include) lmao
#include <fstream>

using namespace Aurie;
using namespace YYTK;

typedef RValue& ScriptFunction(CInstance* self, CInstance* other, RValue& return_value,
                               int num_args, RValue** args);

static AurieModule* g_module = nullptr;
static YYTKInterface* g_module_interface = nullptr;
static ScriptFunction* scr_playercolor_set_original = nullptr;

RValue& scr_playercolor_set(YYTK::CInstance* self, CInstance* other, RValue& return_value,
                            int num_args, RValue** args)
{
    return_value = scr_playercolor_set_original(self, other, return_value, num_args, args);

    CInstance* global_instance = nullptr;
    auto status = g_module_interface->GetGlobalInstance(&global_instance);
    if (!AurieSuccess(status) || global_instance == nullptr) return return_value;

    // The ally ID for the local client
    RValue* own_id = nullptr;
    status = g_module_interface->GetInstanceMember(global_instance, "clientOwnId", own_id);
    if (!AurieSuccess(status) || own_id == nullptr) return return_value;

    // Array of array, index to access ally ID
    RValue* player_char_id = nullptr;
    status = g_module_interface->GetInstanceMember(global_instance, "playerCharId", player_char_id);
    if (!AurieSuccess(status) || player_char_id == nullptr) return return_value;

    RValue* player_char_id_inner = nullptr;
    status = g_module_interface->GetArrayEntry(*player_char_id, 0, player_char_id_inner);
    if (!AurieSuccess(status) || player_char_id_inner == nullptr) return return_value;

    RValue* resolved_ally_id = nullptr;
    status = g_module_interface->GetArrayEntry(*player_char_id_inner, own_id->m_i32, resolved_ally_id);
    if (!AurieSuccess(status) || resolved_ally_id == nullptr) return return_value;

    // The ally ID for the player we're currently setting the color for
    RValue* self_id = nullptr;
    status = g_module_interface->GetInstanceMember(self, "allyId", self_id);
    if (!AurieSuccess(status) || self_id == nullptr) return return_value;

    // If the player we're setting the color for is the local client
    if (self_id->m_i32 == resolved_ally_id->m_i32)
    {
        // Array of array, index to access player color
        RValue* player_color = nullptr;
        status = g_module_interface->GetInstanceMember(global_instance, "playerColor", player_color);
        if (!AurieSuccess(status) || player_color == nullptr) return return_value;

        RValue* arr = nullptr;
        status = g_module_interface->GetArrayEntry(*player_color, 0, arr);
        if (!AurieSuccess(status) || arr == nullptr) return return_value;

        RValue* col = nullptr;
        status = g_module_interface->GetArrayEntry(*arr, own_id->m_i32, col);
        if (!AurieSuccess(status) || col == nullptr) return return_value;

        auto module = GetModuleHandle(nullptr);
        WCHAR path[MAX_PATH];
        GetModuleFileNameW(module, path, MAX_PATH);

        auto dir = fs::path(path).parent_path();
        auto color_path = dir / "player_color.txt";

        std::ifstream file(color_path);
        if (!file.bad())
        {
            auto color = std::string{};
            std::getline(file, color);
            if (!color.empty())
            {
                // Parse RGB
                auto rgb = std::stoul(color, nullptr, 16);
                // 0xBBGGRR
                col->m_Real = (rgb & 0xFF00) | (rgb & 0xFF) << 16 | (rgb & 0xFF0000) >> 16;
            }
        }
    }

    return return_value;
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

    auto runner = g_module_interface->GetRunnerInterface();
    auto id = runner.Script_Find_Id("scr_playercolor_set") - 100000;

    CScript* script = nullptr;
    status = g_module_interface->GetScriptData(id, script);
    if (!AurieSuccess(status) || script == nullptr) return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

    void* orig = nullptr;
    status = MmCreateHook(
        g_module,
        "PlayerColorChanger_scr_playercolor_set",
        script->m_Functions->m_ScriptFunction,
        scr_playercolor_set,
        &orig
    );
    if (!AurieSuccess(status)) return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
    scr_playercolor_set_original = static_cast<ScriptFunction*>(orig);

    return AURIE_SUCCESS;
}
