/**
 * @file	decompressor.c
 * @author	Fabio Carrara, Daniele Formichelli
 * @date	May 14, 2013
 * @brief	Implementation file for decompressor module.
 * @internal
 */

#include <errno.h>
#include <openssl/evp.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>

#include "bitio.h"
#include "common.h"
#include "debug.h"
#include "decompressor.h"
#include "dictionary.h"
#include "metadata.h"
#include "verbose.h"

/**
 * @internal
 * Reads @p bits bits from @p f and returns the fetched number.
 *
 *	@param	f		Pointer to bitio context from which read the bits.
 *	@param	bits	Number of bits to read.
 *
 *	@return	The fetched index on success, ROOT_NODE on failure.
 */
uint32_t fetch(struct bitio* f, uint8_t bits) {

	uint64_t	index;

	if (bitio_read(f, &index, bits) < bits)
		return ROOT_NODE;

	return index;
}

int64_t decompress(const char* in_filename, const char* out_filename, uint8_t flags) {

	struct bitio		*bd = bstdin;
	struct dictionary	*d = NULL;
	struct utimbuf		*t = NULL;
	FILE*				fout = stdout;
	char				*out_file = NULL;
	uint8_t				bits, initial_bits, meta_type, meta_size;
	uint16_t			c;
	uint32_t			bitMask, cur, first_record, len, next_record, dict_size = 0, written = 0, write_count = 0;
	uint64_t			filesize = 0;
	char				*word;
	int					first = 1, md5c_size = 0, md5d_size = 0;
	void				*meta_data, *md5c = NULL, *md5d = NULL;
	EVP_MD_CTX			*md_ctx = NULL;

	if (in_filename != NULL) { 
		bd = bitio_open(in_filename, 'r');
		if (bd == NULL)
			goto error;
	}

	//read metadata
	while ((meta_data = meta_read(bd, &meta_type, &meta_size)) != META_END) {
		LOG("META_TYPE: %d", meta_type);
		switch (meta_type) {
			case META_DICT_SIZE:
				dict_size = *(uint32_t*)meta_data;
				PRINT(1, "Dictionary Size:\t%d\n", dict_size);
				break;

			case META_NAME:
				PRINT(1, "Original file name:\t%s\n", (char*)meta_data);
				if (flags & DEC_ORIG_FILENAME) {
					out_file = malloc(meta_size);
					if (out_file == NULL)
						goto error;
					memcpy((void*)out_file, meta_data, meta_size);
					out_filename = out_file;
				}
				break;

			case META_MD5:
				md5c = malloc(meta_size);
				memcpy(md5c, meta_data, meta_size);
				md5c_size = meta_size;
				word = sprinth(md5c, md5c_size);
				PRINT(1, "Original md5sum:\t%s\n", word);
				free(word);
				// initialize md context
				OpenSSL_add_all_digests();
				md_ctx = malloc(sizeof(EVP_MD_CTX));
				EVP_MD_CTX_init(md_ctx);
				EVP_DigestInit(md_ctx, EVP_get_digestbyname("md5"));
				md5d_size = EVP_MD_CTX_size(md_ctx);
				md5d = malloc(md5d_size);
				break;

			case META_TIMESTAMP:
				t = malloc(sizeof(*t));
				t->actime = *((time_t*)meta_data); // access time
				t->modtime = *((time_t*)meta_data); // modification time
				break;

			default: // META_ERROR
				LOG("Unknown metadata");
				errno = EINVAL;
				goto error;
		}
		free(meta_data);
	}

	if ((flags & DEC_ORIG_FILENAME) && out_file == NULL) // if i have DEC_ORIG_FILENAME setted but no info in metadata i use stdin as outfile
		out_filename = "stdin";
	
	if (out_filename != NULL) {
		fout = fopen(out_filename, "w");
		if (fout == NULL)
			goto error;
	}

	if (out_filename != NULL && in_filename != NULL && strcmp(in_filename, out_filename) == 0) {
		errno = EINVAL;
		goto error;
	}

	if (dict_size == 0)
		goto error;

	d = dict_new(dict_size, 0, dict_size, NUM_SYMBOLS);

	if (d == NULL)
		goto error;

	first_record = dict_init(d);
	next_record = first_record;
	initial_bits = 0;
	bitMask = 1;
	while (bitMask < next_record) {
		bitMask <<= 1;
		initial_bits++;
	}
	bits = initial_bits;
	
	for (;;) {
		// put in cur the index of the fetched word in the dictionary
		cur = fetch(bd, bits);
		if (cur == ROOT_NODE)
			goto error;

		if (cur == EOF_SYMBOL)
			break;

		c = dict_first_symbol(d, cur);
		
		if (c == EOF_SYMBOL)
			goto error;
		
		if (!first) {
			// complete previous record with index of new record
			// ROOT_NODE as current node value means 'don't change it'.
			dict_fill(d, next_record, ROOT_NODE, (uint8_t) c, 0);
			next_record++;
			if ((next_record+1) & bitMask) {
				bitMask <<= 1;
				bits++;
			}
		}
		else
			first = 0;

		// get the word in the dictionary at index cur.
		word = dict_word(d, cur, &len);
		if (word == NULL)
			goto error;
		
		written = fwrite(word, 1, len, fout);

		if (written < len)
			goto error;
		else { // md5 computation and visual feedback

			if (md5c != NULL) // compute md5 of decompressed
				EVP_DigestUpdate(md_ctx, word, len);

			write_count += written;
			if (write_count >= COUNT_THRESHOLD) {
				filesize += write_count;
				write_count = 0;
				PRINT(1, ".");
			}
		}

		if (next_record + 1 == dict_size) {
			
			next_record = first_record;
			
			bits = initial_bits;
			bitMask = 1 << bits;

			first = 1; // set first iteration to be the next
		}

		// add a new record
		dict_fill(d, next_record, cur, 0, 0); // symbol will be filled at the beginning of next iteration

	}
	
	filesize += write_count;

	if (md5c != NULL) {
		EVP_DigestFinal_ex(md_ctx, md5d, (unsigned int*)&md5d_size);

		if (md5c_size == md5d_size && memcmp(md5c, md5d, md5c_size) == 0)
			PRINT(1, "\nmd5sum Check:\t\tOK");
		else {
			PRINT(1, "\nmd5sum Check:\t\tFailed");
			goto error;
		}
	}

	PRINT(1, "\nDecompression Finished\n\n");

	fclose(fout);
	if (out_file != NULL && t != NULL)
		if (utime(out_filename, t) < 0) { // set modification time
			PRINT(1, "Error while changing last modification time");
		}
	free(out_file);
	free(t);
	dict_delete(d);
	bitio_flush(bd);
	if (bd != bstdin)
		bitio_close(bd);
	return filesize;

error:
	PRINT(1, "\n");
	if (out_filename != NULL)
		unlink(out_filename);
	free(out_file);
	free(t);
	dict_delete(d);
	bitio_flush(bd);
	if (bd != bstdin)
		bitio_close(bd);
	if (fout != NULL)
		fclose(fout);
	return -1;
}
