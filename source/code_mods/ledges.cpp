#include <switch_min.h>
#include <string.h>
#include <cmath>
#include <stdio.h>
#include <stdarg.h>

#include "framework/saltysd/saltysd_core.h"
#include "framework/saltysd/saltysd_ipc.h"
#include "framework/saltysd/saltysd_dynamic.h"
#include "framework/saltysd/saltysd_helper.h"

#include "framework/useful/const_value_table.h"
#include "framework/useful/useful.h"
#include "framework/app/lua_bind.h"
#include "framework/acmd_wrapper.h"

#include "code_mods/ledges.h"
#include "framework/useful/raygun_printer.h"

using namespace app::lua_bind;
using namespace lib;

//=========================================//
//=============Ledge Hogging===============//
//=========================================//
Vector3f ledge_pos[8];
u64 
__entry_cliff(u64 boma) {
	// Flag this cliff as occupied
	int entry_id = WorkModule::get_int(boma, FIGHTER_INSTANCE_WORK_ID_INT_ENTRY_ID);	
	ledge_pos[entry_id] = *(Vector3f*)GroundModule::hang_cliff_pos_3f(boma);
	
	// Call base func
	u64 ground_module = load_module(boma,0x58);
	return ((int (*)(u64))(load_module_impl(ground_module, 0x228)))(ground_module);
}

u64 
__can_entry_cliff(u64 boma) {
	// Get player pos and check if another player is 
	// occupying a ledge within our grab range
	Vector3f pos = *(Vector3f*)GroundModule::hang_cliff_pos_3f(boma);
	int entry_id = WorkModule::get_int(boma, FIGHTER_INSTANCE_WORK_ID_INT_ENTRY_ID);
	for (int i=0;i<8;i++) {
		if (i == entry_id || ledge_pos[i].x == 0) {
			continue;
		}

		if(pos.x == ledge_pos[i].x && pos.y == ledge_pos[i].y) {
			return false;
		}
	}

	// Call base func
	u64 ground_module = load_module(boma, 0x58);
	return ((int (*)(u64))(load_module_impl(ground_module, 0x238)))(ground_module);
}

u64 
__leave_cliff(u64 boma) {
	// clear occupancy when leaving edge
	int entry_id = WorkModule::get_int(boma , FIGHTER_INSTANCE_WORK_ID_INT_ENTRY_ID);
	ledge_pos[entry_id] = Vector3f { .x =0, .y=0, .z=0 };

	// Call base func
	u64 ground_module = load_module(boma, 0x58);	
	return ((int (*)(u64))(load_module_impl(ground_module, 0x250)))(ground_module);
}

int
plugin::code_mods::ledges() {
    // ledge hogging
	SaltySD_function_replace_sym("_ZN3app8lua_bind30GroundModule__entry_cliff_implEPNS_26BattleObjectModuleAccessorE",      (u64)&__entry_cliff);
	SaltySD_function_replace_sym("_ZN3app8lua_bind34GroundModule__can_entry_cliff_implEPNS_26BattleObjectModuleAccessorE",  (u64)&__can_entry_cliff);
	SaltySD_function_replace_sym("_ZN3app8lua_bind30GroundModule__leave_cliff_implEPNS_26BattleObjectModuleAccessorE",      (u64)&__leave_cliff);
	return 0;
}