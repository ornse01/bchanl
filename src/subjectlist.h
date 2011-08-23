/*
 * subjectlist.h
 *
 * Copyright (c) 2011 project bchan
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 *
 */

#include    <basic.h>
#include    <btron/clk.h>
#include    "subjectparser.h"

#ifndef __SUBJECTLIST_H__
#define __SUBJECTLIST_H__

typedef struct sbjtlist_tuple_t_ sbjtlist_tuple_t;
typedef struct sbjtlist_t_ sbjtlist_t;
typedef struct sbjtlist_iterator_t_ sbjtlist_iterator_t;

IMPORT sbjtlist_t* sbjtlist_new();
IMPORT VOID sbjtlist_delete(sbjtlist_t *list);
IMPORT W sbjtlist_appendthread(sbjtlist_t *list, sbjtparser_thread_t *parser_thread);
#define SBJTLIST_SORTBY_NUMBER 1
#define SBJTLIST_SORTBY_RES 2
#define SBJTLIST_SORTBY_SINCE 3
#define SBJTLIST_SORTBY_VIGOR 4
IMPORT W sbjtlist_sort(sbjtlist_t *list, W by, TC *filterword, W filterword_len);
IMPORT VOID sbjtlist_clear(sbjtlist_t *list);
IMPORT sbjtlist_iterator_t* sbjtlist_startread(sbjtlist_t *list, Bool descending);
IMPORT VOID sbjtlist_endread(sbjtlist_t *list, sbjtlist_iterator_t *iter);

IMPORT Bool sbjtlist_iterator_next(sbjtlist_iterator_t *iter, sbjtlist_tuple_t **item);

IMPORT VOID sbjtlist_tuple_gettitle(sbjtlist_tuple_t *tuple, TC **str, W *len);
IMPORT VOID sbjtlist_tuple_getnumber(sbjtlist_tuple_t *tuple, W *num);
IMPORT VOID sbjtlist_tuple_getresnumber(sbjtlist_tuple_t *tuple, W *num);
IMPORT VOID sbjtlist_tuple_getsince(sbjtlist_tuple_t *tuple, DATE_TIM *since);
IMPORT VOID sbjtlist_tuple_getvigor(sbjtlist_tuple_t *tuple, W *vigor);
/* vigor = res / minutes * 60 * 24 * (for modify) 10 */

#endif
