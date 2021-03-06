#define _GNU_SOURCE
#include <dlfcn.h>
#include <fcntl.h>
#include <kdb.h>
#include <libgen.h>
#include <limits.h>
#include <linux/limits.h>
#include <pwd.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <kdb.h>
#include <kdbmodule.h>
#include <kdbprivate.h>

#define PRELOAD_PATH "/preload/open"
#define TV_MAX_DIGITS 26

struct _Node
{
	char * key;
	char * value;
	unsigned short oflags;
	struct _Node * next;
};
typedef struct _Node Node;
static Node * head = NULL;

static void canonicalizePath (char * buffer, char * toAppend)
{
	char * destPtr = buffer + strlen (buffer);
	for (unsigned int i = 0; i < strlen (toAppend); ++i)
	{
		if (!strncmp ((toAppend + i), "../", 3))
		{
			i += 2;
			const char * dir = dirname (buffer);
			size_t dirLen = strlen (dir);
			destPtr = buffer + dirLen;
			if (strcmp (dir, "/")) *destPtr++ = '/';
			*destPtr = '\0';
			continue;
		}
		else if (!strncmp ((toAppend + i), "./", 2))
		{
			++i;
			continue;
		}
		else if (!strncmp ((toAppend + i), "//", 2))
		{
			continue;
		}
		else
		{
			*destPtr++ = toAppend[i];
		}
	}
}

static char * createAbsolutePath (const char * path, const char * cwd)
{
	if (path[0] == '/')
		return strdup (path);
	else
	{
		char * absPath = NULL;
		size_t pathlen;
		char * pathPtr = NULL;
		if (path[0] == '~')
		{
			struct passwd * pwd = getpwuid (getuid ());
			pathlen = strlen (path) + strlen (pwd->pw_dir) + 2;
			absPath = calloc (pathlen, sizeof (char));
			snprintf (absPath, pathlen, "%s/", pwd->pw_dir);
			pathPtr = (char *)(path + 2);
		}
		else
		{
			pathlen = strlen (path) + strlen (cwd) + 2;
			absPath = calloc (pathlen, sizeof (char));
			snprintf (absPath, pathlen, "%s/", cwd);
			pathPtr = (char *)path;
		}
		canonicalizePath (absPath, pathPtr);
		return absPath;
	}
}

static void init (void) __attribute__ ((constructor));
static void cleanup (void) __attribute__ ((destructor));
void init ()
{
	char cwd[PATH_MAX];
	getcwd (cwd, PATH_MAX);
	KeySet * tmpKS = ksNew (0, KS_END);
	Key * parentKey = keyNew (PRELOAD_PATH, KEY_CASCADING_NAME, KEY_END);
	Key * key;
	KDB * handle = kdbOpen (parentKey);
	kdbGet (handle, tmpKS, parentKey);
	KeySet * ks = ksCut (tmpKS, parentKey);
	ksRewind (ks);
	ssize_t size = ksGetSize (ks);
	if (size <= 1) goto CleanUp;
	Node * current = head;
	ksNext (ks); // skip head
	while ((key = ksNext (ks)) != NULL)
	{
		if (!keyIsDirectBelow (parentKey, key)) continue;
		Node * tmp = calloc (1, sizeof (Node));
		tmp->key = createAbsolutePath (keyBaseName (key), cwd);
		tmp->value = createAbsolutePath (keyString (key), cwd);
		tmp->oflags = (unsigned short)-1;
		Key * lookupKey = keyDup (key);
		keyAddBaseName (lookupKey, "readonly");
		Key * found = ksLookup (ks, lookupKey, 0);
		if (found)
		{
			if (!strcmp (keyString (found), "1"))
			{
				tmp->oflags = O_RDONLY;
			}
		}
		keyDel (lookupKey);
		tmp->next = NULL;
		if (current == NULL)
		{
			head = tmp;
			current = head;
		}
		else
		{
			current->next = tmp;
			current = current->next;
		}
	}
CleanUp:
	ksAppend (tmpKS, ks);
	ksDel (tmpKS);
	ksDel (ks);
	kdbClose (handle, parentKey);
	keyDel (parentKey);
}


void cleanup ()
{
	Node * current = head;
	while (current)
	{
		Node * tmp = current;
		free (current->key);
		free (current->value);
		current = current->next;
		free (tmp);
	}
}

static Node * resolvePathname (const char * pathname)
{
	Node * node = NULL;
	if (pathname)
	{
		char cwd[PATH_MAX];
		getcwd (cwd, PATH_MAX);
		char * resolvedPath = NULL;
		if (pathname[0] != '/')
		{
			resolvedPath = createAbsolutePath (pathname, cwd);
		}
		else
		{
			resolvedPath = calloc (strlen (pathname), sizeof (char));
			size_t size = sizeof (resolvedPath);
			memset (resolvedPath, 0, size);
			canonicalizePath (resolvedPath, (char *)pathname);
		}
		Node * current = head;
		while (current)
		{
			if (!strcmp (current->key, resolvedPath))
			{
				node = current;
				break;
			}
			current = current->next;
		}
		free (resolvedPath);
	}
	return node;
}


typedef int (*orig_open_f_type) (const char * pathname, int flags, ...);

typedef union {
	void * d;
	orig_open_f_type f;
} Symbol;


int open (const char * pathname, int flags, ...)
{
	Node * node = resolvePathname (pathname);
	const char * newPath = NULL;
	unsigned short newFlags = (unsigned short)-1;
	if (!node)
	{
		newPath = pathname;
	}
	else
	{
		newPath = node->value;
		newFlags = node->oflags;
	}
	if (newFlags == O_RDONLY)
	{
		flags = (flags & (~(0 | O_WRONLY | O_APPEND)));
	}
	Symbol orig_open;
	orig_open.d = dlsym (RTLD_NEXT, "open");

	int fd;
	if (flags & O_CREAT)
	{
		int mode;
		va_list argptr;
		va_start (argptr, flags);
		mode = va_arg (argptr, int);
		va_end (argptr);
		fd = orig_open.f (newPath, flags, mode);
	}
	else
	{
		fd = orig_open.f (newPath, flags);
	}
	return fd;
}
int open64 (const char * pathname, int flags, ...)
{
	Node * node = resolvePathname (pathname);
	const char * newPath = NULL;
	unsigned short newFlags = (unsigned short)-1;
	if (!node)
	{
		newPath = pathname;
	}
	else
	{
		newPath = node->value;
		newFlags = node->oflags;
	}
	if (newFlags == O_RDONLY)
	{
		flags = (flags & (~(0 | O_WRONLY | O_APPEND)));
	}

	Symbol orig_open64;
	orig_open64.d = dlsym (RTLD_NEXT, "open64");

	int fd;
	if (flags & O_CREAT)
	{
		int mode;
		va_list argptr;
		va_start (argptr, flags);
		mode = va_arg (argptr, int);
		va_end (argptr);
		fd = orig_open64.f (newPath, flags, mode);
	}
	else
	{
		fd = orig_open64.f (newPath, flags);
	}
	return fd;
}
