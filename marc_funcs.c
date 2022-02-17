#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iso2709.h"

static void
marc21_field(char *rec, char *data, char field[3], char subfield)
{
	ISO2709Directory *entry;
	char field_tag[4];
	char s_field[3], s_subfield, *p, *res;
	char query[8];
	int field_len;
	int field_start;

	for (entry = (ISO2709Directory *)(rec + sizeof(ISO2709RecordLabel));
	    entry->field_tag[0] != ISO2709_FT; entry++) {
	    	if (!memcmp(entry->field_tag, field, 3)) {
			sscanf(entry->field_len, "%04d", &field_len);
			sscanf(entry->field_start, "%05d", &field_start);
			res = malloc(field_len + 1);
			if (res == NULL)
				errx(1, "memory error");
			memcpy(res, data + field_start, field_len);
			res[field_len] = '\0';
			printf("%s\n", res);
		}
	}
}
