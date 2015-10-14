/*
***************************************************************************
*
* Author: Teunis van Beelen
*
* Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015 Teunis van Beelen
*
* Email: teuniz@gmail.com
*
***************************************************************************
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
***************************************************************************
*/


#ifndef EDFPLUS_ANNOT_LIST_H
#define EDFPLUS_ANNOT_LIST_H


#include <stdlib.h>
#include <string.h>

#include "global.h"


#ifdef __cplusplus
extern "C" {
#endif


void edfplus_annotation_add_item(struct annotationblock **, struct annotationblock *);
void edfplus_annotation_add_copy(struct annotationblock **, struct annotationblock *);
int edfplus_annotation_count(struct annotationblock **);
void edfplus_annotation_delete(struct annotationblock **, int);
void edfplus_annotation_delete_list(struct annotationblock **);
struct annotationblock * edfplus_annotation_item(struct annotationblock **, int);
void edfplus_annotation_sort(struct annotationblock **);
struct annotationblock * edfplus_annotation_copy_list(struct annotationblock **);

int get_tal_timestamp_digit_cnt(struct edfhdrblock *);
int get_tal_timestamp_decimal_cnt(struct edfhdrblock *);
int get_max_annotation_strlen(struct annotationblock **);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif




