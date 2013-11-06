/**
 * @file	verbose.h
 * @author	Fabio Carrara, Daniele Formichelli
 * @date	May 14, 2013
 * @brief	Header file for printing depending on the verbose level.
 */

#ifndef __VERBOSE_H__
#define __VERBOSE_H__

#include <stdio.h>

extern	int		VERBOSE_LEVEL;
extern	FILE	*VERBOSE_STREAM;

#ifdef DEBUG
	#define PRINT(level, format, ...) fprintf(VERBOSE_STREAM, format, ##__VA_ARGS__)
#else
	#define PRINT(level, format, ...) \
if (VERBOSE_LEVEL >= level) { \
			fprintf(VERBOSE_STREAM, format, ##__VA_ARGS__); \
			fflush(VERBOSE_STREAM); \
		} \
		else
#endif

#endif
