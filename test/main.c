#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>
#include "sc2mpq.h"

static void uppercase(char *s)
{
	while(*s)
	{
		*s = toupper(*s);
		s++;
	}
}

static void parse_replay(const char *data, int size)
{
	/* Parse replay file */

}

int main(int argc, char *argv[])
{
	struct mpq *m;
	char *data;
	int fd, size, of;
	char *filename;

	if(argc != 3)
	{
		printf("Usage:\n\t%s: file filename\n", argv[0]);
		return EXIT_SUCCESS;
	}

	mpq_init();

	m = mpq_open(argv[1]);
	if(!m)
	{
		return EXIT_FAILURE;
	}

	filename = strdup(argv[2]);
	uppercase(filename);

	fd = mpq_find_file(m, filename, &size);
	if(!fd)
	{
		printf("File not found in archive.\n");
		return EXIT_FAILURE;
	}

	data = malloc(size);
	mpq_read_file(m, fd, data);

#ifdef DUMP
	char buf[256];
	sprintf(buf, "%s.events", argv[1]);
	of = open(buf, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
	write(of, data, size);
	close(of);
#endif
	parse_replay(data, size);

	free(data);

	return EXIT_SUCCESS;
}
