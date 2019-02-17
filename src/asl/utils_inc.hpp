/**
 * @file utils_inc.hpp
 * @brief 内联工具集
 * @author 程行通
 */

#pragma once

#include "asldef.hpp"
#ifdef WINDOWS
#  include <windows.h>
#else
#  include <unistd.h>
#  include <errno.h>
#  include <sys/time.h>
#  include <pthread.h>
#  include <semaphore.h>
#endif


#ifdef WINDOWS
#  define ASL_MUTEX_TYPE			CRITICAL_SECTION
#  define ASL_MUTEX_INIT(mtx)		InitializeCriticalSection(&mtx)
#  define ASL_MUTEX_DESTROY(mtx)	DeleteCriticalSection(&mtx)
#  define ASL_MUTEX_LOCK(mtx)		EnterCriticalSection(&mtx)
#  define ASL_MUTEX_TRYLOCK(mtx)	((TryEnterCriticalSection(&mtx) != FALSE) ? 0 : -1)
#  define ASL_MUTEX_UNLOCK(mtx)		LeaveCriticalSection(&mtx)
#else
#  define ASL_MUTEX_TYPE			pthread_mutex_t
#  define ASL_MUTEX_INIT(mtx)		pthread_mutex_init(&mtx, NULL)
#  define ASL_MUTEX_DESTROY(mtx)	pthread_mutex_destroy(&mtx)
#  define ASL_MUTEX_LOCK(mtx)		pthread_mutex_lock(&mtx)
#  define ASL_MUTEX_TRYLOCK(mtx)	pthread_mutex_trylock(&mtx)
#  define ASL_MUTEX_UNLOCK(mtx)		pthread_mutex_unlock(&mtx)
#endif
