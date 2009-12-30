/*
 * subjectcache.h
 *
 * Copyright (c) 2009 project bchan
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

#ifndef __SUBJECTCACHE_H__
#define __SUBJECTCACHE_H__

typedef struct sbjtcache_t_ sbjtcache_t;
typedef struct sbjtcache_datareadcontext_t_ sbjtcache_datareadcontext_t;

IMPORT sbjtcache_t* sbjtcache_new();
IMPORT VOID sbjtcache_delete(sbjtcache_t *cache);
IMPORT W sbjtcache_appenddata(sbjtcache_t *cache, UB *data, W len);
IMPORT VOID sbjtcache_cleardata(sbjtcache_t *cache);
IMPORT VOID sbjtcache_getlatestheader(sbjtcache_t *cache, UB **header, W *len);
IMPORT W sbjtcache_updatelatestheader(sbjtcache_t *cache, UB *header, W len);
IMPORT VOID sbjtcache_gethost(sbjtcache_t *cache, UB **host, W *len);
IMPORT W sbjtcache_updatehost(sbjtcache_t *cache, UB *host, W len);
IMPORT VOID sbjtcache_getboard(sbjtcache_t *cache, UB **borad, W *len);
IMPORT W sbjtcache_updateboard(sbjtcache_t *cache, UB *borad, W len);
IMPORT W sbjtcache_datasize(sbjtcache_t *cache);
IMPORT sbjtcache_datareadcontext_t* sbjtcache_startdataread(sbjtcache_t *cache, W start);
IMPORT VOID sbjtcache_enddataread(sbjtcache_t *cache, sbjtcache_datareadcontext_t *context);

IMPORT Bool sbjtcache_datareadcontext_nextdata(sbjtcache_datareadcontext_t *context, UB **bin, W *len);

#endif
