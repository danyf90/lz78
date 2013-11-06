/**
 * @file	compressor.c
 * @author	Fabio Carrara, Daniele Formichelli
 * @date	May 14, 2013
 * @brief	Implementation file for compressor module.
 * @internal
 */

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>

#include "bitio.h"
#include "common.h"
#include "compressor.h"
#include "debug.h"
#include "dictionary.h"
#include "metadata.h"
#include "verbose.h"

/**
 * @internal
 * Writes @p index on @p f using @p bits bits.
 *
 *	@param	f		Pointer to bitio context in which write the bits.
 *	@param	index	Index in the dictionary to be emitted.
 *	@param	bits	Number of bits used to represent the index.
 *
 *	@return	@c 0 on success, @c -1 otherwise.
 */
int emit(struct bitio* f, uint32_t index, uint8_t bits) {
	LOG("Emitted index: %d on %d bits", index, bits);

	if (bitio_write(f, (uint64_t) index, bits) != bits)
		return -1;
	return 0;
}

int64_t compress(const char* in_filename, const char* out_filename, uint32_t dict_size, uint32_t ht_size, uint8_t flags) {

	struct bitio		*bd = bstdout;
	struct dictionary	*d = NULL;
	struct stat			file_stat;
	time_t				t;
	FILE				*fin = stdin;
	char				*md5_str;
	int					c, read_count = 0;
	uint8_t				bits, initial_bits;
	uint32_t			bitMask, cur, next_record, y;
	uint64_t			filesize = 0;
	unsigned char		*md5;


	if (out_filename != NULL && in_filename != NULL && strcmp(in_filename, out_filename) == 0) {
		errno = EINVAL;
		goto error;
	}

	if (in_filename != NULL) {
		fin = fopen(in_filename, "r");
		if (fin == NULL)
			goto error;
	}

	if (out_filename != NULL) {
		bd = bitio_open(out_filename, 'w');
		if (bd == NULL)
			goto error;
	}

	//write metadata
	if (flags & META_DICT_SIZE)
		if (meta_write(bd, META_DICT_SIZE, &dict_size, sizeof(dict_size)) < 0)
			goto error;

	if (flags & META_MD5) {
		if (fin != stdin) {
			int md5_size;

			md5 = compute_digest(fin, "md5", &md5_size);
			if (meta_write(bd, META_MD5, md5, md5_size) < 0)
				goto error;
			md5_str = sprinth(md5, md5_size);
			PRINT(1, "md5sum:\t\t\t%s\n", md5_str);
			free(md5);
			free(md5_str);
		}
		else
			PRINT(1, "md5sum:\t\t\tNot availabe when reading from stdin\n");
	}

	if ((flags & META_NAME) && in_filename != NULL) { //don't put META_NAME if input = stdin
		c = path_len(in_filename);
		if (meta_write(bd, META_NAME, (void*)&in_filename[c], strlen(in_filename) - c + 1) < 0)
			goto error;
	}

	if ((flags & META_TIMESTAMP) && in_filename != NULL) { //don't put META_TIMESTAMP if input = stdin
		fstat(fileno(fin), &file_stat);
		t = file_stat.st_mtime;
		if (meta_write(bd, META_TIMESTAMP, &t, sizeof(t)) < 0)
			goto error;
	}

	if (meta_finalize(bd) < 0)
		goto error;

	d = dict_new(dict_size, 1, ht_size, NUM_SYMBOLS);

	if (d == NULL)
		goto error;

	next_record = dict_init(d);
	initial_bits = 0;
	bitMask = 1;
	while (bitMask < next_record) {
		bitMask <<= 1;
		initial_bits++;
	}
	bits = initial_bits;
	bitMask = 1 << bits;
	
	cur = ROOT_NODE;
	for(;;) {
  		c = fgetc(fin);
		if (c == EOF) {

			//emit last word
			if (emit(bd, cur, bits) < 0)
				goto error;

			//emit EOF
			dict_lookup(d, ROOT_NODE, EOF_SYMBOL, &y);

			if (emit(bd, y, bits) < 0)
				goto error;

			break;
		}
		
		filesize++;

		if (VERBOSE_LEVEL > 0 && ++read_count >= COUNT_THRESHOLD) {
			read_count = 0;
			PRINT(1, ".");
		}

		if (!dict_lookup(d, cur, (uint16_t) c, &y)) { //node not found

			if (emit(bd, cur, bits) < 0)
				goto error;

			dict_fill(d, y, cur, (uint16_t) c, next_record++);
			if (next_record & bitMask) {
				bitMask <<= 1;
				bits++;
			}

			if (next_record == dict_size) {
				next_record = dict_reinit(d);
				bits = initial_bits;
				bitMask = 1 << bits;
			}

			// search again starting from last unmatched symbol
			dict_lookup(d, ROOT_NODE, (uint16_t) c, &y);
		}

		cur = dict_next(d, y);
	}

	PRINT(1, "\nCompression Finished\n\n");
	dict_delete(d);
	bitio_flush(bd);
	if (bd != bstdout)
		bitio_close(bd);
	if (fin != NULL)
		fclose(fin);
	return filesize;

error:
	PRINT(1, "\n");
	dict_delete(d);
	bitio_flush(bd);
	if (bd != bstdout)
		bitio_close(bd);
	if (fin != NULL)
		fclose(fin);
	return -1;
}
