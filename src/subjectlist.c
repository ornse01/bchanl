/*
 * subjectlist.c
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

#include	<basic.h>
#include	<bstdio.h>
#include	<bstdlib.h>
#include	<bstring.h>
#include	<tstring.h>

#include    "subjectlist.h"
#include    "subjectparser.h"
#include    "array.h"

#ifdef BCHANL_CONFIG_DEBUG
# define DP(arg) printf arg
# define DP_ER(msg, err) printf("%s (%d/%x)\n", msg, err>>16, err)
#else
# define DP(arg) /**/
# define DP_ER(msg, err) /**/
#endif

struct sbjtlist_tuple_t_ {
	sbjtparser_thread_t *parser_thread;
	W index; /* by subject.txt */
	W vigor;
};

struct sbjtlist_originarray_t_ {
	arraybase_t base;
};
typedef struct sbjtlist_originarray_t_ sbjtlist_originarray_t;

struct sbjtlist_sortedarray_t_ {
	arraybase_t base;
};
typedef struct sbjtlist_sortedarray_t_ sbjtlist_sortedarray_t;

struct sbjtlist_t_ {
	sbjtlist_originarray_t origin;
	sbjtlist_sortedarray_t sorted;
	W sortby;
};

struct sbjtlist_iterator_t_ {
	Bool descending;
	W i;
	sbjtlist_sortedarray_t *array;
};

IMPORT VOID sbjtlist_tuple_getsinceSTIME(sbjtlist_tuple_t *tuple, STIME *since);
LOCAL W sbjtlist_tuple_calcvigor(sbjtlist_tuple_t *tuple, STIME current)
{
	STIME since;
	W resnum;
	sbjtlist_tuple_getsinceSTIME(tuple, &since);
	sbjtlist_tuple_getresnumber(tuple, &resnum);
	return resnum * 60 * 60 * 24 * 10 / (current - since); /* res per day */
}

LOCAL W sbjtlist_originarray_append(sbjtlist_originarray_t *array, sbjtparser_thread_t *parser_thread, STIME current)
{
	sbjtlist_tuple_t tuple;

	tuple.parser_thread = parser_thread;
	tuple.index = arraybase_length(&array->base);
	tuple.vigor = sbjtlist_tuple_calcvigor(&tuple, current); /* Ugh! should not call this in construction. */

	return arraybase_appendunit(&array->base, &tuple);
}

LOCAL Bool sbjtlist_originarray_getbyindex(sbjtlist_originarray_t *array, W i, sbjtlist_tuple_t **tuple)
{
	return arraybase_getunitbyindex(&array->base, i, (VP*)tuple);
}

LOCAL VOID sbjtlist_originarray_truncate(sbjtlist_originarray_t *array, W newlength)
{
	arraybase_truncate(&array->base, newlength);
}

LOCAL W sbjtlist_originarray_length(sbjtlist_originarray_t *array)
{
	return arraybase_length(&array->base);
}

LOCAL W sbjtlist_originarray_initialize(sbjtlist_originarray_t *array)
{
	return arraybase_initialize(&array->base, sizeof(sbjtlist_tuple_t), 512);
}

LOCAL VOID sbjtlist_originarray_finalize(sbjtlist_originarray_t *array)
{
	arraybase_finalize(&array->base);
}

LOCAL W sbjtlist_sortedarray_append(sbjtlist_sortedarray_t *array, sbjtlist_tuple_t *tuple)
{
	return arraybase_appendunit(&array->base, &tuple);
}

LOCAL Bool sbjtlist_sortedarray_getbyindex(sbjtlist_sortedarray_t *array, W index, sbjtlist_tuple_t **tuple)
{
	Bool ret;
	VP p;
	ret = arraybase_getunitbyindex(&array->base, index, &p);
	if (ret == False) {
		return ret;
	}
	*tuple = *((sbjtlist_tuple_t**)p);
	return True;
}

LOCAL VOID sbjtlist_sortedarray_truncate(sbjtlist_sortedarray_t *array, W newlength)
{
	arraybase_truncate(&array->base, newlength);
}

LOCAL W sbjtlist_sortedarray_length(sbjtlist_sortedarray_t *array)
{
	return arraybase_length(&array->base);
}

LOCAL W sbjtlist_sortedarray_initialize(sbjtlist_sortedarray_t *array)
{
	return arraybase_initialize(&array->base, sizeof(sbjtlist_tuple_t*), 512);
}

LOCAL VOID sbjtlist_sortedarray_finalize(sbjtlist_sortedarray_t *array)
{
	arraybase_finalize(&array->base);
}

/**/

EXPORT VOID sbjtlist_tuple_gettitle(sbjtlist_tuple_t *tuple, TC **str, W *len)
{
	sbjtparser_thread_gettitlestr(tuple->parser_thread, str, len);
}

EXPORT VOID sbjtlist_tuple_getthreadnumberstr(sbjtlist_tuple_t *tuple, UB **str, W *len)
{
	*str = tuple->parser_thread->number;
	*len = tuple->parser_thread->number_len;
}

EXPORT VOID sbjtlist_tuple_getnumber(sbjtlist_tuple_t *tuple, W *num)
{
	*num = tuple->index + 1;
}

EXPORT VOID sbjtlist_tuple_getresnumber(sbjtlist_tuple_t *tuple, W *num)
{
	TC *str;
	W len;
	sbjtparser_thread_getresnumstr(tuple->parser_thread, &str, &len);
	*num = tc_atoi(str + 1);
}

EXPORT VOID sbjtlist_tuple_getresnumberstr(sbjtlist_tuple_t *tuple, TC **str, W *len)
{
	sbjtparser_thread_getresnumstr(tuple->parser_thread, str, len);
}

EXPORT VOID sbjtlist_tuple_getsinceSTIME(sbjtlist_tuple_t *tuple, STIME *since)
{
	W since_u;
	since_u = atoi(tuple->parser_thread->number);
	*since = since_u - 473385600;
}

EXPORT VOID sbjtlist_tuple_getsince(sbjtlist_tuple_t *tuple, DATE_TIM *since)
{
	STIME tim;
	sbjtlist_tuple_getsinceSTIME(tuple, &tim);
	get_tod(since, tim, 0);
}

EXPORT VOID sbjtlist_tuple_getvigor(sbjtlist_tuple_t *tuple, W *vigor)
{
	*vigor = tuple->vigor;
}

LOCAL W sbjtlist_tuple_compare_resnumber(sbjtlist_tuple_t *t1, sbjtlist_tuple_t *t2)
{
	W n1, n2;
	sbjtlist_tuple_getnumber(t1, &n1);
	sbjtlist_tuple_getnumber(t2, &n2);
	if (n1 > n2) {
		return 1;
	}
	if (n1 < n2) {
		return -1;
	}
	return 0;
}

LOCAL W sbjtlist_tuple_compare_vigor(sbjtlist_tuple_t *t1, sbjtlist_tuple_t *t2)
{
	W n1, n2;
	sbjtlist_tuple_getvigor(t1, &n1);
	sbjtlist_tuple_getvigor(t2, &n2);
	if (n1 > n2) {
		return 1;
	}
	if (n1 < n2) {
		return -1;
	}
	return 0;
}

/**/

EXPORT W sbjtlist_appendthread(sbjtlist_t *list, sbjtparser_thread_t *parser_thread, STIME current)
{
	return sbjtlist_originarray_append(&list->origin, parser_thread, current);
}

LOCAL W sbjtlist_copyarraywithfilter(sbjtlist_t *list, TC *filterword, W filterword_len)
{
	W i, len, err;
	Bool found;
	sbjtlist_tuple_t *tuple;

	len = sbjtlist_originarray_length(&list->origin);
	for (i = 0; i < len; i++) {
		found = sbjtlist_originarray_getbyindex(&list->origin, i, &tuple);
		if (found == True) {
			/* TODO: check filter word */
			err = sbjtlist_sortedarray_append(&list->sorted, tuple);
			if (err < 0) {
				return err;
			}
		}
	}

	return 0;
}

LOCAL W sbjtlist_sort_compare(sbjtlist_tuple_t *t1, sbjtlist_tuple_t *t2, W by)
{
	if (by == SBJTLIST_SORTBY_SINCE) {
		return strcmp(t1->parser_thread->number, t2->parser_thread->number);
	}
	if (by == SBJTLIST_SORTBY_RES) {
		return sbjtlist_tuple_compare_resnumber(t1, t2);
	}
	if (by == SBJTLIST_SORTBY_VIGOR) {
		return sbjtlist_tuple_compare_vigor(t1, t2);
	}
	/* SBJTLIST_SORTBY_NUMBER */
	if (t1->index > t2->index) {
		return 1;
	}
	if (t1->index < t2->index) {
		return -1;
	}
	return 0;
}

LOCAL VOID sbjtlist_sort_swap(sbjtlist_tuple_t *t1, sbjtlist_tuple_t *t2)
{
	sbjtlist_tuple_t t;

	memcpy(&t, t1, sizeof(sbjtlist_tuple_t));
	memcpy(t1, t2, sizeof(sbjtlist_tuple_t));
	memcpy(t2, &t, sizeof(sbjtlist_tuple_t));
}

LOCAL VOID sbjtlist_sortedarray_combsort(sbjtlist_sortedarray_t *array, W by)
{
	W h, swaps, i, len, result;
	sbjtlist_tuple_t *tuple_i, *tuple_i_h;

	len = sbjtlist_sortedarray_length(array);
	h = len * 10 / 13;

	for (;;) {
		swaps = 0;
		for (i = 0; i + h < len; ++i) {
			sbjtlist_sortedarray_getbyindex(array, i, &tuple_i);
			sbjtlist_sortedarray_getbyindex(array, i + h, &tuple_i_h);
			result = sbjtlist_sort_compare(tuple_i, tuple_i_h, by);
			if (result > 0) {
				sbjtlist_sort_swap(tuple_i, tuple_i_h);
				++swaps;
			}
		}
		if (h == 1) {
			if (swaps == 0) {
				break;
			}
		} else {
			h = h * 10 / 13;
		}
	}
}

EXPORT W sbjtlist_sort(sbjtlist_t *list, W by, TC *filterword, W filterword_len)
{
	W err;

	sbjtlist_sortedarray_truncate(&list->sorted, 0);

	err = sbjtlist_copyarraywithfilter(list, filterword, filterword_len);
	if (err < 0) {
		return err;
	}
	sbjtlist_sortedarray_combsort(&list->sorted, by);

	return 0;
}

EXPORT VOID sbjtlist_clear(sbjtlist_t *list)
{
	W i, len;
	Bool found;
	sbjtlist_tuple_t *tuple;

	len = sbjtlist_originarray_length(&list->origin);
	for (i = 0; i < len; i++) {
		found = sbjtlist_originarray_getbyindex(&list->origin, i, &tuple);
		if (found == True) {
			sbjtparser_thread_delete(tuple->parser_thread);
		}
	}
	sbjtlist_originarray_truncate(&list->origin, 0);
	sbjtlist_sortedarray_truncate(&list->sorted, 0);
}

LOCAL sbjtlist_iterator_t* sbjtlist_iterator_new(Bool descending, sbjtlist_t *target)
{
	sbjtlist_iterator_t *iter;

	iter = malloc(sizeof(sbjtlist_iterator_t));
	if (iter == NULL) {
		return NULL;
	}
	iter->descending = descending;
	if (descending == True) {
		iter->i = sbjtlist_sortedarray_length(&target->sorted) - 1;
	} else {
		iter->i = 0;
	}
	iter->array = &target->sorted;

	return iter;
}

LOCAL VOID sbjtlist_iterator_delete(sbjtlist_iterator_t *iter)
{
	free(iter);
}

EXPORT Bool sbjtlist_iterator_next(sbjtlist_iterator_t *iter, sbjtlist_tuple_t **item)
{
	Bool found;

	found = sbjtlist_sortedarray_getbyindex(iter->array, iter->i, item);
	if (found == False) {
		return False;
	}

	if (iter->descending == True) {
		iter->i--;
	} else {
		iter->i++;
	}

	return True;
}

EXPORT sbjtlist_iterator_t* sbjtlist_startread(sbjtlist_t *list, Bool descending)
{
	return sbjtlist_iterator_new(descending, list);
}

EXPORT VOID sbjtlist_endread(sbjtlist_t *list, sbjtlist_iterator_t *iter)
{
	return sbjtlist_iterator_delete(iter);
}

EXPORT sbjtlist_t* sbjtlist_new()
{
	sbjtlist_t *list;
	W err;

	list = malloc(sizeof(sbjtlist_t));
	if (list == NULL) {
		return NULL;
	}
	err = sbjtlist_originarray_initialize(&list->origin);
	if (err < 0) {
		free(list);
		return NULL;
	}
	err = sbjtlist_sortedarray_initialize(&list->sorted);
	if (err < 0) {
		sbjtlist_originarray_finalize(&list->origin);
		free(list);
		return NULL;
	}
	list->sortby = SBJTLIST_SORTBY_NUMBER;

	return list;
}

EXPORT VOID sbjtlist_delete(sbjtlist_t *list)
{
	sbjtlist_clear(list);
	sbjtlist_sortedarray_finalize(&list->sorted);
	sbjtlist_originarray_finalize(&list->origin);
	free(list);
}
