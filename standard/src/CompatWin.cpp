/* Parts of the compatibility layer (Compat.h) that need the real Windows API. This file must not
   include Compat.h: its emulated types clash with <windows.h>. */

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Path of the running executable (GetModuleFileName of Compat.h)
extern "C" int CompatGetExePath(char *buf, int size) {
  DWORD len = GetModuleFileNameA(NULL, buf, (DWORD)size);
  if (len == 0 || len >= (DWORD)size) return 0;
  return (int)len;
}

// Can the byte at ptr be read (IsBadReadPtr without its guard page side effects)?
extern "C" int CompatIsReadablePtr(const void *ptr) {
  MEMORY_BASIC_INFORMATION info;
  if (!VirtualQuery(ptr, &info, sizeof(info))) return 0;
  if (info.State != MEM_COMMIT) return 0;
  if (info.Protect & (PAGE_NOACCESS | PAGE_GUARD)) return 0;
  return 1;
}

#endif
