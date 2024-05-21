#include <YYToolkit/Shared.hpp>
#include <YYToolkit/Shared.cpp>  // NOLINT(bugprone-suspicious-include) lmao

using namespace Aurie;
using namespace YYTK;

static YYTKInterface* g_yytk_interface = nullptr;
static AurieModule* g_module = nullptr;

typedef RValue& ScriptFunction(CInstance* self, CInstance* other, RValue& return_value, int num_args, RValue** args);

static std::unordered_map<std::string, ScriptFunction*> orig_funcs;

ScriptFunction* hook_function(const std::string function_name, void* callback)
{
	auto runner = g_yytk_interface->GetRunnerInterface();
	auto id = runner.Script_Find_Id(function_name.c_str()) - 100000;
	CScript* script = nullptr;

	auto status = g_yytk_interface->GetScriptData(id, script);
	if (!AurieSuccess(status))
	{
		g_yytk_interface->Print(CM_LIGHTRED, "Failed to get script data for %s", function_name.c_str());
		return nullptr;
	}

	if (script == nullptr)
	{
		g_yytk_interface->Print(CM_LIGHTRED, "Script data for %s is null", function_name.c_str());
		return nullptr;
	}

	void* trampoline = nullptr;
	MmCreateHook(g_module,
		function_name,
		script->m_Functions->m_ScriptFunction,
		callback,
		&trampoline);
	if (orig_funcs[function_name] != nullptr)
	{
		g_yytk_interface->PrintWarning("Function %s is already hooked", function_name.c_str());
	}
	else
	{
		orig_funcs[function_name] = reinterpret_cast<ScriptFunction*>(trampoline);
	}
	return reinterpret_cast<ScriptFunction*>(trampoline);
}

template<size_t N>
struct StringLiteral {
	constexpr StringLiteral(const char(&str)[N]) {
		std::copy_n(str, N, value);
	}

	char value[N];
};

double get_enemy_hp(double id) {
	CInstance* instance;
	RValue* player_list;
	RValue* enemy_list;
	RValue* enemy;
	RValue* hp;

	g_yytk_interface->GetGlobalInstance(&instance);
	g_yytk_interface->GetInstanceMember(instance, "player", player_list);
	g_yytk_interface->GetArrayEntry(*player_list, 1, enemy_list);
	g_yytk_interface->GetArrayEntry(*enemy_list, id, enemy);
	g_yytk_interface->GetInstanceMember(*enemy, "displayHp", hp);
	return hp->m_Real;
}

template<StringLiteral name> RValue& zero_dmg(CInstance* self, CInstance* other, RValue& return_value, int num_args, RValue** args) {
	double enemy_hp = get_enemy_hp(args[1]->m_Real);
	if (args[2]->m_Real > enemy_hp - 1) {
		args[2]->m_Real = enemy_hp - 1;
	}
	return_value = orig_funcs[name.value](self, other, return_value, num_args, args);
	return return_value;
}

void setup_hooks(FWFrame&) {
	CInstance* global_instance;
	g_yytk_interface->GetGlobalInstance(&global_instance);
	RValue* online_version;
	g_yytk_interface->GetInstanceMember(global_instance, "onlineVersion", online_version);
	if (online_version->m_Real <= 10000) { online_version->m_Real += 10000; }

	g_yytk_interface->PrintInfo("Hardcore Mode Enabled");

	// Zero out damage when an enemy is close to dying.
	hook_function("gml_Script_scr_pattern_deal_damage_enemy_subtract", zero_dmg<"gml_Script_scr_pattern_deal_damage_enemy_subtract">);

	// TODO: check this interacts properly with DOTs
	// 
	// TODO: allow damage when enrage starts
	// - Check for bp_enrage calls (hopefulllllly works with bosses)
	// - Reset a flag on new encounter: scr_enemy_add_pattern or scrbp_phase_pattern_remove
	// TODO: make this work with matti mice (need some flag when they get summoned, remove it at end of fight)
	// TODO: make this work with shira (investigation needed for shira defense required move/patterns in general)
	
	// TODO (long term): Disable iFrames except from taking damage
	// - Look into wolf boss time slow code for hints
	g_yytk_interface->RemoveCallback(g_module, setup_hooks);
}

EXPORTED AurieStatus ModuleInitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(ModulePath);

	AurieStatus last_status = AURIE_SUCCESS;

	last_status = ObGetInterface("YYTK_Main", reinterpret_cast<AurieInterfaceBase*&>(g_yytk_interface));

	if (!AurieSuccess(last_status)) {
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}
	g_yytk_interface->CreateCallback(Module, EVENT_FRAME, setup_hooks, 0);

	g_module = Module;

	return AURIE_SUCCESS;
}