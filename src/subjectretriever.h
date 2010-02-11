/*
 * subjectretriever.h
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

#include	<basic.h>
#include	"subjectcache.h"

#ifndef __SUBJECTRETREIEVER_H__
#define __SUBJECTRETREIEVER_H__

typedef struct sbjtretriever_t_ sbjtretriever_t;

IMPORT sbjtretriever_t* sbjtretriever_new();
IMPORT VOID sbjtretriever_delete(sbjtretriever_t *retriever);
IMPORT W sbjtretriever_sendrequest(sbjtretriever_t *retriever, sbjtcache_t *cache);

#endif