#include <switch_min.h>
#include <switch_min/kernel/ipc.h>

#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/iosupport.h>
#include <sys/reent.h>

#include "saltysd/saltysd_core.h"
#include "saltysd/saltysd_ipc.h"
#include "saltysd/saltysd_dynamic.h"
#include "saltysd/saltysd_helper.h"

#include "useful/useful.h"
#include "mods.h"
#include "logging.h"

extern "C" {
	extern u32 __start__;

	static char g_heap[0x8000];

	void __libnx_init(void* ctx, Handle main_thread, void* saved_lr);
	void __attribute__((weak)) NORETURN __libnx_exit(int rc);
	void __nx_exit(int, void*);
	void __libc_fini_array(void);
	void __libc_init_array(void);
}

u32 __nx_applet_type = AppletType_None;

Handle orig_main_thread;
void* orig_ctx;
void* orig_saved_lr;

void __libnx_init(void* ctx, Handle main_thread, void* saved_lr) {
	extern char* fake_heap_start;
	extern char* fake_heap_end;

	fake_heap_start = &g_heap[0];
	fake_heap_end   = &g_heap[sizeof g_heap];
	
	orig_ctx = ctx;
	orig_main_thread = main_thread;
	orig_saved_lr = saved_lr;
	
	// Call constructors.
	//void __libc_init_array(void);
	__libc_init_array();
}

void __attribute__((weak)) NORETURN __libnx_exit(int rc) {
	// Call destructors.
	//void __libc_fini_array(void);
	__libc_fini_array();

	SaltySD_printf("SaltySD Plugin: jumping to %p\n", orig_saved_lr);

	__nx_exit(0, orig_saved_lr);
	while (true);
}

int main(int argc, char *argv[]) {
	SaltySD_printf("SaltySD Plugin: alive\n");

	// syslogger_send("Hello from plugin!!");
	// Get anchor for imports
	// do not remove if you plan on using IMPORT
	ANCHOR_ABS = SaltySDCore_getCodeStart();

	// Set our version string
	const char *ver = "Ver. %d.%d.%d";
	u64 version_string = SaltySDCore_findCode((u8 *)ver, strlen(ver));
	if (version_string) {
		SaltySD_Memcpy(version_string, (u64) "Hax. %d.%d.%d", 13);
	}

	// hook for script replacements
	//SaltySD_function_replace_sym("_ZN3lib8L2CAgent15clear_lua_stackEv", (u64) &clear_lua_stack_replace);

	//plugin::script_mods::script_replacement();
    FILE* f = SaltySDCore_fopen("sdmc:/SaltySD/syslog.conf", "w");
	if (f) {
		SaltySD_printf("Writing config file...\n");
		char buffer[20];
		snprintf(buffer, 20, "%lx", (u64)&logger);
		SaltySDCore_fwrite(buffer, strlen(buffer), 1, f);
		SaltySDCore_fclose(f);
	}

	plugin::code_mods::ledges();
	plugin::code_mods::limitedRegrabs();

	__libnx_exit(0);
}
