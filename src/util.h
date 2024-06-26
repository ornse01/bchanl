/*
 * util.h
 *
 * Copyright (c) 2012 project bchan
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

#ifndef __UTIL_H__
#define __UTIL_H__

enum TCURL_CHECK_VALID_BBSURL_ {
	TCURL_CHECK_VALID_BBSURL_OK,
	TCURL_CHECK_VALID_BBSURL_INVALID_SCHEME,
	TCURL_CHECK_VALID_BBSURL_INVALID_HOST,
	TCURL_CHECK_VALID_BBSURL_INVALID_PATH,
	TCURL_CHECK_VALID_BBSURL_NO_LAST_SLSH,
};
typedef enum TCURL_CHECK_VALID_BBSURL_ TCURL_CHECK_VALID_BBSURL;
IMPORT TCURL_CHECK_VALID_BBSURL tcurl_check_valid_bbsurl(CONST TC *url, W url_len);

IMPORT W tray_pushstring(TC *str, W len);
IMPORT W tray_popstring(TC *str, W len);
IMPORT W tray_deletedata();
IMPORT Bool tray_isempty();
IMPORT W tray_getextbbsinfo(TC *name, W *name_len, TC *url, W *url_len);

#endif
