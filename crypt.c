#include <stdint.h>
#include "mpq.h"

static uint32_t crypt_table[0x500];

void init_crypt()
{
	uint32_t seed = 0x100001;
	int i, j, c;

	for(j = 0; j < 0x100; j++)
	{
		for(i = j, c = 0; c < 5; c++, i += 0x100)
		{
			int t1, t2;

			seed = (seed * 125 + 3) % 0x2AAAAB;
			t1 = (seed & 0xFFFF) << 0x10;

			seed = (seed * 125 + 3) % 0x2AAAAB;
			t2 = (seed & 0xFFFF);

			crypt_table[i] = (t1 | t2);
		}  
	}
}

uint32_t hash_string(const char *key, int offset)
{
	uint32_t seed1 = 0x7FED7FED;
	uint32_t seed2 = 0xEEEEEEEE;

	while(*key)
	{
		seed1 = crypt_table[offset + *key] ^ (seed1 + seed2);
		seed2 = *key + seed1 + seed2 + (seed2 << 5) + 3;		
		key++;
	}

	return seed1;
}

void decrypt_table(uint32_t *table, int size, const char *key)
{
	uint32_t seed1;
	uint32_t seed2 = 0xEEEEEEEE;
	uint32_t ch;
	int i = 0;

	seed1 = hash_string(key, 0x300);

	while(i < size)
	{
		seed2 += crypt_table[0x400 + (seed1 & 0xFF)];
		ch     = table[i] ^ (seed1 + seed2);
		seed1  = ((~seed1 << 0x15) + 0x11111111) | (seed1 >> 0xB);
		seed2  = ch + seed2 + (seed2 << 5) + 3;
		table[i] = ch;
		i++;
	}
}

