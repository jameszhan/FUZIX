#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static uint8_t use[256];

struct section {
	struct section *next;
	char name[64];
	char code;
	unsigned int start;
	unsigned int len;
	int flags;
#define KNOW_START	1
#define KNOW_SIZE	2
};

static struct section *head;

static struct section *find_create(const char *name)
{
	struct section *s = head;
	while (s) {
		if (strcmp(s->name, name) == 0)
			return s;
		s = s->next;
	}
	s = malloc(sizeof(struct section));
	if (s == NULL) {
		fputs("Out of memory.\n", stderr);
		exit(1);
	}
	s->flags = 0;
	strncpy(s->name, name, 64);
	s->name[63] = 0;
	s->next = head;
	head = s;
	return s;
}

static char code_for(const char *name)
{
	if (strcmp(name, "CODE") == 0)
		return '1';
	if (strcmp(name, "CODE1") == 0)
		return '1';
	if (strcmp(name, "CODE2") == 0)
		return '2';
	if (strcmp(name, "CODE3") == 0)
		return '3';
	if (strcmp(name, "CODE4") == 0)
		return '4';
	if (strcmp(name, "CODE5") == 0)
		return '5';
	if (strcmp(name, "CODE6") == 0)
		return '6';
	if (strcmp(name, "VIDEO") == 0)
		return 'V';
	if (strcmp(name, "FONT") == 0)
		return 'F';
	if (strcmp(name, "INITIALIZED") == 0)
		return 'I';
	if (strcmp(name, "HOME") == 0)
		return 'H';
	if (strcmp(name, "DISCARD") == 0)
		return 'X';
	if (strcmp(name, "DATA") == 0)
		return 'D';
	if (strcmp(name, "BUFFERS") == 0)
		return 'B';
	if (strcmp(name, "COMMONMEM") == 0)
		return 'S';
	if (strcmp(name, "COMMONDATA") == 0)
		return 's';
	if (strcmp(name, "CONST") == 0)
		return 'C';
	if (strcmp(name, "INITIALIZER") == 0)
		return 'i';
	return '?';
}

static void learn_size(const char *size, const char *name)
{
	struct section *s = find_create(name);
	if (sscanf(size, "%8X", &s->len) != 1) {
		fprintf(stderr, "Invalid size '%s'.\n", size);
		exit(1);
	}
	s->flags |= KNOW_SIZE;
}

static void learn_start(const char *addr, const char *name)
{
	struct section *s = find_create(name);
	if (sscanf(addr, "%8X", &s->start) != 1) {
		fprintf(stderr, "Invalid start '%s'.\n", addr);
		exit(1);
	}
	s->flags |= KNOW_START;
}

static void mark_map(void)
{
	unsigned int base, end;
	struct section *s = head;
	while (s) {
		if (s->flags != (KNOW_SIZE | KNOW_START)) {
			fprintf(stderr, "Incomplete section '%s'.\n",
				s->name);
			exit(1);
		}
		if (s->len) {
			s->code = code_for(s->name);
			base = (s->start + 127) >> 8;
			end = (s->start + s->len + 127) >> 8;
			while (base <= end) {
				use[base++] = s->code;
			}
		}
		s = s->next;
	}
}

int main(int argc, char *argv[])
{
	char buf[512];
	int i;
	int r;

	memset(use, '#', 256);

	while (fgets(buf, 511, stdin)) {
		char *p1 = strtok(buf, " \t\n");
		char *p2 = NULL;

		if (p1)
			p2 = strtok(NULL, " \t\n");

		if (p1 == NULL || p2 == NULL)
			continue;

		if (strncmp(p2, "l__", 3) == 0)
			learn_size(p1, p2 + 3);
		if (strncmp(p2, "s__", 3) == 0)
			learn_start(p1, p2 + 3);
	}
	mark_map();
	for (r = 0; r < 4; r++) {
		for (i = 0; i < 256; i += 4) {
			putchar(use[i + r]);
			if ((i & 0x3C) == 0x3C)
				putchar(' ');
		}
		putchar('\n');
	}
	exit(0);
}
