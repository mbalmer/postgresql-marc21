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

/* This will be done in the database server */
static int
marc21_process_record(char *rec, int len)
{
	ISO2709Directory *entry;
	char field_tag[4];
	char s_field[3], s_subfield, *p, *res, *data;
	char query[8];
	int field_len;
	int field_start;

	for (entry = (ISO2709Directory *)(rec + sizeof(ISO2709RecordLabel));
	    entry->field_tag[0] != ISO2709_FT; entry++) {
		sscanf(entry->field_tag, "%3s", field_tag);
		field_tag[3] = '\0';
		sscanf(entry->field_len, "%04d", &field_len);
		sscanf(entry->field_start, "%05d", &field_start);
		/* printf("field %s of len %d starts at %d\n", field_tag,
		    field_len, field_start); */
	}
	data = (char *)entry;
	do {
		printf("MARC21 > ");
		fgets(query, sizeof query, stdin);
		if (strlen(query))
			query[strlen(query) - 1] = '\0';
		if (strlen(query) >= 3) {
			memcpy(s_field, query, 3);
			if ((p = strchr(query, '$')) != NULL)
				s_subfield = *(++p);
			else
				s_subfield = '\0';
			marc21_field(rec, data, s_field, s_subfield);
		}

	} while (strlen(query));

	return -1;
}

/* This is what will be the MARC21 loader */
static int
marc21_read(FILE *fp)
{
	ISO2709RecordLabel label;
	ISO2709Directory direntry;
	char *rec;
	int reclen, nfields, n;
	int nrecords;

	nrecords = 0;

	for (;;) {
		if (fread(&label, sizeof label, 1, fp) != 1) {
			printf("read %d MARC21 records\n", nrecords);
			break;
		}
		sscanf(label.record_len, "%05d", &reclen);
		rec = malloc(reclen);
		if (rec == NULL)
			errx(1, "memory error");
		if (fread(&rec[sizeof label], 1, reclen - sizeof label,
		    fp) != reclen - sizeof label)
			errx(1, "error reading MARC21 record");
		memcpy(rec, &label, sizeof label);
		marc21_process_record(rec, reclen);
		free(rec);
		printf("record %d, size %d\n\n", nrecords, reclen);
		nrecords++;
		break;
	}

	return nrecords == 0 ? -1 : 0;
}

int
main(int argc, char *argv[])
{
	FILE *fp;

	if (argc != 2)
		errx(1, "usage: marc21 <filename>");

	if ((fp = fopen(argv[1], "r")) == NULL)
		errx(1, "can't open %s", argv[1]);

	if (marc21_read(fp))
		errx(1, "error reading MARC21 file\n");
	fclose(fp);
	return 0;
}
