#include <switch_min.h>

#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/iosupport.h>
#include <sys/reent.h>
#include <switch_min/kernel/ipc.h>

#include "saltysd/saltysd_core.h"
#include "saltysd/saltysd_ipc.h"
#include "saltysd/saltysd_dynamic.h"
#include "saltysd/saltysd_helper.h"

#include "useful/useful.h"
#include "logging.h"

extern "C"
{
	extern u32 __start__;

	static char g_heap[0x8000];

	void __libnx_init(void *ctx, Handle main_thread, void *saved_lr);
	void __attribute__((weak)) NORETURN __libnx_exit(int rc);
	void __nx_exit(int, void *);
	void __libc_fini_array(void);
	void __libc_init_array(void);
}

u32 __nx_applet_type = AppletType_None;

Handle orig_main_thread;
void *orig_ctx;
void *orig_saved_lr;

void __libnx_init(void *ctx, Handle main_thread, void *saved_lr)
{
	extern char *fake_heap_start;
	extern char *fake_heap_end;

	fake_heap_start = &g_heap[0];
	fake_heap_end = &g_heap[sizeof g_heap];

	orig_ctx = ctx;
	orig_main_thread = main_thread;
	orig_saved_lr = saved_lr;

	// Call constructors.
	//void __libc_init_array(void);
	__libc_init_array();
}

void __attribute__((weak)) NORETURN __libnx_exit(int rc)
{
	// Call destructors.
	//void __libc_fini_array(void);
	__libc_fini_array();

	SaltySD_printf("SaltySD Plugin: jumping to %p\n", orig_saved_lr);

	__nx_exit(0, orig_saved_lr);
	while (true)
		;
}

// 7103580C20
u64 orig_func;
u64 test_replace(const char *message, u64 len)
{
	u64 jumpback = orig_func + (4*4);
	log_message("7103580C20: %s\n", message);
	asm("MOV X11, %x0" : : "r"(jumpback));
	asm("MOV X0, %x0" : : "r"(message));
	asm("MOV x1, %x0" : : "r"(len));

	asm("MOV	x9, 0xEB4F");
	asm("MOVK	X9, #0x27D4, LSL#16");
	asm("MOV    X8, #0x79F9");
	asm("MOVK   X8, #0x9E37, LSL#16");
	asm("BLR	X11");

	return;
}

int main(int argc, char *argv[])
{
	SaltySD_printf("SaltySD Plugin: alive\n");

	ANCHOR_ABS = SaltySDCore_getCodeStart();
	FILE *f = SaltySDCore_fopen("sdmc:/SaltySD/syslog.conf", "w");
	if (f)
	{
		SaltySD_printf("Writing config file...\n");
		char buffer[20];
		snprintf(buffer, 20, "%lx", (u64)&logger);
		SaltySDCore_fwrite(buffer, strlen(buffer), 1, f);
		SaltySDCore_fclose(f);
	}

	orig_func = IMPORT(0x7103580C20);
	SaltySD_function_replace(orig_func, (u64)&test_replace);

	__libnx_exit(0);
}
