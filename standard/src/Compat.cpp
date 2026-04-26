#include <C4Include.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fnmatch.h>
#include <stdint.h>
#include <map>

struct _findstate_t {
    DIR* dir;
    char pattern[260];
    char path[260];
};

static std::map<intptr_t, _findstate_t*> g_findHandles;
static intptr_t g_nextHandle = 1;

extern "C" {

intptr_t _findfirst(const char* pattern, struct _finddata_t* data) {
    _findstate_t* state = new _findstate_t();
    
    const char* lastSlash = strrchr(pattern, '/');
    if (!lastSlash) lastSlash = strrchr(pattern, '\\');

    if (lastSlash) {
        int len = lastSlash - pattern;
        if (len > 259) len = 259;
        strncpy(state->path, pattern, len);
        state->path[len] = '\0';
        strcpy(state->pattern, lastSlash + 1);
    } else {
        strcpy(state->path, ".");
        strcpy(state->pattern, pattern);
    }

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

int _findnext(intptr_t handle, struct _finddata_t* data) {
    if (g_findHandles.find(handle) == g_findHandles.end()) return -1;
    _findstate_t* state = g_findHandles[handle];
    struct dirent* entry;

    while ((entry = readdir(state->dir)) != NULL) {
        if (fnmatch(state->pattern, entry->d_name, 0) == 0) {
            strcpy(data->name, entry->d_name);
            
            char fullPath[520];
            sprintf(fullPath, "%s/%s", state->path, entry->d_name);
            struct stat st;
            if (stat(fullPath, &st) == 0) {
                data->size = st.st_size;
                data->attrib = 0;
                if (S_ISDIR(st.st_mode)) data->attrib |= 0x10; // _A_SUBDIR
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
    if (g_findHandles.find(handle) == g_findHandles.end()) return;
    _findstate_t* state = g_findHandles[handle];
    if (state->dir) closedir(state->dir);
    delete state;
    g_findHandles.erase(handle);
}

}
