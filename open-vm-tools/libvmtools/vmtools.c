/*********************************************************
 * Copyright (C) 2008 VMware, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation version 2.1 and no later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the Lesser GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA.
 *
 *********************************************************/

/**
 * @file vmtools.c
 *
 * Library entry point, utility and memory de-allocation functions for the VMTools
 * shared library.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#if defined(_WIN32)
#  include <windows.h>
#  include "coreDump.h"
#  include "netutil.h"
#endif

#include "vm_assert.h"
#include "vmtools.h"
#include "wiper.h"

#if !defined(__APPLE__)
#include "embed_version.h"
#include "vmtoolslib_version.h"
VM_EMBED_VERSION(VMTOOLSLIB_VERSION_STRING);
#endif


/**
 * A convenience function for wrapping an array with a GArray instance.
 *
 * @param[in]  data        The array data. The original data is copied into the
 *                         new array.
 * @param[in]  elemSize    The size of each element in the array.
 * @param[in]  count       The number of elements in the array.
 *
 * @return A new GArray.
 */

GArray *
VMTools_WrapArray(gconstpointer data,
                  guint elemSize,
                  guint count)
{
   GArray *array;

   array = g_array_sized_new(FALSE, TRUE, elemSize, count);
   memcpy(array->data, data, elemSize * count);
   array->len = count;

   return array;
}


/**
 * Library constructor. Calls any needed initialization functions.
 *
 * @param[in]  lib      The library handle (Win32 only).
 */

#if defined(__GNUC__)
__attribute__((constructor))
#endif
static void
VMToolsDllInit(void *lib)
{
   Bool success;
#if defined(_WIN32)
   WiperInitData wiperData;
   CoreDump_SetUnhandledExceptionFilter();
   VMTools_ResetLogging(FALSE);
   wiperData.resourceModule = lib;
   success = (NetUtil_LoadIpHlpApiDll() == ERROR_SUCCESS);
   ASSERT(success);
   success = Wiper_Init(&wiperData);
   ASSERT(success);
#else
   VMTools_ResetLogging(FALSE);
   success = Wiper_Init(NULL);
   ASSERT(success);
#endif
}


/**
 * Library destructor. Uninitializes any libraries that need to be cleaned up.
 */

#if defined(__GNUC__)
__attribute__((destructor))
#endif
static void
VMToolsDllFini(void)
{
#if defined(_WIN32)
   NetUtil_FreeIpHlpApiDll();
#endif
   VMTools_ResetLogging(TRUE);
}


/**
 * Frees a pointer allocated by the vmtools library.
 *
 * @param[in] ptr Pointer to memory to be freed.
 */

void
vm_free(void *ptr)
{
   free(ptr);
}


#if defined(_WIN32)
/**
 * Windows initialization callback. Calls the library's constructor or
 * destructor, depending on what is being done.
 *
 * @param[in]  hinstDLL    The library handle.
 * @param[in]  fdwReason   Why the callback is being called.
 * @param[in]  lpvReserved Unused.
 *
 * @return TRUE.
 */

BOOL WINAPI
DllMain(HINSTANCE hinstDLL,
        DWORD fdwReason,
        LPVOID lpvReserved)
{
   switch (fdwReason) {
   case DLL_PROCESS_ATTACH:
      VMToolsDllInit(hinstDLL);
      break;

   case DLL_PROCESS_DETACH:
      VMToolsDllFini();
      break;
   }
   return TRUE;
}
#endif

