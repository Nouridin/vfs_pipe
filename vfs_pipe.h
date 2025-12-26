/* ==========================================================
 * VFS-PIPE - A Virtual Filesystem Variable Tracker
 * ==========================================================
 * Copyright (c) 2025 Nouridin
 * This software is licensed under the MIT License.
 * You are free to use, modify, and distribute this code 
 * as long as this copyright notice remains intact.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY 
 * OF ANY KIND, EXPRESS OR IMPLIED.
 * ==========================================================
 */

#define FUSE_USE_VERSION 26
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

// --- THE REGISTRY ---

typedef enum { VFS_INT, VFS_STRING } vfs_type;

typedef struct 
{
    char      name[64];                           
    void * ptr;      // Changed to void* to be generic
    vfs_type  type;     // Track if this entry is an INT or STRING
} vfs_entry;

static vfs_entry * registry        = NULL;
static int          entry_count     = 0;
static int          entry_capacity  = 0;

static int find_entry(const char * path) 
{
    for (int i = 0; i < entry_count; i++) {
        if (strcmp(path + 1, registry[i].name) == 0) return i;
    }
    return -1;
}

// --- FUSE CALLBACKS ---

static int vfs_getattr(const char * path, struct stat * stbuf) 
{
    memset(stbuf, 0, sizeof(struct stat));
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    } 

    if (find_entry(path) != -1) {
        stbuf->st_mode = S_IFREG | 0666;
        stbuf->st_nlink = 1;
        stbuf->st_size = 256; // Bigger buffer for strings
        return 0;
    }
    return -ENOENT;
}

static int vfs_readdir(const char * path, void * buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info * fi) 
{
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    for (int i = 0; i < entry_count; i++) {
        filler(buf, registry[i].name, NULL, 0);
    }
    return 0;
}

static int vfs_read(const char * path, char * buf, size_t size, off_t offset, struct fuse_file_info * fi) 
{
    int idx = find_entry(path);
    if (idx == -1) return -ENOENT;

    char str[256];
    int len;

    // Logic Switch: Handle based on type
    if (registry[idx].type == VFS_INT) {
        len = sprintf(str, "%d\n", *(int*)(registry[idx].ptr));
    } else {
        len = sprintf(str, "%s\n", (char*)(registry[idx].ptr));
    }

    if (offset < len) {
        if (offset + size > len) size = len - offset;
        memcpy(buf, str + offset, size);
        return size;
    }
    return 0;
}

static int vfs_write(const char * path, const char * buf, size_t size, off_t offset, struct fuse_file_info * fi) 
{
    int idx = find_entry(path);
    if (idx == -1) return -ENOENT;

    char tmp[256];
    size_t copy_size = (size < 255) ? size : 255;
    memcpy(tmp, buf, copy_size);
    tmp[copy_size] = '\0';

    // Logic Switch: Handle based on type
    if (registry[idx].type == VFS_INT) {
        *(int*)(registry[idx].ptr) = atoi(tmp);
    } else {
        // Strip the newline often added by 'echo'
        if (tmp[strlen(tmp)-1] == '\n') tmp[strlen(tmp)-1] = '\0';
        strcpy((char*)(registry[idx].ptr), tmp);
    }

    return size;
}

static int vfs_truncate(const char * path, off_t size) 
{
    return (find_entry(path) != -1) ? 0 : -ENOENT;
}

static struct fuse_operations vfs_oper = 
{
    .getattr  = vfs_getattr,
    .readdir  = vfs_readdir, 
    .read     = vfs_read,
    .write    = vfs_write,
    .truncate = vfs_truncate,
};

// --- API ---

static void vfs_grow_if_needed()
{
    if (entry_count >= entry_capacity) {
        entry_capacity = (entry_capacity == 0) ? 8 : entry_capacity * 2;
        registry = realloc(registry, sizeof(vfs_entry) * entry_capacity);
    }
}

void vfs_register_int(const char * name, int * var_ptr) 
{
    vfs_grow_if_needed();
    if (registry != NULL) {
        strncpy(registry[entry_count].name, name, 63);
        registry[entry_count].ptr  = (void*)var_ptr;
        registry[entry_count].type = VFS_INT;
        entry_count++;
    }
}

void vfs_register_str(const char * name, char * var_ptr) 
{
    vfs_grow_if_needed();
    if (registry != NULL) {
        strncpy(registry[entry_count].name, name, 63);
        registry[entry_count].ptr  = (void*)var_ptr;
        registry[entry_count].type = VFS_STRING;
        entry_count++;
    }
}

void * vfs_thread_func(void * arg) 
{
    char * argv[] = {"vfs_exe", "-f", (char*)arg, NULL};
    fuse_main(3, argv, &vfs_oper, NULL);
    return NULL;
}

void vfs_init(char * path) 
{
    char cleanup_cmd[128];
    sprintf(cleanup_cmd, "fusermount -u %s > /dev/null 2>&1", path);
    system(cleanup_cmd);

    pthread_t thread;
    pthread_create(&thread, NULL, vfs_thread_func, strdup(path));
}

void vfs_cleanup(char * path)
{
    char command[128];
    sprintf(command, "fusermount -u %s", path);
    system(command);

    if (registry != NULL) 
    {
        free(registry);
        registry = NULL;
    }
}