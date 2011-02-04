#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "mpq.h"

#define u32 uint32_t
#define off_t int

#define U8(x)  (*((unsigned char *)&x))
#define U16(x) (*((unsigned short *)&x))
#define U32(x) (*((unsigned int *)&x))
#define U64(x) (*((unsigned long long int *)&x))

static void read_block_table(struct mpq *m, const char *data)
{
	uint32_t *block_table;
	off_t offset;
	int i;

	offset     = U32(data[0x14]);
	m->bt_size = U32(data[0x1C]);

	block_table = malloc(sizeof(struct block_table) * m->bt_size);
	memcpy(block_table, &data[offset], m->bt_size * sizeof(struct block_table));

	/* The decryption function works only on 32bit values, so calculate how many
	 * iterations it needs via dividing the size of a block table entry by 4
	 * and then mutliplying the amount of block table entries we have.
	 */
	decrypt_table(block_table, m->bt_size * (sizeof(struct block_table) / sizeof(uint32_t)),
		"(BLOCK TABLE)");

	/* Convert our blob of memory into an array of block_table structs */
	m->bt = malloc(sizeof(struct block_table) * m->bt_size);

	for(i = 0; i < m->bt_size; i++)
	{
		m->bt[i].data = &data[block_table[i*4+0]];
		m->bt[i].csize  = block_table[i*4+1];
		m->bt[i].usize  = block_table[i*4+2];
		m->bt[i].flags  = block_table[i*4+3];
	}

	free(block_table);
}

static void read_hash_table(struct mpq *m, const char *data)
{
	uint32_t *hash_table;
	off_t offset;
	int i;

	offset     = U32(data[0x10]);
	m->ht_size = U32(data[0x18]);

	hash_table = malloc(sizeof(struct hash_table) * m->ht_size);
	memcpy(hash_table, &data[offset], m->ht_size * sizeof(struct hash_table));

	/* The decryption function works only on 32bit values, so calculate how many
	 * iterations it needs via dividing the size of a hash table entry by 4
	 * and then mutliplying the amount of hash table entries we have.
	 */
	decrypt_table(hash_table, m->ht_size * (sizeof(struct hash_table) / sizeof(uint32_t)),
		"(HASH TABLE)");

	/* Convert our blob of memory into an array of hash_table structs */
	m->ht = malloc(sizeof(struct hash_table) * m->ht_size);

	for(i = 0; i < m->ht_size; i++)
	{
		m->ht[i].hash   = ((uint64_t)hash_table[i*4+0]<<32) | hash_table[i*4+1];
		m->ht[i].locale = hash_table[i*4+2];
		m->ht[i].index  = hash_table[i*4+3];
	}

	free(hash_table);

}

static struct mpq *parse_MPQ(const char *file_data, int header_offset)
{
	struct mpq *m;

	m = malloc(sizeof(struct mpq));

	read_block_table(m, &file_data[header_offset]);
	read_hash_table(m, &file_data[header_offset]);

	return m;
}

/* Start parsing for MPQ headers at file_data[offset] */
struct mpq *parse_replay(const char *file_data, int offset)
{
	struct mpq *m = NULL;

	/* MPQ user data header */
	if(memcmp(&file_data[offset], "MPQ\x1B", 4) == 0)
	{
		int header_offset;

		header_offset = U32(file_data[offset + 0x8]);

		/* Skip to the first piece of user data and see if an MPQ1A can be found to parse */
		m = parse_replay(file_data, header_offset);
	}
	else if(memcmp(&file_data[offset], "MPQ\x1A", 4) == 0)
	{
		/* MPQ header */
		m = parse_MPQ(file_data, offset);
	}
	else
	{
		return NULL;
	}

	return m;
}

