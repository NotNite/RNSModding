#include <Aurie/shared.hpp>
#include <YYToolkit/Shared.hpp>
#include <YYToolkit/Shared.cpp>  // NOLINT(bugprone-suspicious-include) lmao
#include <fstream>

using namespace Aurie;
using namespace YYTK;

static AurieModule* g_module = nullptr;
static YYTKInterface* g_module_interface = nullptr;

static bool upgrade_enabled = false;
static bool enc_enabled = false;
static bool dialog_enabled = false;
static bool item_enabled = false;
static bool item2_enabled = false;
static bool stat_enabled = false;

void CodeCallback(
    IN FWCodeEvent& CodeContext
)
{
    auto args = CodeContext.Arguments();
    auto self = std::get<0>(args);
    auto code = std::get<2>(args);
    auto name = std::string(code->GetName());

    auto enc = name == "gml_Object_obj_encDebugMenu_Create_0";
    auto dialog = name == "gml_Object_obj_dialogDebugMenu_Create_0";
    auto upgrade = name == "gml_Object_obj_upgradeDebugMenu_Create_0";
    auto item = name == "gml_Object_obj_itemDebugMenu_Create_0";
    auto item2 = name == "gml_Object_obj_itemDebugMenu_Step_0"; 
    auto stat = name == "gml_Object_obj_statDebugMenu_Create_0";

    if (enc || dialog || upgrade || item || stat)
    {
        RValue* is_shown = nullptr;
        g_module_interface->GetInstanceMember(self, "isShown", is_shown);
        if (is_shown != nullptr)
        {
            is_shown->m_Kind = VALUE_BOOL;
            is_shown->m_Real = 1;
        }
    }

    // I don't think it can be expressed how cursed editing a function as you're about to call it is
    if ((enc && enc_enabled) || (dialog && dialog_enabled))
    {
        auto func = code->m_Functions->m_CodeFunction;
        auto pos = MmSigscanRegion(reinterpret_cast<unsigned char*>(func), 0x1000,
                                   UTEXT(
                                       "\xC7\x45\x00\x00\x00\x00\x00\x0F\x57\xC0\xF2\x0F\x11\x45\x00\x48\x8D\x55\xBF\x48\x8B\xC8\xE8\x00\x00\x00\x00"),
                                   "xx?????xxxxxxx?xxxxxxxx????"
        );
        if (pos != 0)
        {
            auto ptr = reinterpret_cast<unsigned char*>(pos) + 22;
            DWORD old_protect;
            VirtualProtect(ptr, 5, PAGE_EXECUTE_READWRITE, &old_protect);
            memset(ptr, 0x90, 5);
            VirtualProtect(ptr, 5, old_protect, &old_protect);
        }

        if (enc) enc_enabled = false;
        if (dialog) dialog_enabled = false;
    }

    if (upgrade && upgrade_enabled)
    {
        auto func = code->m_Functions->m_CodeFunction;
        auto pos = MmSigscanRegion(reinterpret_cast<unsigned char*>(func), 0x1000,
                                   UTEXT(
                                       "\xC7\x45\x00\x00\x00\x00\x00\x0F\x57\xF6\xF2\x0F\x11\x74\x24\x00\x48\x8D\x54\x24\x00\x48\x8B\xC8\xE8\x00\x00\x00\x00"),
                                   "xx?????xxxxxxxx?xxxx?xxxx????"
        );
        if (pos != 0)
        {
            auto ptr = reinterpret_cast<unsigned char*>(pos) + 24;
            DWORD old_protect;
            VirtualProtect(ptr, 5, PAGE_EXECUTE_READWRITE, &old_protect);
            memset(ptr, 0x90, 5);
            VirtualProtect(ptr, 5, old_protect, &old_protect);
        }

        upgrade_enabled = false;
    }

    if (item && item_enabled)
    {
        auto func = code->m_Functions->m_CodeFunction;
        auto pos = MmSigscanRegion(reinterpret_cast<unsigned char*>(func), 0x1000,
                                   UTEXT(
                                       "\xC7\x44\x24\x00\x00\x00\x00\x00\x0F\x57\xF6\xF2\x0F\x11\x74\x24\x00\x48\x8D\x54\x24\x00\x48\x8B\xC8\xE8\x00\x00\x00\x00"),
                                   "xxx?????xxxxxxxx?xxxx?xxxx????"
        );
        if (pos != 0)
        {
            auto ptr = reinterpret_cast<unsigned char*>(pos) + 25;
            DWORD old_protect;
            VirtualProtect(ptr, 5, PAGE_EXECUTE_READWRITE, &old_protect);
            memset(ptr, 0x90, 5);
            VirtualProtect(ptr, 5, old_protect, &old_protect);
        }
    }

    if (item2 && item2_enabled)
    {
        auto func = reinterpret_cast<unsigned char*>(code->m_Functions->m_CodeFunction);
        auto pos = MmSigscanRegion(func, 0x1000,
                                   UTEXT(
                                       "\xC7\x45\x00\x00\x00\x00\x00\xF2\x44\x0F\x11\x45\x00\x48\x8D\x55\xF0\x48\x8B\xC8\xE8\x00\x00\x00\x00"),
                                   "xx?????xxxxx?xxxxxxxx????"
        );
        if (pos != 0)
        {
            auto ptr = reinterpret_cast<unsigned char*>(pos) + 20;
            DWORD old_protect;
            VirtualProtect(ptr, 5, PAGE_EXECUTE_READWRITE, &old_protect);
            memset(ptr, 0x90, 5);
            VirtualProtect(ptr, 5, old_protect, &old_protect);
        }

        item2_enabled = false;
    }

    if (stat && stat_enabled)
    {
        auto func = code->m_Functions->m_CodeFunction;
        auto pos = MmSigscanRegion(reinterpret_cast<unsigned char*>(func), 0x1000,
                                   UTEXT(
                                       "\xC7\x45\x00\x00\x00\x00\x00\x0F\x57\xF6\xF2\x0F\x11\x75\x00\x48\x8D\x55\xAF\x48\x8B\xC8\xE8\x00\x00\x00\x00"),
                                   "xx?????xxxxxxx?xxxxxxxx????"
        );
        if (pos != 0)   
        {
            auto ptr = reinterpret_cast<unsigned char*>(pos) + 22;
            DWORD old_protect;
            VirtualProtect(ptr, 5, PAGE_EXECUTE_READWRITE, &old_protect);
            memset(ptr, 0x90, 5);
            VirtualProtect(ptr, 5, old_protect, &old_protect);
        }

        stat_enabled = false;
    }

    CodeContext.Call();
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

    CInstance* global = nullptr;
    status = g_module_interface->GetGlobalInstance(&global);
    if (!AurieSuccess(status) || global == nullptr) return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

    RValue* online_version;
    status = g_module_interface->GetInstanceMember(global, "onlineVersion", online_version);
    if (!AurieSuccess(status) || online_version == nullptr) return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
    if (online_version->m_Real <= 10000) online_version->m_Real += 10000;

    RValue* dev_mode = nullptr;
    status = g_module_interface->GetInstanceMember(global, "devMode", dev_mode);
    if (!AurieSuccess(status) || dev_mode == nullptr) return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
    dev_mode->m_Kind = VALUE_BOOL;
    dev_mode->m_Real = 1;

    auto handle = GetModuleHandle(nullptr);
    WCHAR path[MAX_PATH];
    GetModuleFileNameW(handle, path, MAX_PATH);

    auto dir = fs::path(path).parent_path();
    auto encounter_path = dir / "debug.txt";

    std::ifstream file(encounter_path);
    if (!file.bad())
    {
        std::string line;
        while (std::getline(file, line))
        {
            if (line == "upgrade") upgrade_enabled = true;
            if (line == "enc") enc_enabled = true;
            if (line == "dialog") dialog_enabled = true;
            if (line == "stat") stat_enabled = true;
            if (line == "item")
            {
                item_enabled = true;
                item2_enabled = true;
            }
        }
    }

    g_module_interface->CreateCallback(
        g_module,
        EVENT_OBJECT_CALL,
        CodeCallback,
        0
    );

    return AURIE_SUCCESS;
}
