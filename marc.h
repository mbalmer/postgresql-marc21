#ifndef __MARC_H__
#define __MARC_H__

typedef struct MARC {
	char	vl_len_[4];	/* For TOAST */
	size_t	 len;
	char	data[1];
} MARC;

#endif /* __MARC_H__ */
