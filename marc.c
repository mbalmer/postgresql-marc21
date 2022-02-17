#include "postgres.h"

#include "fmgr.h"
#include "libpq/pqformat.h"

#include "iso2709.h"
#include "marc.h"

PG_MODULE_MAGIC;

Datum		marc_in(PG_FUNCTION_ARGS);
Datum		marc_out(PG_FUNCTION_ARGS);

#ifdef HAVE_RECV_SEND
Datum		marc_recv(PG_FUNCTION_ARGS);
Datum		marc_send(PG_FUNCTION_ARGS);
#endif

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

static int
unhex(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return 0;
}

static MARC *
hex2marc(char *src)
{
	MARC *res;
	char *dst;
	int n;

	res = (MARC *)palloc(VARHDRSZ + strlen(src) / 2 + sizeof(size_t));
	SET_VARSIZE(res, VARHDRSZ + strlen(src) / 2 + sizeof(size_t));
	res->len = strlen(src) / 2;

	dst = res->data;
	for (n = 0; n < res->len; n++) {
		*dst = unhex(*src++) << 4;
		*dst++ |= (unhex(*src++) & 0x0f);
	}
	return res;
}

static char *
marc2hex(MARC *m)
{
	char *res, *dst, *src;
	int n;

	res = dst = palloc(m->len * 2 + 1);
	src = m->data;
	for (n = 0; n < m->len; n++) {
		*dst++ = hex((*src >> 4) & 0x0f);
		*dst++ = hex(*src++ & 0x0f);
	}
	*dst = '\0';
	return res;
}

/* Input/Output functions */
PG_FUNCTION_INFO_V1(marc_in);

Datum
marc_in(PG_FUNCTION_ARGS)
{
	char *str = PG_GETARG_CSTRING(0);
	PG_RETURN_POINTER(hex2marc(str));
}

PG_FUNCTION_INFO_V1(marc_out);

Datum
marc_out(PG_FUNCTION_ARGS)
{
	MARC *marc = (MARC *)PG_DETOAST_DATUM(PG_GETARG_POINTER(0));
	PG_RETURN_VARCHAR_P(marc2hex(marc));
}

PG_FUNCTION_INFO_V1(marc_field);

Datum
marc_field(PG_FUNCTION_ARGS)
{
	MARC *rec = (MARC *)PG_DETOAST_DATUM(PG_GETARG_POINTER(0));
	char *field = PG_GETARG_CSTRING(1);
	ISO2709Directory *entry;
	char *res, *data;
	int field_len;
	int field_start;

	for (entry = (ISO2709Directory *)(rec->data +
	    sizeof(ISO2709RecordLabel));
	    entry->field_tag[0] != ISO2709_FT; entry++)
	    	;
	data = (char *)entry;

	for (entry = (ISO2709Directory *)(rec->data +



	    sizeof(ISO2709RecordLabel));
	    entry->field_tag[0] != ISO2709_FT; entry++) {
	    	if (!memcmp(entry->field_tag, field, 3)) {
			sscanf(entry->field_len, "%04d", &field_len);
			sscanf(entry->field_start, "%05d", &field_start);
			res = malloc(VARHDRSZ + field_len);
			if (res == NULL)
				PG_RETURN_NULL();
			SET_VARSIZE(res, VARHDRSZ + field_len);
			memcpy(res + VARHDRSZ, data + field_start, field_len);
			/* res[field_len] = '\0'; */
			PG_RETURN_VARCHAR_P(res);
		}
	}
	PG_RETURN_NULL();
}


#ifdef HAVE_RECV_SEND
/* Binary Input/Output functions (optional) */
PG_FUNCTION_INFO_V1(marc_recv);

Datum
marc_recv(PG_FUNCTION_ARGS)
{
	StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
	Complex *result;

	result = (Complex *) palloc(sizeof(Complex));
	result->x = pq_getmsgfloat8(buf);
	result->y = pq_getmsgfloat8(buf);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(marc_send);

Datum
marc_send(PG_FUNCTION_ARGS)
{
	Complex    *complex = (Complex *) PG_GETARG_POINTER(0);
	StringInfoData buf;

	pq_begintypsend(&buf);
	pq_sendfloat8(&buf, complex->x);
	pq_sendfloat8(&buf, complex->y);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}
#endif
