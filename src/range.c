/*
 * $Id: range.c,v 1.7 2003/05/25 18:19:16 skyper Exp $
 */
#include "default.h"
#include <stdio.h>
#include <sys/types.h>
#include "thc-rut.h"
#include "network.h"
#include "range.h"

void
IP_dinit(struct _ipranges *ipr)
{
	if (ipr->data)
		free(ipr->data);
	ipr->data = NULL;
	ipr->range = NULL;
}

/*
 * Reset range back the the beginning.
 */
void
IP_reset(struct _ipranges *ipr)
{
	//ipr->used = 0;
	ipr->tot_used = 0;
	//ipr->ofs = 0;
	ipr->range = ipr->data;	/* Start with the first range again */

	IP_range_init(ipr);
}

void
IP_range_init(struct _ipranges *ipr)
{
	ipr->used = 0;
	ipr->ofs = 0;
	ipr->next_ip = ipr->range->start;
	ipr->distance = (ipr->range->end - ipr->range->start) >> 2;
	if ((ipr->mode != IPR_MODE_SPREAD) || (ipr->distance <= 0))
		ipr->distance = 1;
	else if (ipr->distance > 259) 
		ipr->distance = 259; 
}

/*
 * Third parameter is mode.
 */
void
IP_init(struct _ipranges *ipr, char *argv[], unsigned char mode)
{
	int c;
	unsigned int n;
	unsigned int start, end;
	char *ptr, *split;
	char splitc;
	size_t len;
	char buf[16];

	memset(ipr, 0, sizeof (struct _ipranges));
	ipr->argv = argv;
	ipr->next = ipr->argv[0];

	/*
	 * Allocat enough memory to hold all the ip ranges.
	 * Ranges are terminated with start = end = 0.
	 */
	for (c = 0; ipr->argv[c]; c++)
		;;
	ipr->data = calloc(sizeof (struct _iprange), c + 1);
	ipr->range = ipr->data;
	ipr->mode = mode;

	/*
	 * Fill the array with ip ranges
	 */
	for (c = 0; ipr->argv[c]; c++)
	{
		start = 0;
		end = 0;

		ptr = ipr->argv[c];
		while (*ptr == ' ')
			ptr++;
		split = ptr + 1;
		while (1)
		{
			if (*split == '-')
				break;
			else if (*split == '/')
				break;
			else if (*split == ',')
				break;
			else if (*split == '\0')
				break;
			split++;
		}
		if (*split)
		{
			len = MIN(sizeof buf - 1, split - ptr);
			memcpy(buf, ptr, len);
			buf[len] = '\0';
			ptr = buf;
		}
		start = ntohl(inet_addr(ptr));
		splitc = *split++;
		if (splitc == '\0')
			goto next;

		while (*split == ' ')
			split++;

		if (*split)
			end = ntohl(inet_addr(split));

		if (splitc == '/')
		{
			n = end;

			if ((n > 30) || (n == 0))
				n = 32;
			start &= ~((1 << (32 - n)) - 1);
			end = start + (1 << (32 - n)) - 1;
			if (n <= 30)
			{
				start++;
				end--;
			}
		}
next:
		if (end == 0)           /* a.b.c.d */
			end = start;
		else if (end < start)   /* a.b.c.d-<n> */
			end = start + end - 1;

		ipr->data[c].start = start;
		ipr->data[c].end = end;
		ipr->total += (end - start + 1);
	}

	DEBUGF("start: %s\n", int_ntoa(htonl(ipr->range->start)));
	DEBUGF("end  : %s\n", int_ntoa(htonl(ipr->range->end)));
	/* Set current_ip */
	DEBUGF("ipr->data = %p\n", ipr->data);
	IP_range_init(ipr);
	DEBUGF("ipr->data = %p\n", ipr->data);
}

