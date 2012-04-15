/*
 * test.h
 *
 * Copyright (c) 2009-2012 project bchan
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
#include    <unittest_driver.h>

#ifndef __BCHANL_TEST_H__
#define __BCHANL_TEST_H__

/* test_subjectcache.c */
IMPORT VOID test_sbjtcache_main(unittest_driver_t *driver);

/* test_subjectparser.c */
IMPORT VOID test_sbjtparser_main(unittest_driver_t *driver);

/* test_subjectlist.c */
IMPORT VOID test_sbjtlist_main(unittest_driver_t *driver);

/* test_subjectlayout.c */
IMPORT VOID test_sbjtlayout_main(unittest_driver_t *driver);

/* test_bbsmenucache.c */
IMPORT VOID test_bbsmncache_main(unittest_driver_t *driver);

/* test_bbsmenuparser.c */
IMPORT VOID test_bbsmnparser_main(unittest_driver_t *driver);

/* test_bbsmenulayout.c */
IMPORT VOID test_bbsmnlayout_main(unittest_driver_t *driver);

/* test_bbsmenufilter.c */
IMPORT VOID test_bbsmnfilter_main(unittest_driver_t *driver);

/* test_extbbslist.c */
IMPORT VOID test_extbbslist_main(unittest_driver_t *driver);

#endif
