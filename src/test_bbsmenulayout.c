/*
 * test_bbsmenulayout.c
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

#include    "test.h"

#include    "bbsmenulayout.h"

#include    <btron/btron.h>
#include	<btron/dp.h>
#include	<tcode.h>
#include    <bstdio.h>
#include    <bstring.h>
#include    <bstdlib.h>

#include    <unittest_driver.h>

#include    "bbsmenuparser.h"
#include    "bbsmenucache.h"

LOCAL UB test_bbsmnlayout_testdata_01[] = {
"
<HTML>
<HEAD>
<TITLE>BBS MENU for test</TITLE>
</HEAD>

<BODY>
<BR>
<font size=2>

<A HREF=http://xxx.2ch.net/AAA/>AAA</A>

<BR><BR><B>Cat 1</B><BR>
<A HREF=http://xxx.2ch.net/BBB/>BBB</A><br>
<A HREF=http://yyy.2ch.net/CCC/>CCC</A><br>
<A HREF=http://yyy.2ch.net/DDD/>DDD</A><br>
<A HREF=http://yyy.2ch.net/EEE/>EEE</A>

<BR><BR><B>Cat 2</B><BR>
<A HREF=http://xxx.2ch.net/FFF/>FFF</A><br>
<A HREF=http://xxx.2ch.net/GGG/>GGG</A><br>
<A HREF=http://xxx.2ch.net/HHH/>HHH</A><br>
<A HREF=http://xxx.2ch.net/III/>III</A><br>

<BR><BR><B>Cat 3</B><BR>
<A HREF=http://xxx.2ch.net/>other</A><br>
<A HREF=http://yyy.2ch.net/>other2</A><br>
<A HREF=http://zzz.2ch.net/zzz/zzz.php>other3</A><br>

<BR><BR><B>Cat 4</B><BR>
<A HREF=http://www.chokanji.com/ TARGET=_blank>ck4</A><br>
<A HREF=http://www.parsonal-media.co.jp/ TARGET=_blank>ck4</A><br>

</BODY></HTML>
"
};

LOCAL BMP* test_bbsmnlayout_util_alloc_BMP()
{
	W size;
	BMP* dest;

	dest = malloc(sizeof(UW)+sizeof(UH)*2+sizeof(RECT)+sizeof(UB*)*1);
	dest->planes = 1;
	dest->pixbits = (32 << 8)|28;
	dest->rowbytes = 100*4;
	dest->bounds = (RECT){{0,0,100,100}};

	size = ((dest->bounds.c.right - dest->bounds.c.left) * (dest->pixbits >> 8) + 15) / 16 * 2 * (dest->bounds.c.bottom - dest->bounds.c.top);

	dest->baseaddr[0] = malloc(size);

	return dest;
}

LOCAL VOID test_bbsmnlayout_util_free_BMP(BMP *bmp)
{
	free(bmp->baseaddr[0]);
	free(bmp);
}

LOCAL UNITTEST_RESULT test_bbsmnlayout_1()
{
	BMP *bmp;
	GID gid;
	W err;
	bbsmnlayout_t *layout;
	bbsmncache_t *cache;
	bbsmnparser_t *parser;
	bbsmnparser_item_t *item = NULL;
	UNITTEST_RESULT result = UNITTEST_RESULT_PASS;

	bmp = test_bbsmnlayout_util_alloc_BMP();
	if (bmp == NULL) {
		return UNITTEST_RESULT_FAIL;
	}
	gid = gopn_mem(NULL, bmp, NULL);
	if (gid < 0) {
		return UNITTEST_RESULT_FAIL;
	}

	cache = bbsmncache_new();
	bbsmncache_appenddata(cache, test_bbsmnlayout_testdata_01, strlen(test_bbsmnlayout_testdata_01));

	parser = bbsmnparser_new(cache);
	layout = bbsmnlayout_new(gid);

	for (;;) {
		err = bbsmnparser_getnextitem(parser, &item);
		if (err != 1) {
			break;
		}
		if (item != NULL) {
			bbsmnlayout_appenditem(layout, item);
		} else {
			break;
		}
	}

	bbsmnlayout_delete(layout);
	bbsmnparser_delete(parser);

	bbsmncache_delete(cache);

	gcls_env(gid);
	test_bbsmnlayout_util_free_BMP(bmp);

	return result;
}

EXPORT VOID test_bbsmnlayout_main(unittest_driver_t *driver)
{
	UNITTEST_DRIVER_REGIST(driver, test_bbsmnlayout_1);
}
