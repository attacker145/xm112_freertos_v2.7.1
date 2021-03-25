// Copyright (c) Acconeer AB, 2016-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "acc_board.h"
#include "acc_device_uart.h"


#define MODULE	"start"


#define DEBUG_UART_PORT_INVALID (0xFF)

// Heap regions defined in MCU specific acc_heap.c file
extern HeapRegion_t  xHeapRegions[];
// Debug uart port defined in integration file acc_board_xxx.c
uint8_t                  acc_debug_uart_port = DEBUG_UART_PORT_INVALID;
static SemaphoreHandle_t acc_debug_uart_mutex;

#ifdef SAME70
void system_fatal_error_handler(const char* reason);
#define SYSTEM_FATAL(x) system_fatal_error_handler(x)
#else
#define SYSTEM_FATAL(x)
#endif

int _close(int file)
{
	(void)file;
	return 0;
}


int _fstat(int file, struct stat *st)
{
	(void)file;

	memset(st, 0, sizeof(*st));
	st->st_mode = S_IFCHR;
	return 0;
}


int _isatty(int file)
{
	(void)file;

	return 1;
}


int _lseek(int file, int ptr, int dir)
{
	(void)file;
	(void)ptr;
	(void)dir;

	return 0;
}


int _open(const char *name, int flags, int mode)
{
	(void)name;
	(void)flags;
	(void)mode;

	return -1;
}


int _read(int file, char *ptr, int len)
{
	(void)file;
	(void)ptr;
	(void)len;

	return -1;
}


caddr_t _sbrk(int incr)
{
	(void)incr;

	return (void*)(-1);
}


void _exit(int status)
{
	(void)status;
	SYSTEM_FATAL("_exit() called");
	for (;;);
}


void _init(void)
{
}


void _fini(void)
{
	SYSTEM_FATAL("_fini() called");
	for (;;);
}


void _kill(int pid, int sig)
{
	(void)pid;
	(void)sig;
	return;
}


int _getpid (void)
{
	return -1;
}


/**
 * @brief Write data to output device, used by newlib
 *
 * @param[in] file File to write to, ignored
 * @param[in] ptr Buffer with data to write
 * @param[in] len Number of bytes to write
 * @return number of bytes written
 */
int _write(int file, char *ptr, int len)
{
	(void)file;

	if (acc_debug_uart_port != DEBUG_UART_PORT_INVALID)
	{
		xSemaphoreTake(acc_debug_uart_mutex, portMAX_DELAY);
		acc_device_uart_write_buffer(acc_debug_uart_port, ptr, len);
		xSemaphoreGive(acc_debug_uart_mutex);
	}

	return len;
}

#ifndef SAME70
// For SAME70 these vectors can be found in cstartup.c
void HardFault_Handler(void)
{
	for (;;) ;
}


void MemManage_Handler(void)
{
	for (;;) ;
}


void BusFault_Handler(void)
{
	for (;;) ;
}


void UsageFault_Handler(void)
{
	for (;;) ;
}


void WDT_IRQHandler(void)
{
	for (;;) ;
}
#endif


/**
 * @brief The real main function to be started as first task
 */
extern int main(int argc, char *argv[]);


// Weak default implementation
int call_main(void) __attribute__ ((weak));


int call_main(void)
{
	return main(0, NULL);
}


/**
 * @brief Call main() using correct arguments
 */
static void start_main(void *param)
{
	(void)param;

	call_main();

	for (;;) ;
}


/**
 * @brief Create main task and start FreeRTOS scheduler
 */
void _start(void)
{
#ifdef STM32L476xx
	void SystemClock_80MHz(void);
	SystemClock_80MHz();
#endif

	vPortDefineHeapRegions(xHeapRegions);

	acc_debug_uart_mutex = xSemaphoreCreateMutex();
	if (acc_debug_uart_mutex == NULL)
	{
		SYSTEM_FATAL("Could not create mutex");
	}

	TaskHandle_t handle;
	xTaskCreate(start_main, "AccTask", 14000 / sizeof(int), NULL, tskIDLE_PRIORITY + 1, &handle);

	vTaskStartScheduler();

	for (;;) ;
}
