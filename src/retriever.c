/*
 * retriever.c
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
#include	<bstdlib.h>
#include	<bstdio.h>
#include	<bstring.h>
#include	<errcode.h>
#include	<btron/btron.h>
#include	<btron/bsocket.h>
#include	<util/zlib.h>

#include    "retriever.h"

#ifdef BCHANL_CONFIG_DEBUG
# define DP(arg) printf arg
# define DP_ER(msg, err) printf("%s (%d/%x)\n", msg, err>>16, err)
#else
# define DP(arg) /**/
# define DP_ER(msg, err) /**/
#endif

struct retriever_t_ {
	SOCKADDR addr;
	W buffer_len;
	B *buffer;
	W rcv_len;
    /* http */
	W header_len;
	W responsebody_len;
	B *responsebody;
	/* zlib */
	W inflate_buffer_len;
	W inflate_written_len;
	B *inflate_buffer;
};

EXPORT W retriever_gethost(retriever_t *retriever, UB *host)
{
	W err;
	B buf[HBUFLEN];
	HOSTENT ent;
	struct sockaddr_in *addr_in;

	err = so_gethostbyname(host, &ent, buf);
	if (err < 0) {
		return err;
	}

	addr_in = (struct sockaddr_in *)&(retriever->addr);
	addr_in->sin_family = AF_INET;
	addr_in->sin_port = htons( 80 );
	addr_in->sin_addr.s_addr = *(unsigned int *)(ent.h_addr_list[0]);

	return 0;
}

EXPORT W retriver_connectsocket(retriever_t *retriever)
{
	W sock, err;

	sock = so_socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		return sock;
	}
	err = so_connect(sock, &(retriever->addr), sizeof(SOCKADDR));
	if (err < 0) {
		so_close(sock);
		return err;
	}

	return sock;
}

EXPORT W retriever_recieve(retriever_t *retriever, W sock)
{
	W off, rcvlen;

	retriever->rcv_len = 0;

	for (off = 0; ;) {
		rcvlen = so_recv (sock, retriever->buffer + off, retriever->buffer_len - off, 0);
		if (rcvlen == EX_CONNABORTED) {
			return 0;
		}
		if (rcvlen <= 0) {
			return rcvlen;
		}
		off += rcvlen;
		retriever->rcv_len += rcvlen;

		if (off >= retriever->buffer_len) {
			retriever->buffer_len += 1024;
			retriever->buffer = realloc(retriever->buffer, retriever->buffer_len);
			if (retriever->buffer == NULL) {
				return -1;
			}
		}
	}

	return 0;
}

EXPORT W retriever_parsehttpresponse(retriever_t *retriever)
{
	W i,len;
	B *str;

	len = retriever->rcv_len;
	str = retriever->buffer;

	for (i = 0; i < len; i++) {
		if ((str[i] == '\r')
			&&(str[i+1] == '\n')
			&&(str[i+2] == '\r')
			&&(str[i+3] == '\n')) {
			break;
		}
	}

	if (i == len) {
		return -1;
	}

	str[i] = NULL;
	str[i+1] = NULL;
	str[i+2] = NULL;
	str[i+3] = NULL;

	retriever->header_len = i;
	retriever->responsebody = str + i + 4;
	retriever->responsebody_len = len - i - 4;

	return 0;
}

EXPORT W retriever_parse_response_status(retriever_t *retriever)
{
	B *str;

	str = strchr(retriever->buffer, ' ');
	if (str == NULL) {
		return -1;
	}
	return atoi(str+1);
}

LOCAL W retriever_checkheader_content_encoding(retriever_t *retriever)
{
	B *str;

	str = strstr(retriever->buffer, "Content-Encoding: gzip");
	if (str != NULL) {
		return 1;
	}
	str = strstr(retriever->buffer, "Content-Encoding");
	if (str != NULL) {
		return -1;
	}
	return 0;
}

LOCAL W retriever_checkheader_transfer_encoding(retriever_t *retriever)
{
	B *str;

	str = strstr(retriever->buffer, "Transfer-Encoding: chunked");
	if (str != NULL) {
		return 1;
	}
	return 0;
}

typedef struct {
	retriever_t *retriever;
	Bool is_chunked;
	UB *next;
} retriever_chunkediterator_t;

LOCAL VOID datretriver_startchunkediterator(retriever_t *retriever, retriever_chunkediterator_t *iterator)
{
	W is_chunked;

	iterator->retriever = retriever;
	is_chunked = retriever_checkheader_transfer_encoding(retriever);
	if (is_chunked) {
		iterator->is_chunked = True;
	} else {
		iterator->is_chunked = False;
	}
	iterator->next = retriever->responsebody;
}

LOCAL VOID retriever_chunkediterator_getnext(retriever_chunkediterator_t *iterator, UB **ptr, W *len)
{
	W len0;
	B *next;

	if (iterator->is_chunked == False) {
		if (iterator->next != NULL) {
			*ptr = iterator->retriever->responsebody;
			*len = iterator->retriever->responsebody_len;
			iterator->next = NULL;
		} else {
			*ptr = NULL;
			*len = 0;
		}
		return;
	}

	len0 = strtol(iterator->next, &next, 16);
	if (len0 == 0) {
		*ptr = NULL;
		*len = 0;
		return;
	}
	*ptr = next + 2;
	*len = len0;

	iterator->next = next + 2 + len0;
}

/* http://ghanyan.monazilla.org/gzip.html */
/* http://ghanyan.monazilla.org/gzip.txt */
LOCAL W gzip_headercheck(UB *buf)
{
	W i, method, flags, len;

	if ((buf[0] != 0x1f)||(buf[1] != 0x8b)) {
		DP(("error gzip format 1\n"));
		return -1;
	}

	i = 2;
	method = buf[i++];
	flags  = buf[i++];
	if (method != Z_DEFLATED || (flags & 0xE0) != 0) {
		DP(("error gzip format 2\n"));
		return -1;
	}
	i += 6;

	if (flags & 0x04) { /* skip the extra field */
		len = 0;
		len = (W)buf[i++];
		len += ((W)buf[i++])<<8;
		i += len;
	}
	if (flags & 0x08) { /* skip the original file name */
		while (buf[i++] != 0) ;
	}
	if (flags & 0x10) {   /* skip the .gz file comment */
		while (buf[i++] != 0) ;
	}
	if (flags & 0x02) {  /* skip the header crc */
		i += 2;
	}

	return i;
} 

EXPORT W retriever_decompress(retriever_t *retriever)
{
	retriever_chunkediterator_t iter;
	UB *bin;
	W gzipheader_len, bin_len, err;
	z_stream z;

	err = retriever_checkheader_content_encoding(retriever);
	if (err < 0) {
		return -1; /* unsupported */
	}
	if (err == 0) {
		return 0; /* not compressed */
	}

	datretriver_startchunkediterator(retriever, &iter);
	retriever_chunkediterator_getnext(&iter, &bin, &bin_len);

	gzipheader_len = gzip_headercheck(bin);
	if (gzipheader_len < 0) {
		DP_ER("gzip_headercheck error\n", gzipheader_len);
		return -1;
	}

	retriever->inflate_buffer_len = 1024;
	retriever->inflate_buffer = malloc(sizeof(B)*1024);
	if (retriever->inflate_buffer == NULL) {
		DP_ER("inflate buffer alloc", 0);
		return -1;
	}

    z.zalloc = Z_NULL;
    z.zfree = Z_NULL;
    z.opaque = Z_NULL;

	z.next_in = Z_NULL;
	z.avail_in = 0;

	err = inflateInit2(&z, -MAX_WBITS);
	//err = inflateInit(&z);
    if (err != Z_OK) {
		DP_ER("error inflateInit", err);
        return -1;
    }

	z.next_in = bin + gzipheader_len;
	z.avail_in = bin_len - gzipheader_len;
    z.next_out = retriever->inflate_buffer;
    z.avail_out = retriever->inflate_buffer_len;

	for (;;) {
        err = inflate(&z, Z_NO_FLUSH);
        if (err == Z_STREAM_END) {
			break;
		}
        if (err != Z_OK) {
			DP_ER("inflate error", err);
			inflateEnd(&z);
			return -1;
        }
        if (z.avail_out == 0) {
			retriever->inflate_written_len += 1024;
			retriever->inflate_buffer_len += 1024;
			retriever->inflate_buffer = realloc(retriever->inflate_buffer, retriever->inflate_buffer_len);
			if (retriever->inflate_buffer == NULL) {
				DP_ER("inflate buffer realloc", 0);
				inflateEnd(&z);
				return -1;
			}
			z.next_out = retriever->inflate_buffer + retriever->inflate_written_len;
            z.avail_out = 1024;
        }
		if (z.avail_in == 0) {
			retriever_chunkediterator_getnext(&iter, &bin, &bin_len);
			if (bin == NULL) {
				DP(("full consumed\n"));
				break;
			}
			z.next_in = bin;
			z.avail_in = bin_len;
		}
	}
	retriever->inflate_written_len += 1024 - z.avail_out;

	inflateEnd(&z);

	return 0;
}

EXPORT VOID retriever_clearbuffer(retriever_t *retriever)
{
	if (retriever->inflate_buffer != NULL) {
		free(retriever->inflate_buffer);
	}
	retriever->rcv_len = 0;
	retriever->header_len = 0;
	retriever->responsebody_len = 0;
	retriever->responsebody = NULL;
	retriever->inflate_buffer_len = 0;
	retriever->inflate_written_len = 0;
	retriever->inflate_buffer = NULL;
}

EXPORT B* retriever_getheader(retriever_t *retriever)
{
	return retriever->buffer;
}

EXPORT W retriever_getheaderlength(retriever_t *retriever)
{
	return retriever->header_len;
}

EXPORT B* retriever_getbody(retriever_t *retriever)
{
	if (retriever->inflate_buffer != NULL) {
		return retriever->inflate_buffer;
	}
	return retriever->responsebody;
}

EXPORT W retriever_getbodylength(retriever_t *retriever)
{
	if (retriever->inflate_buffer != NULL) {
		return retriever->inflate_written_len;
	}
	return retriever->responsebody_len;
}

#ifdef BCHANL_CONFIG_DEBUG
EXPORT VOID retriever_dumpheader(retriever_t *retriever)
{
	W i;

	for (i=0;i<retriever->header_len;i++) {
		putchar(retriever->buffer[i]);
	}
	printf("\n");
	for (i=0;i<16;i++) {
		printf("%02x ", retriever->responsebody[i]);
	}
	printf("\n");
}

EXPORT VOID retriever_debugprint(retriever_t *retriever)
{
	struct sockaddr_in *addr_in = (struct sockaddr_in *)&(retriever->addr);

	printf("ip = %s\n", inet_ntoa(addr_in->sin_addr));
	printf("buffer len = %d\n", retriever->buffer_len);
	printf("receive len = %d\n", retriever->rcv_len);
	printf("header len = %d\n", retriever->header_len);
	printf("response body len = %d\n", retriever->responsebody_len);
	printf("inflate buffer len = %d\n", retriever->inflate_buffer_len);
	printf("inflate written len = %d\n", retriever->inflate_written_len);
	printf("\n");
}
#endif

EXPORT retriever_t* retriever_new()
{
	retriever_t *retriever;

	retriever = malloc(sizeof(retriever_t));
	if (retriever == NULL) {
		goto error_fetch;
	}

	retriever->buffer_len = 1024;
	retriever->buffer = malloc(sizeof(B)*1024);
	retriever->rcv_len = 0;
	retriever->header_len = 0;
	retriever->responsebody_len = 0;
	retriever->responsebody = NULL;
	retriever->inflate_buffer_len = 0;
	retriever->inflate_written_len = 0;
	retriever->inflate_buffer = NULL;

	return retriever;

error_fetch:
	return NULL;
}

EXPORT VOID retriever_delete(retriever_t *retriever)
{
	if (retriever->inflate_buffer != NULL) {
		free(retriever->inflate_buffer);
	}

	if (retriever->buffer != NULL) {
		free(retriever->buffer);
	}

	free(retriever);
}
