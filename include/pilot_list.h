#ifndef _pilot_list
#define _pilot_list(type, name) struct _pilot_list_##type {\
			struct type *item; \
			struct _pilot_list_##type *next; \
			struct _pilot_list_##type *it; \
		} name
#define pilot_list_append(list, entry) do { typeof (list) *it = &list; \
			while (it->next) it = it->next; \
			it->item = entry; \
			it->next = malloc(sizeof(typeof (list))); \
			it = it->next; \
			memset(it, 0, sizeof(typeof (list))); \
		} while(0)
#define pilot_list_first(list) (list.next)?list.next->item:NULL; list.it = list.next
#define pilot_list_next(list) (list.it->next)?list.it->next->item:NULL;list.it = list.it->next;
#define pilot_list_foreach(list, func, data) do { typeof (list) *it = &list; \
			while (it->next) { \
				func(data, it->item); \
				it = it->next; \
			} \
		} while(0)
#define pilot_list_destroy(list) do { typeof (list) *it = &list; \
			it = it->next; \
			while (it->next) { \
				typeof (list) *tmp = it->next; \
				free(it); \
				it = tmp; \
			} \
			free(it); \
		} while(0)
#endif
