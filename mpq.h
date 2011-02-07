#include <stdint.h>

struct block_table {
	const char *data;	/* Block data */
	uint32_t csize;		/* compressed size of block */
	uint32_t usize;		/* uncompressed size of block */
	uint32_t flags;
};

struct hash_table {
	uint64_t hash;
	uint32_t locale;	/* Language and platform information */
	uint32_t index;		/* Index into the block table */
};

struct mpq {
	const char *data;	/* mmap() of the .mpq */
	struct block_table *bt;
	int bt_size;
	struct hash_table *ht;
	int ht_size;
	int fd;				/* file descriptor of mapped archive */
};

void init_crypt(void);
void decrypt_table(uint32_t *, int, const char *);
uint32_t hash_string(const char *, int);

struct mpq *parse_replay(const char *, int);
struct mpq *mpq_open(const char *);

void decompress_bz2(const char *, char *, int, int);

void mpq_init();
char *mpq_read_file(struct mpq *, int, char *);
int mpq_find_file(struct mpq *, const char *, int *);
