#ifndef __ISO2709_H__
#define __ISO2709_H__

#define ISO2709_FT	0x1e	/* field terminator */
#define ISO2709_RT	0x1d	/* record terminator */

typedef struct __attribute__ ((__packed__))  ISO2709RecordLabel {
	char	record_len[5];
	char	record_status[1];
	char	record_type[1];
	char	impl_def1[2];
	char	character_coding_scheme[1];
	char	indicator_count[1];
	char	subfield_code_length[1];
	char	base_address[5];
	char	impl_def2[3];
	char	entry_map[4];
} ISO2709RecordLabel;

typedef struct __attribute__ ((__packed__)) ISO2709Directory {
	char	field_tag[3];
	char	field_len[4];
	char	field_start[5];
} ISO2709Directory;

typedef struct ISO2709Record {
	ISO2709RecordLabel	label;
} ISO2709Record;

#endif /* __ISO2709_H__ */
