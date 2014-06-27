#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>

#include <pilot_mods.h>
#include <pilot_list.h>

struct _pilot_mods_internal
{
	char *name;
	void *handle;
	struct pilot_mods *mods;
};
static _pilot_list(_pilot_mods_internal, g_mods); 
static char *g_mods_path = NULL;
static int g_mods_appid = -1;

static struct _pilot_mods_internal *
_pilot_mods_internal_create(char *name, void *handle, struct pilot_mods *mods);
static int
_pilot_mods_load_dir(long flags, short type, short version);
static int
_pilot_mods_load(char *name, long flags, short type, short version);
static void *
_pilot_mods_load_lib(char *name);
static int
_pilot_mods_check(struct pilot_mods *mods, short type, short version);

int
pilot_mods_load(char *path, long flags, short appid, short type, short version)
{
	if (!g_mods_path && path)
	{
		g_mods_path = malloc(strlen(path)+1);
		strcpy(g_mods_path, path);
	}
	if (!g_mods_path)
		return -1;
	if (path && strcmp(g_mods_path, path))
		return -1;
	g_mods_appid = appid;
	return _pilot_mods_load_dir(flags, type, version);
}

static struct _pilot_mods_internal *
_pilot_mods_internal_create(char *name, void *handle, struct pilot_mods *mods)
{
		struct _pilot_mods_internal *thiz = malloc(sizeof(*thiz));
		thiz->name = malloc(strlen(name)+1);
		strcopy(thiz->name, name);
		thiz->handle = handle;
		thiz->mods = mods;
		return thiz;
}

static int
_pilot_mods_load_dir(long flags, short type, short version)
{
	int ret = 0;
	DIR *dir = NULL;
	struct dirent *entry;
	dir = opendir(g_mods_path);
	if (dir == NULL)
	{
		LOG_DEBUG("%s", strerror(errno));
		return -errno;
	}
	while ((entry = readdir(dir)) != NULL)
	{
		if (strstr(entry->d_name, ".so") != NULL)
		{
			_pilot_mods_load(entry->d_name, flags, type, version);
		}
	}
	return ret;
}

static void *
_pilot_mods_load_lib(char *name)
{
	void *handle;

	char *fullpath = NULL;

	fullpath = malloc(strlen(g_mods_path) + strlen(name) + 2);
	sprintf(fullpath, "%s/%s", g_mods_path, name);

	handle = dlopen(fullpath, RTLD_LAZY);
	free(fullpath);
	if (!handle)
	{
		LOG_DEBUG("error on plugin loading err : %s\n",dlerror());
		return NULL;
	}
	return handle;
}

static int
_pilot_mods_load(char *name, long flags, short type, short version)
{
	void *handle;
	struct pilot_mods *info;
	int ret = 0;

	handle = _pilot_mods_load_lib(name);

	info = dlsym(handle, PILOT_MODS_INFO);
	if (info != NULL)
	{
		if (_pilot_mods_check(info, type, version))
		{
			dclose(handle);
			return -1;
		}

		if (flags & PILOT_MODS_FLAGSLAZY)
		{
			dclose(handle);
			handle = NULL;
		}
		struct _pilot_mods_internal *mods = 
			_pilot_mods_internal_create(name, handle, info);
		pilot_list_append(g_mods, mods);
	}
	else
	{
		dclose(handle);
		ret = -errno;
	}
	return ret;
}

static int
_pilot_mods_check(struct pilot_mods *mods, short type, short version)
{
	int ret = 0;
	if (mods->appid != g_mods_appid)
	{
		ret = -1;
	}
	if ((type != 0) && (mods->type != type))
	{
		ret = -1;
	}
	if ((version != 0) &&
			(((version && 0xFF00) != (mods->version && 0xFF00)) ||
			((version && 0x00FF) < (mods->version && 0x00FF))))
	{
		ret = -1;
	}
	return ret;
}

struct pilot_mods *
pilot_mods_get(char *name)
{
	struct _pilot_mods_internal *mods = pilot_list_first(g_mods);
	while (strcmp(mods->name, name))
	{
		mods = pilot_list_next(g_mods);
	}
	if (!mods->handle)
		mods->handle = _pilot_mods_load_lib(mods->name);

	return mods->mods;
}

struct pilot_mods *
pilot_mods_first(short type, short version)
{
	struct _pilot_mods_internal *mods = pilot_list_first(g_mods);
	while (_pilot_mods_check(mods->mods, type, version))
	{
		mods = pilot_list_next(g_mods);
	}
	if (!mods->handle)
		mods->handle = _pilot_mods_load_lib(mods->name);

	return mods->mods;
}

struct pilot_mods *
pilot_mods_next(short type, short version)
{
	struct _pilot_mods_internal *mods = pilot_list_next(g_mods);
	while (_pilot_mods_check(mods->mods, type, version))
	{
		mods = pilot_list_next(g_mods);
	}
	if (!mods->handle)
		mods->handle = _pilot_mods_load_lib(mods->name);

	return mods->mods;
}
