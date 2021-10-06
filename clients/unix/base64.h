/*
 * Hacked up from https://github.com/badzong/base64 to use url safe base64
 * without padding characters
 * https://datatracker.ietf.org/doc/html/rfc4648#page-7
 * https://datatracker.ietf.org/doc/html/rfc4648#section-3.2
 *
 * */
//#include <stdint.h>
//#include <stdlib.h>
//#include <string.h>


static char encoder[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

int
base64_encode(char *dest, int size, unsigned char *src, int slen)
{
	int i, j;
	uint32_t a, b, c, triple;

	for (i = 0, j = 0; i < slen;)
	{
		a = src[i++];

		// b and c may be off limit
		b = i < slen ? src[i++] : 0;
		c = i < slen ? src[i++] : 0;

		triple = (a << 16) + (b << 8) + c;

		dest[j++] = encoder[(triple >> 18) & 0x3F];
		dest[j++] = encoder[(triple >> 12) & 0x3F];
		dest[j++] = encoder[(triple >> 6) & 0x3F];
		dest[j++] = encoder[triple & 0x3F];
	}

	// Pad zeroes at the end
	switch (slen % 3)
	{
	case 0:
		dest[j] = 0;
	case 1:
		dest[j - 2] = 0;
	case 2:
		dest[j - 1] = 0;
	}
}
