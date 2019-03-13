#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pq_util.h"
#include "sprd_fts_type.h"
#include "sprd_fts_log.h"


void HexStrstoInt(char *buf, int size)
{
	int *temp = (int*) buf;
	u32 i = 0;
	u32 j = 0;
	for(; i < size; i++){
		temp[i] = strtol(buf + j, NULL, 16);
		j += 9;
	}
}

int eng_diag_encode7d7e(char *buf, int len, int *extra_len) {
	int i, j;
	char tmp;


	for (i = 0; i < len; i++) {
		if ((buf[i] == 0x7d) || (buf[i] == 0x7e)) {
			tmp = buf[i] ^ 0x20;
			buf[i] = 0x7d;
			for (j = len; j > i + 1; j--) {
				buf[j] = buf[j - 1];
			}
			buf[i + 1] = tmp;
			len++;
			(*extra_len)++;

			for (j = 0; j < len; j++) {
				ENG_LOG("%x,", buf[j]);
			}
		}
	}

	return len;
}

void *tune_ops_attach(const char *str, struct list_head *head)
{
	struct ops_list *list;
	const char *ver;

	list_for_each_entry(list, head, head) {
		ver = list->entry->version;
		ENG_LOG("PQ versioniii %s", str);
		if (!strncmp(str, ver, strlen(ver)))
			return list->entry->ops;
	}

	return NULL;
}

int tune_ops_register(struct ops_entry *entry, struct list_head *head)
{
	struct ops_list *list;

	list = malloc(sizeof(struct ops_list));
	if (!list)
		return -1;

	list->entry = entry;
	list_add(&list->head, head);

	return 0;
}

