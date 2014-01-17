#ifndef __PILOT_MODS_H__
#define __PILOT_MODS_H__
struct pilot_mods
{
	long appid;
	short type;
	short version;
	void *api;
};

#ifndef PILOT_MODS
#define PILOT_MODS_FLAGSLAZY 0x0001

int
pilot_mods_load(char *path, long flags, long appid, short type, short version);
struct pilot_mods *
pilot_mods_first(short type, short version);
struct pilot_mods *
pilot_mods_next(short type, short version);
#endif
#endif
