#include "eng_list.h"

#define NULL ((void*)0)

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

struct ops_entry {
	const char *version;
	void *ops;
};

struct ops_list {
	struct list_head head;
	struct ops_entry *entry;
};
