lib-y+=pilot_mods
pilot_mods_SOURCES=pilot_mods.c
pilot_mods-objs=$(pilot_mods_SOURCES:%.c=%.o)
pilot_mods_CFLAGS=-I./include
pilot_mods_LDFLAGS=-ldl
$(foreach s, $(pilot_mods-objs), $(eval $(s:%.o=%)_CFLAGS+=$(pilot_mods_CFLAGS)) )
