#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include "mpq.h"

void mpq_init(void)
{
	init_crypt();
}

struct mpq *mpq_open(const char *path)
{
	int infile, r;
	int len;
	const char *data;
	struct stat statinfo;
	struct mpq *m;

	infile = open(path, O_RDONLY);
	if(infile == -1)
		return NULL;

	r = fstat(infile, &statinfo);
	if(r == -1)
		return NULL;

	len = (unsigned int)statinfo.st_size;

	data = mmap(NULL, len, PROT_READ, MAP_PRIVATE, infile, 0);

	if(data == MAP_FAILED)
		return NULL;

	m = parse_replay(data, 0);
	m->fd = infile;

    return m;
}

void mpq_close(struct mpq *m)
{
	close(m->fd);
	free(m->bt);
	free(m->ht);
}

int mpq_find_file(struct mpq *m, const char *filename, int *size)
{
	uint32_t hasha, hashb, offset;
	uint64_t hash;
	int block;

	/* Generate bucket and hash from filename */
	offset = hash_string(filename, 0) & (m->ht_size - 1);
	hasha  = hash_string(filename, 0x100);
	hashb  = hash_string(filename, 0x200);

	hash = ((uint64_t)hasha << 32) | hashb;

	/* Check all the entries in the hash bucket */
	while(1)
	{
		/* 0xFFF.. means 'end of bucket', and we didn't find the filename in the table */
		if(m->ht[offset].hash == 0xFFFFFFFFFFFFFFULL)
			return 0;

		/* Hashes match, we found the file in the hash table */
		if(hash == m->ht[offset].hash)
			break;

		/* Check next bucket entry */
		offset++;
	}

	/* Use the hash table entry we found to tell us what index the file has
	 * into the block table.
	 */
	block = m->ht[offset].index;

	/* Fill out the given pointer with the size of the
	 * block so that memory can be allocated by the caller.
	 */
	if(size)
		*size = m->bt[block].usize;

	return block;
}

char *mpq_read_file(struct mpq *m, int block_index, char *out)
{
	int usize, csize;
	const char *data;

	data  = m->bt[block_index].data;
	usize = m->bt[block_index].usize;
	csize = m->bt[block_index].csize;

	switch(data[0])
	{
		case 0x0:	/* Not encrypted */
			memcpy(out, data+1, usize);
		break;
		case 0x10:	/* BZ2 */
			decompress_bz2(data+1, out, csize, usize);
		break;
		default:
			return NULL;
		break;

	}

	return out;
}
