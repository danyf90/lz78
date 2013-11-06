/**
 * @file	common.c
 * @author	Fabio Carrara, Daniele Formichelli
 * @date	May 14, 2013
 * @brief	Implementation file for common module.
 * @internal
 */

#include <openssl/evp.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

unsigned char* compute_digest(FILE *fin, const char *md_name, int *size) {
	unsigned char	buf[1048576], *md;
	int				r;
	EVP_MD_CTX		*md_ctx;

	//message digest initialization
	OpenSSL_add_all_digests();
	md_ctx = malloc(sizeof(EVP_MD_CTX));
	EVP_MD_CTX_init(md_ctx);
	EVP_DigestInit(md_ctx, EVP_get_digestbyname(md_name));
	*size = EVP_MD_CTX_size(md_ctx);
	md = malloc(*size);

	//compute md5
	while ((r = fread(buf, 1, 1048576, fin)) > 0)
		EVP_DigestUpdate(md_ctx, buf, r);
	EVP_DigestFinal_ex(md_ctx, md, NULL);

	fseek(fin, 0, SEEK_SET);

 	return md;
}

char* sprinth(const unsigned char *buff, int size) {

	int		i;
	char	*str;

	str = malloc(size*2 + 1);
	if (str == NULL)
		return NULL;

	for (i = 0; i < size; i++)
		sprintf(&str[2*i], "%02x", buff[i]);
	str[2 * size] = '\0';

	return str;
}

int path_len(const char* filename) {
	int i;
	
	if (filename == NULL)
		return -1;
	
	for (i = strlen(filename) - 1; i >= 0; i--) {
		if (filename[i] == '/') {
			return i+1;
			break;
		}
	}
	
	return 0;
}