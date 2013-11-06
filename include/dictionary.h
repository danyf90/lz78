/**
 * @file	dictionary.h
 * @author	Fabio Carrara, Daniele Formichelli
 * @date	May 13, 2013
 * @brief	Header file for dictionary interface.
 */

#ifndef __DICTIONARY_H__
#define __DICTIONARY_H__

#include "common.h"
#include <stdint.h>

#define DICT_MIN_SIZE	(NUM_SYMBOLS + 1)	/**< Minimum size of the dictionary that can be specified, in number of records. */
#define DICT_MAX_SIZE	(((uint64_t)1 << 32) - 2)	/**< Maximum size of the dictionary that can be specified, in number of records. */ //2 indexes are reserver for root_node and empty node

#define ROOT_NODE		DICT_MAX_SIZE + 1	/**< Index of root node of the dictionary tree. */
#define EMPTY_NODE		DICT_MAX_SIZE		/**< Symbol code to indicate an empty record. */

#define EOF_SYMBOL		NUM_SYMBOLS			/**< Symbol code for EndOfFile. */

/**
 * Dictionary context structure.
 */
struct dictionary;

// utilities for debugging purposes
#ifdef DEBUG
void dict_print_tree(const struct dictionary* d);
void dict_print_words(struct dictionary* d);
int	 dict_isempty(const struct dictionary* d, uint32_t ht_index);
#endif

/**
 * Allocate and initialize a new dictionary.
 *	@param	size		Size of the new dictionary, in number of records.
 *	@param	compression Indicates if the dictionary will be used for compression.
 *	@param	ht_size		Size of the hash table, in number of records.
 *	@param	symbols		Number of symbols in the alphabet.
 *
 *	@return	Pointer to the newly allocated dictionary on success, @c NULL on failure.
 */
struct dictionary* dict_new(uint32_t size, int compression, uint32_t ht_size, uint32_t symbols);

/**
 * Deallocate a dictionary.
 *
 *	@param	d	The dictionary to be deleted.
 */
void dict_delete(struct dictionary* d);

/**
 * Initialize the dictionary @p d.
 *	@param	d	Pointer to the dictionary to be initialized.
 *
 *	@return	The index of the next free record on success, @c 0 on failure.
 */
uint16_t dict_init(struct dictionary* d);

/**
 * Reinitialize (cleanup) the dictionary @p d..
 *	@param	d	Pointer to the dictionary to be initialized.
 *
 *	@return	The index of the next free record on success, @c 0 on failure.
 */
uint16_t dict_reinit(struct dictionary* d);

/**
 * Searches for next node in the dictionary tree.
 * If a node corresponding to symbol @p symbol starting from node @p current is
 * found, its index is put in @p ht_index and @c 1 is returned.
 * If the node is not found, in @p ht_index is put the index of the free record
 * where it should be and @c 0 is returned.
 * On error, @c -1 is returned.
 *
 *	@param	d			Pointer to the dictionary.
 *	@param	current		Node from which search starts.
 *	@param	symbol		The symbol to search for.
 *	@param	ht_index	Pointer to area where to store the index of the found node.
 *
 *	@return	@c 1 on success, @c 0 when the node in not found, @c -1 on failure.
 */
int dict_lookup(const struct dictionary* d, uint32_t current, uint16_t symbol, uint32_t* ht_index);

/**
 * Fill the record at index @p index in the dictionary @p d.
 *	@param	d			Pointer to the dictionary.
 *	@param	ht_index	Index of the record to be filled up.
 *	@param	current		Current node index to be placed in the record. If this is
						set to @c ROOT_NODE, the corresponding value is left untouched.
 *	@param	symbol		Symbol to be placed in the record.
  *	@param	next		Next node index to be placed in the record.
 *
 *	@return	@¢ 1 on success, @c 0 on failure.
 */
int dict_fill(struct dictionary* d, uint32_t ht_index, uint32_t current, uint8_t symbol, uint32_t next);

/**
 * Returns the next node index contained in the record at index @p i.
 * If an error occurs, @c ROOT_NODE as an invalid value of next is returned.
 * 
 *	@example
 *	uint16_t nxt = get_next(d,cur);
 *
 *	if (nxt == ROOT_NODE) { // error
 *		perror("get_next");
 *	}
 *
 *	@param	d			Pointer to the dictionary.
 *	@param	ht_index	Index of the record.
 *
 *	@return	The next index on success, @c ROOT_NODE on error.
 */
uint32_t dict_next(const struct dictionary* d, uint32_t ht_index);

/**
 * Returns a pointer to the word contained in the dictionary @p d correspondent
 * to the node @p node_index in the tree.
 * On return @p len contains the length of the word found. A call to dict_word overwrites
 * word returned on previous calls.
 *
 *	@param	d			Pointer to the dictionary.
 *	@param	node_index	Index of the node.
 *	@param	len			Pointer to an integer where to store the word size.
 *
 *	@return	The pointer to the word and its size in @p len on success, @c NULL on error.
 */
char* dict_word(struct dictionary* d, uint32_t node_index, uint32_t* len);

/**
 * Returns the first symbol of the word contained in dictionary @p d at node
 * index @p node_index.
 *
 *	@param	d			Pointer to the dictionary.
 *	@param	node_index	Index of the record.
 *
 *	@return	The first symbol of the correspondent word on success, @c EOF_SYMBOL on failure.
 */
uint16_t dict_first_symbol(const struct dictionary* d, uint32_t node_index);

#endif
