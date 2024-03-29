#ifndef NM_ERROR_H
#define NM_ERROR_H

#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "ft_nm_types.h"

#define ERIOPT		1
#define ERNOFILE	2
#define ERWFFMT		3
#define EREMPTY		4	
#define ERRCORRUPT	5
#define ERMEM		6
#define ERISDIR		7
#define ERRNO		8

#define	ERIOPT_MSG		"invalid option --"
#define ERISDIR_MSG		"is a directory"
#define	ERNOFILE_MSG	"no such file"
#define	ERWFFMT_MSG		"file format not recognized"
#define EREMPTY_MSG		"empty file"
#define ERRCORRUPT_MSG	"corrupted data"
#define ERMEM_MSG		"memory allocation failed"

uint8_t *error_location(void);

#define error (*error_location())

#define SET_ERROR(er) (error = er)

void print_error(uint8_t err, char *i);

#endif // NM_ERROR_H
