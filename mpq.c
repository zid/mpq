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

    return m;
}

int mpq_find_file(struct mpq *m, const char *filename, int *size)
{
	uint32_t hasha, hashb, offset;
	int block;

	offset = hash_string(filename, 0) & (m->ht_size - 1);
	hasha  = hash_string(filename, 0x100);
	hashb  = hash_string(filename, 0x200);

	block = m->ht[offset].index;

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
