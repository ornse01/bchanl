/*
 * test_main.c
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

#include	<basic.h>
#include	<btron/btron.h>

#include    <unittest_driver.h>

#include    "test.h"

EXPORT	W	MAIN(MESSAGE *msg)
{
	unittest_driver_t *driver;

	malloctest(-2);

	driver = unittest_driver_new();
	if (driver == NULL) {
		return 0;
	}

	test_sbjtcache_main(driver);
	test_sbjtparser_main(driver);
	test_sbjtlist_main(driver);
	test_sbjtlayout_main(driver);
	test_bbsmncache_main(driver);
	test_bbsmnparser_main(driver);
	test_bbsmnlayout_main(driver);
	test_bbsmnfilter_main(driver);

	unittest_driver_runnning(driver);
	unittest_driver_delete(driver);

	return 0;
}
