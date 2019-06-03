/*
 * multithread.c - Make some functions of Keil C lib to support thread safety in FreeRTOS
 */

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*----------------------------------------------------------------------------
 *      Standard Library multithreading interface
 *---------------------------------------------------------------------------*/

/*--------------------------- _mutex_initialize -----------------------------*/

int _mutex_initialize(SemaphoreHandle_t *mutex) {
	/* Allocate and initialize a system mutex. */

	*mutex = xSemaphoreCreateBinary();
	xSemaphoreGive(*mutex);

	return 1;
}

/*--------------------------- _mutex_acquire --------------------------------*/

__attribute__((used)) void _mutex_acquire(SemaphoreHandle_t *mutex) {
	/* Acquire a system mutex, lock stdlib resources. */

	if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
		/* FreeRTOS running, acquire a mutex. */
		xSemaphoreTake(*mutex, portMAX_DELAY);
	}
}

/*--------------------------- _mutex_release --------------------------------*/

__attribute__((used)) void _mutex_release(SemaphoreHandle_t *mutex) {
	/* Release a system mutex, unlock stdlib resources. */

	if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
		/* FreeRTOS running, release a mutex. */
		xSemaphoreGive(*mutex);
	}
}

/* end of file multithread.c */
