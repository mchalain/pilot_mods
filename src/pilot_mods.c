#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>

#include <pilot_mods.h>
#include <pilot_list.h>

struct pilot_string
{
	char *data;
};

struct pilot_string *
pilot_string_create(char *value, int flags)
{
	struct pilot_string *thiz = malloc(sizeof(*thiz) + strlen(value) + 1);
	thiz->data = ((char *)thiz) + sizeof(*thiz);
	strcpy(thiz->data, value);
	return thiz;
}

struct _pilot_mods_internal
{
	void *handle;
	struct pilot_mods *mods;
};
static _pilot_list(_pilot_mods_internal, g_mods); 
static _pilot_list(pilot_string, g_modnames); 

static struct _pilot_mods_internal *
_pilot_mods_internal_create(void *handle, struct pilot_mods *mods);
static int
_pilot_mods_load_dir(char *path, long flags, long appid, short type, short version);
static int
_pilot_mods_load_lib(char *path, long flags, long appid, short type, short version);
static int
_pilot_mods_check(struct pilot_mods *mods, short type, short version);

int
pilot_mods_load(char *path, long flags, long appid, short type, short version)
{
	return _pilot_mods_load_dir(path, flags, appid, type, version);
}

static struct _pilot_mods_internal *
_pilot_mods_internal_create(void *handle, struct pilot_mods *mods)
{
		struct _pilot_mods_internal *thiz = malloc(sizeof(*thiz));
		thiz->handle = handle;
		thiz->mods = mods;
		return thiz;
}

static int
_pilot_mods_load_dir(char *path, long flags, long appid, short type, short version)
{
	int ret = 0;
	DIR *dir = NULL;
	struct dirent *entry;
	dir = opendir(path);
	if (dir == NULL)
	{
		LOG_DEBUG("%s", strerror(errno));
		return -errno;
	}
	while ((entry = readdir(dir)) != NULL)
	{
		if (strstr(entry->d_name, ".so") != NULL)
		{
			if (flags && PILOT_MODS_FLAGSLAZY)
			{
				struct pilot_string *name = pilot_string_create(entry->d_name, 0);
				pilot_list_append(g_modnames, name);
			}
			else
			{
				char *completpath = malloc(strlen(path) + strlen(entry->d_name) + 2);
				sprintf(completpath, "%s/%s", path, entry->d_name);
				_pilot_mods_load_lib(completpath, flags, appid, type, version);
				free(completpath);
			}
		}
	}
	return ret;
}

static int
_pilot_mods_load_lib(char *path, long flags, long appid, short type, short version)
{
	void *handle;
	struct pilot_mods *info;
	int ret = 0;

	handle = dlopen(path, RTLD_LAZY);
	if (!handle)
	{
		LOG_DEBUG("error on plugin loading err : %s\n",dlerror());
		return -errno;
	}
	info = dlsym(handle, "pilot_mods_info");
	if (info != NULL)
	{
		if ((info->appid != appid) ||
			((type != 0) && (info->type != type)) ||
			((version != 0) && 
				(((version && 0xFF00) != (info->version && 0xFF00)) ||
				((version && 0x00FF) < (info->version && 0x00FF)))))
		{
			dclose(handle);
			return -1;
		}

		struct _pilot_mods_internal *mods = 
			_pilot_mods_internal_create(handle, info);
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
pilot_mods_first(short type, short version)
{
	struct _pilot_mods_internal *mods = pilot_list_first(g_mods);
	while (_pilot_mods_check(mods->mods, type, version))
	{
		mods = pilot_list_next(g_mods);
	}
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
	return mods->mods;
}