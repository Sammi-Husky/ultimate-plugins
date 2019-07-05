#include <switch_min.h>
#include <string.h>
#include <cmath>
#include <stdio.h>
#include <stdarg.h>

#include "framework/saltysd/saltysd_core.h"
#include "framework/saltysd/saltysd_ipc.h"
#include "framework/saltysd/saltysd_dynamic.h"
#include "framework/saltysd/saltysd_helper.h"

#include "framework/useful/useful.h"
#include "framework/app/lua_bind.h"
#include "framework/acmd_wrapper.h"
#include "framework/useful/const_value_table.h"
#include "framework/useful/raygun_printer.h"

#include "code_mods/grabs.h"


using namespace app::lua_bind;

u64
__is_grab(u64 boma, int unk1) {
    print_string(boma, "is_grab %llx", unk1);

    u64 grab_module = load_module(boma, 0x158);
    return ((int (*)(u64, int))(load_module_impl(grab_module, 0x78)))(grab_module, unk1);
}

int plugin::code_mods::limitedRegrabs() {

    SaltySD_function_replace_sym("_ZN3app8lua_bind24GrabModule__is_grab_implEPNS_26BattleObjectModuleAccessorEi", (u64)__is_grab);
    return 0;
}