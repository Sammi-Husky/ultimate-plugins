#include <switch_min.h>

#include <stdint.h>

#include "framework/useful/crc32.h"
#include "framework/useful/useful.h"
#include "framework/useful/const_value_table.h"
#include "framework/useful/raygun_printer.h"

#include "framework/saltysd/nn_ro.h"
#include "framework/saltysd/saltysd_dynamic.h"

#include "framework/acmd_wrapper.h"

using namespace lib;
using namespace app::sv_animcmd;
using namespace app::lua_bind;

void sv_replace_status_func(u64 l2c_agentbase, int status_kind, u64 key, void* func);

u64 shine_replace(L2CAgent* l2c_agent, void* variadic);
u64 ivy_upsmash(L2CAgent* l2c_agent, void* variadic);
u64 squirtle_utilt(L2CAgent* l2c_agent, void* variadic);
u64 end_shieldbreakfly_replace(u64 l2c_fighter, u64 l2c_agent);

u64 suicide_bomb_acmd_game = 0;

void replace_scripts(L2CAgent* l2c_agent, u8 category, int kind) {
	// fighter
	if (category == BATTLE_OBJECT_CATEGORY_FIGHTER) {
		
	}
}

void* sv_get_status_func(u64 l2c_agentbase, int status_kind, u64 key) {
	u64 unk48 = LOAD64(l2c_agentbase + 0x48);
	u64 unk50 = LOAD64(l2c_agentbase + 0x50);
	if (0x2E8BA2E8BA2E8BA3LL * ((unk50 - unk48) >> 4) > (u64)status_kind)
		return *(void **)(unk48 + 0xB0LL * status_kind + (key << 32 >> 29));
  
	return 0;
}

void sv_replace_status_func(u64 l2c_agentbase, int status_kind, u64 key, void* func) {
	u64 unk48 = LOAD64(l2c_agentbase + 0x48);
	u64 unk50 = LOAD64(l2c_agentbase + 0x50);
	if (0x2E8BA2E8BA2E8BA3LL * ((unk50 - unk48) >> 4) > (u64)status_kind) {
		*(void **)(unk48 + 0xB0LL * status_kind + (key << 32 >> 29)) = func;
	}
}

u64 clear_lua_stack_replace(u64 l2c_agent) {
	u64 lua_state = LOAD64(l2c_agent + 8);
	if ((lua_state - 8) && LOAD64(lua_state - 8) && (LOAD64(LOAD64(lua_state - 8) + 416LL))) {
		u8 battle_object_category = *(u8 *)(LOAD64(lua_state - 8) + 404LL);
		int battle_object_kind = *(int *)(LOAD64(lua_state - 8) + 408LL);
		replace_scripts((L2CAgent*)l2c_agent, battle_object_category, battle_object_kind);
	}

	// Original clear_lua_stack:
	u64 v1 = LOAD64(l2c_agent + 8);
	u64 v2 = LOAD64(v1 + 16);
	u64 i = LOAD64(LOAD64(v1 + 32)) + 16LL;
	for (; v2 < i; v2 = LOAD64(v1 + 16)) {
		LOAD64(v1 + 16) = v2 + 16;
		*(u32 *)(v2 + 8) = 0;
	}
	LOAD64(v1 + 16) = i;
	return l2c_agent;
}

int LoadModule_intercept(nn::ro::Module * module, void const* unk1, void * unk2, unsigned long unk3, int unk4) {
    int ret = nn::ro::LoadModule(module, unk1, unk2, unk3, unk4);

    SaltySDCore_RegisterModule((void*)(module->module.module->module_base));
	suicide_bomb_acmd_game = SaltySDCore_FindSymbol("_ZN7lua2cpp27L2CFighterAnimcmdGameCommon31bind_hash_call_game_SuicideBombEPN3lib8L2CAgentERNS1_7utility8VariadicEPKcSt9__va_list");

    return ret;
}
