/**
 * @file	dictionary.c
 * @author	Fabio Carrara, Daniele Formichelli
 * @date	May 14, 2013
 * @brief	Implementation file for dictionary module.
 * @internal
 */

#include <errno.h>
#include <stdlib.h>
#include <stdint.h>

#include "debug.h"

#include "dictionary.h"
#include "verbose.h"

#define WORD_START_SIZE	10

/**
 * Type that represent the tree/hash table.
 * @internal
 */
struct ht_t {
	uint32_t	*current;	/**< Index of starting node of the branch. */
	uint8_t		*symbol;	/**< Symbol correspondent to the branch. */
	uint32_t	*next;		/**< Index of ending node of the branch. */
};

/**
 * Structure of the dictionary context.
 * @internal
 */
struct dictionary {
	uint32_t		size;			/**< Maximum number of nodes (words) in the dictionary tree. */
	uint16_t		symbols;		/**< Size of the alphabet. */
	struct ht_t		ht;				/**< Structure that contains the hash table. */
	uint32_t		ht_size;		/**< Size of the hash table, in number of records. */
	char			*word;			/**< Pointer to auxiliary memory used by dict_word() function. */
	int				max_size;		/**< Current size of the memory pointend by word. */
	uint8_t			compression; 	/**< Indicates if the dictionary is used for compression or decompression. */
};

/**
 * Hash function: returns a value between @p min and (@p max - 1).
 * It uses Division Hash Function: H(x) = x mod m. 
 * (@p current || @p symbol) is used as key.
 *
 *	@param	current	first part of the key
 *	@param	symbol	second part of the key
 *	@param	min		minimum return value
 *	@param	max		max return value plus one
 *
 *	@return	Hash of the key between min ad (max - 1)
 *	@internal Knuth division hash algorithm, using as key
 *	((@p current << 8) | @p symbol ).
 */
inline uint32_t dict_hash(uint32_t current, uint32_t symbol, uint32_t min, uint32_t max) {
	return min + ((current << 8 | symbol) % (max - min));
}

struct dictionary* dict_new(uint32_t size, int compression, uint32_t ht_size, uint32_t symbols) {

	struct dictionary* d = NULL;

	if (size > DICT_MAX_SIZE || size > ht_size || size < symbols) {
		errno = EINVAL;
		return NULL;
	}

	d = malloc(sizeof(struct dictionary));
	if(d == NULL)
		return NULL;
	d->ht.current = NULL;
	d->ht.symbol = NULL;
	d->ht.next = NULL;
	d->word = NULL;
	d->compression = compression;
	
	
	d->size = size;
	d->symbols = symbols;
	
	d->ht.current = malloc(sizeof(d->ht.current)*ht_size);
	if (d->ht.current == NULL)
		goto error;
	d->ht.symbol = malloc(sizeof(d->ht.symbol)*ht_size);
	if (d->ht.symbol == NULL)
		goto error;
	if (compression == 1) {
		d->ht.next = malloc(sizeof(d->ht.next)*ht_size);
		if (d->ht.next == NULL)
			goto error;
	}
	d->ht_size = ht_size;
	
	d->word = malloc(WORD_START_SIZE + 1);
	if (d->word == NULL)
		goto error;
	d->max_size = WORD_START_SIZE;
	
	return d;

error:
	free(d->ht.current);
	free(d->ht.symbol);
	free(d->ht.next);
	free(d->word);
	free(d);
	return NULL;
}

void dict_delete(struct dictionary* d) {

	if (d != NULL) {
		free(d->ht.current);
		free(d->ht.symbol);
		free(d->ht.next);
		free(d->word);
		free(d);
	}
}

uint16_t dict_init(struct dictionary* d) {

	uint32_t i;

	if (d == NULL) {
		errno = EINVAL;
		return 0;
	}

	// set initial symbols from root
	for (i = 0; i <= d->symbols; i++) {
		d->ht.current[i] = ROOT_NODE;
		d->ht.symbol[i] = i;
		if (d->compression) // decompressor doesn't need next field
			d->ht.next[i] = i;
	}

	if (d->compression) // decompressor doesn't need to clean empty records
		dict_reinit(d);

	return d->symbols+1;
}

uint16_t dict_reinit(struct dictionary* d) {
	
	uint32_t i;
	
	if (d == NULL) {
		errno = EINVAL;
		return 0;
	}
	
	for (i = d->symbols+1; i < d->ht_size; i++)
		d->ht.current[i] = EMPTY_NODE;
	
	return d->symbols+1;
}

int dict_lookup(const struct dictionary* d, uint32_t current, uint16_t symbol, uint32_t *ht_index) {

	uint32_t i;

	if (d == NULL || ((symbol > d->symbols-1 && symbol != EOF_SYMBOL)) || (current > d->size-1 && current != ROOT_NODE) ) {
		errno = EINVAL;
		return -1;
	}

	if (current == ROOT_NODE) {
		*ht_index = symbol;
		return 1;
	}

	*ht_index = dict_hash(current, symbol, d->symbols+1, d->ht_size);

	for (i = 0; ; i++) {
		if (d->ht.current[*ht_index] == current && d->ht.symbol[*ht_index] == symbol) // symbol found
			return 1;
		else if (d->ht.current[*ht_index] == EMPTY_NODE) // empty record found
			return 0;

		(*ht_index)++;
		if (*ht_index == d->ht_size)
			*ht_index = d->symbols + 1;
	}
}

int dict_fill(struct dictionary* d, uint32_t ht_index, uint32_t current, uint8_t symbol, uint32_t next) {

	if (d == NULL || ht_index > d->ht_size || symbol > d->symbols || (current > d->size-1 && current != ROOT_NODE)) {
		errno = EINVAL;
		return 0;
	}

	if (current != ROOT_NODE) // ROOT_NODE as current means don't change it
		d->ht.current[ht_index] = current;

	d->ht.symbol[ht_index] = symbol;
	if (d->compression)
		d->ht.next[ht_index] = next;

	return 1;
}

uint32_t dict_next(const struct dictionary* d, uint32_t ht_index) {

	if (d == NULL || ht_index > d->ht_size || d->compression == 0) {
		errno = EINVAL;
		return ROOT_NODE;
	}

	return d->ht.next[ht_index];
}

char* dict_word(struct dictionary* d, uint32_t node_index, uint32_t* len) {

	uint32_t		cur, i = 0, l = 0;
	char			swap;

	if (d == NULL || node_index > d->size-1 || len == NULL) {
		errno = EINVAL;
		return NULL;
	}

	while(node_index != ROOT_NODE) {
		cur = node_index;

		if (l == d->max_size) { // reallocate a bigger buffer
			char *reallocated;
			d->max_size *= 2;
			reallocated = realloc(d->word, d->max_size+1);
			if (reallocated == NULL) {
				free(d->word);
				d->word = NULL;
				d->max_size = 0;
				errno = ENOMEM;
				return NULL;
			}
			d->word = reallocated;
		}

		d->word[l] = (char) d->ht.symbol[cur];
		l++;
		node_index = d->ht.current[cur];
	}

	// reverse the string and add '\0'
	for (i = 0; i < l/2; i++) {
		swap = d->word[i];
		d->word[i] = d->word[l-i-1];
		d->word[l-i-1] = swap;
	}
	d->word[l] = '\0';

	*len = l;

	return d->word;
}

uint16_t dict_first_symbol(const struct dictionary* d, uint32_t node_index) {

	uint32_t cur = 0;

	if (d == NULL || node_index > d->size-1) {
		errno = EINVAL;
		return EOF_SYMBOL; //invalid first symbol
	}

	while (node_index != ROOT_NODE) {
		cur = node_index;
		node_index = d->ht.current[node_index];
	}

	return d->ht.symbol[cur];
}
