#include <glib.h>
#include <glib/gstdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "util.h"

static GRand *__rand = NULL;

GRand *__get_grand()
{
    if (__rand == NULL) {
        __rand = g_rand_new();
    }
    return __rand;
}

/**
 * Return the current timestamp as a long.
 *
 * @return the current timestamp.
 */
glong get_current_timestamp()
{
    GTimeVal gtv;
    g_get_current_time(&gtv);
    return gtv.tv_sec;
}

/**
 * Return a unique hexa identifier
 *
 * @return newly allocated string containing a unique id (free with g_free).
 */
gchar *get_unique_hexa_identifier()
{
    GRand *rnd = __get_grand();
    gchar *res = g_malloc(sizeof(gchar) * 33);
    guint32 ri;
    int i;
    for (i = 0 ; i < 4 ; i++) {
        ri = g_rand_int(rnd);
        sprintf(res + (i * 8) * sizeof(gchar),  "%08x", ri);
    }
    return res;
}


/**
 * Return the size (in bytes) of the given file path.
 *
 * @param file_path the file path.
 * @return the size (in bytes), -1 means error.
 */
glong get_file_size(const gchar *file_path) {
    GStatBuf buf;
    int res = g_stat(file_path, &buf);
    if (res != 0) {
        return -1;
    }
    return buf.st_size;
}

/**
 * Return the inode of the given file path.
 *
 * @param file_path the file path.
 * @return the file inode, -1 means error.
 */
glong get_file_inode(const gchar *file_path) {
    GStatBuf buf;
    int res = g_stat(file_path, &buf);
    if (res != 0) {
        return -1;
    }
    return buf.st_ino;
}

/**
 * Return the inode of the given file descriptor.
 *
 * @param fd file descriptor.
 * @return the file inode, -1 means error.
 */
glong get_fd_inode(int fd) {
    struct stat buf;
    int res = fstat(fd, &buf);
    if (res != 0) {
        return -1;
    }
    return buf.st_ino;
}

/**
 * Compute and append a strftime suffix.
 *
 * @param str original string.
 * @param strftime_suffix suffix to append to previous string (with strftime placeholders).
 * @return newly allocated string (free it with g_free) with resolved and appended suffix.
 */
gchar *compute_strftime_suffix(const gchar *str, const gchar *strftime_suffix) {
    time_t t;
    struct tm *tmp;
    t = time(NULL);
    tmp = localtime(&t);
    g_assert(tmp != NULL);
    char outstr[200];
    if (strftime(outstr, sizeof(outstr), strftime_suffix, tmp) == 0) {
        g_critical("problem with strftime on %s", strftime_suffix);
        return NULL;
    }
    return g_strdup_printf("%s%s", str, outstr);
}

/**
 * Create an empty file if it does not exist
 *
 * Note: if the file already exists, nothing is done and TRUE is returned.
 *
 * @param file_path file path.
 * @return TRUE if ok, FALSE if errors.
 */
gboolean create_empty(const gchar *file_path) {
    int fd2, close_res;
    while (TRUE) {
        fd2 = open(file_path, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fd2 < 0) {
            if (errno == EEXIST) {
                return TRUE;
            }
            if (errno == EINTR) {
                continue;
            }
            g_warning("can't create %s (errno: %i)", file_path, errno);
            return FALSE;
        } else {
            break;
        }
    }
    while (TRUE) {
        close_res = close(fd2);
        if (close_res < 0) {
            if (errno == EINTR) {
                continue;
            }
            g_warning("can't close %s (errno: %i)", file_path, errno);
            return FALSE;
        } else {
            break;
        }
    }
    return TRUE;
}