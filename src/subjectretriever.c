/*
 * subjectretriever.c
 *
 * Copyright (c) 2009-2010 project bchan
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
#include	<bstdlib.h>
#include	<bstdio.h>
#include	<bstring.h>
#include	<errcode.h>
#include	<btron/btron.h>
#include	<btron/bsocket.h>

#include    "subjectretriever.h"

#include    "retriever.h"
#include    "subjectcache.h"

#ifdef BCHANL_CONFIG_DEBUG
# define DP(arg) printf arg
# define DP_ER(msg, err) printf("%s (%d/%x)\n", msg, err>>16, err)
#else
# define DP(arg) /**/
# define DP_ER(msg, err) /**/
#endif

struct sbjtretriever_t_ {
	retriever_t *retr;
};

EXPORT sbjtretriever_t* sbjtretriever_new()
{
	sbjtretriever_t *retriever;

	retriever = (sbjtretriever_t*)malloc(sizeof(sbjtretriever_t));
	if (retriever == NULL) {
		return NULL;
	}
	retriever->retr = retriever_new();
	if (retriever->retr == NULL) {
		free(retriever);
		return NULL;
	}

	return retriever;
}

EXPORT VOID sbjtretriever_delete(sbjtretriever_t *retriever)
{
	retriever_delete(retriever->retr);
	free(retriever);
}

#define SO_ERR_SEND_LEN(sockID, str, len) \
   err = so_send(sockID, (str), (len), 0); \
   if(err < 0){ \
     return err; \
   }

#define SO_ERR_SEND(sockID, str) SO_ERR_SEND_LEN(sockID, (str), strlen((str)))

LOCAL W sbjtretriver_sendheader(W sock, UB *host, UB *board)
{
	W err;

	SO_ERR_SEND(sock, "GET /");
	SO_ERR_SEND(sock, board);
	SO_ERR_SEND(sock, "/subject.txt HTTP/1.1\r\n");
	SO_ERR_SEND(sock, "Accept-Encoding: gzip\r\n");
	SO_ERR_SEND(sock, "HOST: ");
	SO_ERR_SEND(sock, host);
	SO_ERR_SEND(sock, "\r\n");
	SO_ERR_SEND(sock, "Accept: */*\r\n");
	SO_ERR_SEND(sock, "Referer: http://");
	SO_ERR_SEND(sock, host);
	SO_ERR_SEND(sock, "/");
	SO_ERR_SEND(sock, board);
	SO_ERR_SEND(sock, "/");
	SO_ERR_SEND(sock, "/\r\n");
	SO_ERR_SEND(sock, "Accept-Language: ja\r\nUser-Agent: Monazilla/1.00 (bchanl/0.101)\r\nConnection: close\r\n\r\n");

	return 0;
}
/* from http://www.monazilla.org/index.php?e=197 */
#if 0
"GET /[板名]/subject.txt HTTP/1.1
Accept-Encoding: gzip
Host: [サーバー]
Accept: */*
Referer: http://[サーバー]/[板名]/
Accept-Language: ja
If-Modified-Since: Tue, 23 Dec 2008 14:08:21 GMT
If-None-Match: \"3462-1b80-45eb74ece1f40\"
User-Agent: Monazilla/1.00 (monaweb/1.00)
Connection: close
"
#endif

EXPORT W sbjtretriever_sendrequest(sbjtretriever_t *retriever, sbjtcache_t *cache)
{
	W sock, err, ret = -1, len, status, host_len, board_len;
	UB *bin, *host, *board;

	retriever_clearbuffer(retriever->retr);

	sbjtcache_gethost(cache, &host, &host_len);
	sbjtcache_getboard(cache, &board, &board_len);

	err = retriever_gethost(retriever->retr, host);
	if (err < 0) {
		return err;
	}
	sock = retriver_connectsocket(retriever->retr);
	if (sock < 0) {
		return sock;
	}

	err = sbjtretriver_sendheader(sock, host, board);
	if (err < 0) {
		so_close(sock);
		return err;
	}

	err = retriever_recieve(retriever->retr, sock);
	if (err < 0) {
		so_close(sock);
		return err;
	}

	err = retriever_parsehttpresponse(retriever->retr);
	if (err < 0) {
		so_close(sock);
		return err;
	}
	retriever_dumpheader(retriever->retr);

	err = retriever_decompress(retriever->retr);
	if (err < 0) {
		so_close(sock);
		return err;
	}

	status = retriever_parse_response_status(retriever->retr);
	if (status == 200) {
		bin = retriever_getbody(retriever->retr);
		len = retriever_getbodylength(retriever->retr);
		sbjtcache_cleardata(cache);
		sbjtcache_appenddata(cache, bin, len);
		ret = 0;
	}

	so_close(sock);

	retriever_clearbuffer(retriever->retr);

	return ret;
}
