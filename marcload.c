#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libpq-fe.h>

#include "iso2709.h"

static char
hex(int n)
{
	if (n < 0 || n > 15)
		return 'X';
	if (n >= 0 && n < 10)
		return '0' + n;
	else
		return 'a' + n - 10;
}

static char *
marc2hex(char *m, int len)
{
	char *res, *dst, *src;
	int n;

	res = dst = malloc(len * 2 + 1);
	src = m;
	for (n = 0; n < len; n++) {
		*dst++ = hex((*src >> 4) & 0x0f);
		*dst++ = hex(*src++ & 0x0f);
	}
	*dst = '\0';
	return res;
}

static int
marc21_read(PGconn *conn, FILE *fp)
{
	ISO2709RecordLabel label;
	char *rec, *hex, *sql;
	int reclen;
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
		hex = marc2hex(rec, reclen);
		sql = malloc(256 + strlen(hex));
		snprintf(sql, 256 + strlen(hex), "insert into test_marc "
		    "(marc21) values ('%s')", hex);
		PQexec(conn, sql);
		free(sql);
		free(hex);
		/* marc21_process_record(rec, reclen); */
		free(rec);
		nrecords++;
	}

	return nrecords == 0 ? -1 : 0;
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	PGconn *conn;

	if (argc != 2)
		errx(1, "usage: marcload <filename>");

	if ((fp = fopen(argv[1], "r")) == NULL)
		errx(1, "can't open %s", argv[1]);

	if ((conn = PQconnectdb("dbname=books user=mbalmer password=1234"))
	    == NULL)
	    	errx(1, "can't connect database");

	printf("db connected\n");
	if (marc21_read(conn, fp))
		errx(1, "error reading MARC21 file\n");
	fclose(fp);
	return 0;
}
