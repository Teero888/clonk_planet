#include <C4Include.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fnmatch.h>
#include <stdint.h>
#include <map>

struct _findstate_t {
  DIR *dir;
  char pattern[260];
  char path[260];
};

static std::map<intptr_t, _findstate_t *> g_findHandles;
static intptr_t g_nextHandle = 1;

extern "C" {

intptr_t _findfirst(const char *pattern, struct _finddata_t *data) {
  char cwd[1024];
  getcwd(cwd, sizeof(cwd));
  // printf("_findfirst: %s (cwd: %s)\n", pattern, cwd);
  _findstate_t *state = new _findstate_t();

  const char *lastSlash = strrchr(pattern, '/');
  if (!lastSlash)
    lastSlash = strrchr(pattern, '\\');

  if (lastSlash) {
    int len = lastSlash - pattern;
    if (len > 259)
      len = 259;
    if (len == 0) {
      strcpy(state->path, "/");
    } else {
      strncpy(state->path, pattern, len);
      state->path[len] = '\0';
    }
    strcpy(state->pattern, lastSlash + 1);
  } else {
    strcpy(state->path, ".");
    strcpy(state->pattern, pattern);
  }

  // printf("_findfirst: %s (path: %s, pattern: %s)\n", pattern, state->path,
  //        state->pattern);

  state->dir = opendir(state->path);
  if (!state->dir) {
    delete state;
    return -1;
  }

  intptr_t h = g_nextHandle++;
  g_findHandles[h] = state;

  if (_findnext(h, data) == 0) {
    return h;
  }

  _findclose(h);
  return -1;
}

int _findnext(intptr_t handle, struct _finddata_t *data) {
  if (g_findHandles.find(handle) == g_findHandles.end())
    return -1;
  _findstate_t *state = g_findHandles[handle];
  struct dirent *entry;

  while ((entry = readdir(state->dir)) != NULL) {
    // printf("  checking: %s against %s\n", entry->d_name, state->pattern);
    if (fnmatch(state->pattern, entry->d_name, FNM_CASEFOLD) == 0) {
      strcpy(data->name, entry->d_name);

      char fullPath[520];
      sprintf(fullPath, "%s/%s", state->path, entry->d_name);
      struct stat st;
      if (stat(fullPath, &st) == 0) {
        data->size = st.st_size;
        data->attrib = 0;
        if (S_ISDIR(st.st_mode))
          data->attrib |= 0x10; // _A_SUBDIR
        else
          data->attrib |= 0x20; // _A_ARCH
        data->time_write = st.st_mtime;
        data->time_access = st.st_atime;
        data->time_create = st.st_ctime;
        return 0;
      }
    }
  }

  return -1;
}

void _findclose(intptr_t handle) {
  if (g_findHandles.find(handle) == g_findHandles.end())
    return;
  _findstate_t *state = g_findHandles[handle];
  if (state->dir)
    closedir(state->dir);
  delete state;
  g_findHandles.erase(handle);
}
}

#include <pthread.h>
#include <unistd.h>
#include <deque>
#include <mutex>

struct C4ThreadState {
  pthread_t thread;
  DWORD(WINAPI *start_address)(void *);
  void *parameter;
  DWORD exit_code;
  bool is_active;
};

static void *C4ThreadWrapper(void *arg) {
  C4ThreadState *state = (C4ThreadState *)arg;
  // pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  state->is_active = true;
  state->exit_code = state->start_address(state->parameter);
  state->is_active = false;
  return NULL;
}

BOOL GetExitCodeThread(HANDLE hThread, LPDWORD lpExitCode) {
  if (!hThread || !lpExitCode) return FALSE;
  C4ThreadState *state = (C4ThreadState *)hThread;
  if (state->is_active) {
    *lpExitCode = STILL_ACTIVE;
  } else {
    *lpExitCode = state->exit_code;
  }
  return TRUE;
}

void Sleep(DWORD dwMilliseconds) {
  usleep(dwMilliseconds * 1000);
}

BOOL TerminateThread(HANDLE hThread, DWORD dwExitCode) {
  if (!hThread) return FALSE;
  C4ThreadState *state = (C4ThreadState *)hThread;
  pthread_cancel(state->thread);
  pthread_join(state->thread, NULL);
  delete state;
  return TRUE;
}

HANDLE CreateThread(void *lpThreadAttributes, DWORD dwStackSize, DWORD(WINAPI *lpStartAddress)(void *), void *lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId) {
  C4ThreadState *state = new C4ThreadState();
  state->start_address = lpStartAddress;
  state->parameter = lpParameter;
  state->exit_code = 0;
  state->is_active = false;
  
  if (pthread_create(&state->thread, NULL, C4ThreadWrapper, state) != 0) {
    delete state;
    return NULL;
  }
  
  if (lpThreadId) {
    *lpThreadId = (DWORD)(uintptr_t)state->thread;
  }
  
  return (HANDLE)state;
}

static std::deque<MSG> g_msgQueue;
static std::mutex g_msgMutex;

BOOL PeekMessage(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) {
  std::lock_guard<std::mutex> lock(g_msgMutex);
  if (g_msgQueue.empty()) return FALSE;
  if (lpMsg) *lpMsg = g_msgQueue.front();
  if (wRemoveMsg & PM_REMOVE) g_msgQueue.pop_front();
  return TRUE;
}

BOOL GetMessage(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax) {
  while (TRUE) {
    {
      std::lock_guard<std::mutex> lock(g_msgMutex);
      if (!g_msgQueue.empty()) {
        if (lpMsg) *lpMsg = g_msgQueue.front();
        g_msgQueue.pop_front();
        return TRUE;
      }
    }
    usleep(1000);
  }
}

BOOL TranslateMessage(const MSG *lpMsg) { return FALSE; }

LRESULT DispatchMessage(const MSG *lpMsg) {
  if (!lpMsg) return 0;
  switch (lpMsg->message) {
    case WM_NET_REQUESTJOIN:
      Game.RequestJoin((C4Stream*)lpMsg->lParam);
      break;
    case WM_NET_REQUESTREFERENCE:
      Game.RequestNetworkReference((C4Stream*)lpMsg->lParam);
      break;
    case WM_USER_CREATEREFERENCE:
      Game.Network.CreateReference((const char*)lpMsg->lParam);
      break;
    case WM_USER_LOG:
      Log((const char*)lpMsg->lParam);
      break;
    case WM_USER_NETLOG:
      NetLog((const char*)lpMsg->lParam);
      break;
  }
  return 0;
}

BOOL IsDialogMessage(HWND hWnd, LPMSG lpMsg) { return FALSE; }

BOOL PostMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
  std::lock_guard<std::mutex> lock(g_msgMutex);
  MSG msg;
  msg.hwnd = hWnd;
  msg.message = Msg;
  msg.wParam = wParam;
  msg.lParam = lParam;
  g_msgQueue.push_back(msg);
  return TRUE;
}

LRESULT SendMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
  // Simple synchronous-like SendMessage for now
  PostMessage(hWnd, Msg, wParam, lParam);
  return 0;
}
