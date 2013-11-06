/**
 * @file	debug.h
 * @author	Fabio Carrara, Daniele Formichelli
 * @date	May 14, 2013
 * @brief	Header file that provides debug macros.
 */

#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef DEBUG

#include<stdio.h>

#define LOG(format, ...) fprintf(stderr, "%s:%d:%s -> " format "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#else

#define LOG(...)
#define PERROR(...)

#endif

#endif
