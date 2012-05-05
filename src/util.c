/*
 * util.c
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
#include	<bstdio.h>
#include	<tcode.h>
#include	<tstring.h>
#include	<errcode.h>

#include    "util.h"

#ifdef BCHANL_CONFIG_DEBUG
# define DP(arg) printf arg
# define DP_ER(msg, err) printf("%s (%d/%x)\n", msg, err>>16, err)
#else
# define DP(arg) /**/
# define DP_ER(msg, err) /**/
#endif

EXPORT TCURL_CHECK_VALID_BBSURL tcurl_check_valid_bbsurl(CONST TC *url, W url_len)
{
	TC scheme[] = {TK_h, TK_t, TK_t, TK_p, TK_COLN, TK_SLSH, TK_SLSH, TNULL};
	TC *p;
	W cmp;

	cmp = tc_strncmp(url, scheme, 7);
	if (cmp != 0) {
		return TCURL_CHECK_VALID_BBSURL_INVALID_SCHEME;
	}
	if (url_len == 7) {
		return TCURL_CHECK_VALID_BBSURL_INVALID_HOST;
	}
	p = tc_strchr(url+7, TK_SLSH);
	if (p == NULL) {
		return TCURL_CHECK_VALID_BBSURL_INVALID_PATH;
	}
	if ((p+1) == (url+url_len)) {
		return TCURL_CHECK_VALID_BBSURL_INVALID_PATH;
	}
	if (p == (url+7)) {
		return TCURL_CHECK_VALID_BBSURL_INVALID_HOST;
	}
	if ((p+1) == (url+url_len)) {
		return TCURL_CHECK_VALID_BBSURL_INVALID_PATH;
	}
	p = tc_strchr(p+1, TK_SLSH);
	if (p == NULL) {
		return TCURL_CHECK_VALID_BBSURL_NO_LAST_SLSH;
	}
	if ((p+1) != (url+url_len)) {
		return TCURL_CHECK_VALID_BBSURL_INVALID_PATH;
	}

	return TCURL_CHECK_VALID_BBSURL_OK;
}
