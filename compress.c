#include <bzlib.h>

/* Decompress in to out, from csize to usize */
void decompress_bz2(const char *in, char *out, int csize, int usize)
{
	bz_stream bz;

	bz.bzalloc = NULL;
	bz.bzfree  = NULL;

	BZ2_bzDecompressInit(&bz, 0, 0);
			
	bz.next_in = (void *)in;
	bz.avail_in = csize;
	bz.next_out = out;
	bz.avail_out = usize;

	while(BZ2_bzDecompress(&bz) != BZ_STREAM_END)
		;

	BZ2_bzDecompressEnd(&bz);
}
