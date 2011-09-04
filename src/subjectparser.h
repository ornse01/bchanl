/*
 * subjectparser.h
 *
 * Copyright (c) 2009-2011 project bchan
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
#include    "subjectcache.h"

#ifndef __SUBJECTPARSER_H__
#define __SUBJECTPARSER_H__

struct sbjtparser_thread_t_ {
	UB *number;
	W number_len;
	TC *title;
	W title_len;
	W i_titlesepareter;
};
typedef struct sbjtparser_thread_t_ sbjtparser_thread_t;
typedef struct sbjtparser_t_ sbjtparser_t;

EXPORT VOID sbjtparser_thread_gettitlestr(sbjtparser_thread_t *thr, TC **str, W *len);
EXPORT VOID sbjtparser_thread_getresnumstr(sbjtparser_thread_t *thr, TC **str, W *len);
EXPORT VOID sbjtparser_thread_delete(sbjtparser_thread_t *thr);

EXPORT sbjtparser_t* sbjtparser_new(sbjtcache_t *cache);
EXPORT VOID sbjtparser_delete(sbjtparser_t *parser);
EXPORT W sbjtparser_getnextthread(sbjtparser_t *parser, sbjtparser_thread_t **thr);
EXPORT VOID sbjtparser_clear(sbjtparser_t *parser);

#endif
