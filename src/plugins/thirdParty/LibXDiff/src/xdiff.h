/*
 *  LibXDiff by Davide Libenzi ( File Differential Library )
 *  Copyright (C) 2003  Davide Libenzi
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Davide Libenzi <davidel@xmailserver.org>
 *
 */

#if !defined(XDIFF_H)
#define XDIFF_H
//---OPENCOR--- BEGIN
#include "config.h"
//---OPENCOR--- END

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */


#define XDF_NEED_MINIMAL (1 << 1)

#define XDL_PATCH_NORMAL '-'
#define XDL_PATCH_REVERSE '+'
#define XDL_PATCH_MODEMASK ((1 << 8) - 1)
#define XDL_PATCH_IGNOREBSPACE (1 << 8)

#define XDL_MMB_READONLY (1 << 0)

#define XDL_MMF_ATOMIC (1 << 0)

#define XDL_BDOP_INS 1
#define XDL_BDOP_CPY 2
#define XDL_BDOP_INSB 3



/*---OPENCOR---
typedef struct s_memallocator {
*/
//---OPENCOR--- BEGIN
LIBXDIFF_EXPORT typedef struct s_memallocator {
//---OPENCOR--- END
	void *priv;
	void *(*malloc)(void *, unsigned int);
	void (*free)(void *, void *);
	void *(*realloc)(void *, void *, unsigned int);
} memallocator_t;

/*---OPENCOR---
typedef struct s_mmblock {
*/
//---OPENCOR--- BEGIN
LIBXDIFF_EXPORT typedef struct s_mmblock {
//---OPENCOR--- END
	struct s_mmblock *next;
	unsigned long flags;
	long size, bsize;
	char *ptr;
} mmblock_t;

/*---OPENCOR---
typedef struct s_mmfile {
*/
//---OPENCOR--- BEGIN
LIBXDIFF_EXPORT typedef struct s_mmfile {
//---OPENCOR--- END
	unsigned long flags;
	mmblock_t *head, *tail;
	long bsize, fsize, rpos;
	mmblock_t *rcur, *wcur;
} mmfile_t;

/*---OPENCOR---
typedef struct s_mmbuffer {
*/
//---OPENCOR--- BEGIN
LIBXDIFF_EXPORT typedef struct s_mmbuffer {
//---OPENCOR--- END
	char *ptr;
	long size;
} mmbuffer_t;

/*---OPENCOR---
typedef struct s_xpparam {
*/
//---OPENCOR--- BEGIN
LIBXDIFF_EXPORT typedef struct s_xpparam {
//---OPENCOR--- END
	unsigned long flags;
} xpparam_t;

/*---OPENCOR---
typedef struct s_xdemitcb {
*/
//---OPENCOR--- BEGIN
LIBXDIFF_EXPORT typedef struct s_xdemitcb {
//---OPENCOR--- END
	void *priv;
	int (*outf)(void *, mmbuffer_t *, int);
} xdemitcb_t;

/*---OPENCOR---
typedef struct s_xdemitconf {
*/
//---OPENCOR--- BEGIN
LIBXDIFF_EXPORT typedef struct s_xdemitconf {
//---OPENCOR--- END
	long ctxlen;
} xdemitconf_t;

/*---OPENCOR---
typedef struct s_bdiffparam {
*/
//---OPENCOR--- BEGIN
LIBXDIFF_EXPORT typedef struct s_bdiffparam {
//---OPENCOR--- END
	long bsize;
} bdiffparam_t;


/*---OPENCOR---
int xdl_set_allocator(memallocator_t const *malt);
void *xdl_malloc(unsigned int size);
void xdl_free(void *ptr);
void *xdl_realloc(void *ptr, unsigned int size);
*/
//---OPENCOR--- BEGIN
LIBXDIFF_EXPORT int xdl_set_allocator(memallocator_t const *malt);
LIBXDIFF_EXPORT void *xdl_malloc(unsigned int size);
LIBXDIFF_EXPORT void xdl_free(void *ptr);
LIBXDIFF_EXPORT void *xdl_realloc(void *ptr, unsigned int size);
//---OPENCOR--- END

/*---OPENCOR---
int xdl_init_mmfile(mmfile_t *mmf, long bsize, unsigned long flags);
void xdl_free_mmfile(mmfile_t *mmf);
int xdl_mmfile_iscompact(mmfile_t *mmf);
int xdl_seek_mmfile(mmfile_t *mmf, long off);
long xdl_read_mmfile(mmfile_t *mmf, void *data, long size);
long xdl_write_mmfile(mmfile_t *mmf, void const *data, long size);
long xdl_writem_mmfile(mmfile_t *mmf, mmbuffer_t *mb, int nbuf);
void *xdl_mmfile_writeallocate(mmfile_t *mmf, long size);
long xdl_mmfile_ptradd(mmfile_t *mmf, char *ptr, long size, unsigned long flags);
long xdl_copy_mmfile(mmfile_t *mmf, long size, xdemitcb_t *ecb);
void *xdl_mmfile_first(mmfile_t *mmf, long *size);
void *xdl_mmfile_next(mmfile_t *mmf, long *size);
long xdl_mmfile_size(mmfile_t *mmf);
int xdl_mmfile_cmp(mmfile_t *mmf1, mmfile_t *mmf2);
int xdl_mmfile_compact(mmfile_t *mmfo, mmfile_t *mmfc, long bsize, unsigned long flags);
*/
//---OPENCOR--- BEGIN
LIBXDIFF_EXPORT int xdl_init_mmfile(mmfile_t *mmf, long bsize, unsigned long flags);
LIBXDIFF_EXPORT void xdl_free_mmfile(mmfile_t *mmf);
LIBXDIFF_EXPORT int xdl_mmfile_iscompact(mmfile_t *mmf);
LIBXDIFF_EXPORT int xdl_seek_mmfile(mmfile_t *mmf, long off);
LIBXDIFF_EXPORT long xdl_read_mmfile(mmfile_t *mmf, void *data, long size);
LIBXDIFF_EXPORT long xdl_write_mmfile(mmfile_t *mmf, void const *data, long size);
LIBXDIFF_EXPORT long xdl_writem_mmfile(mmfile_t *mmf, mmbuffer_t *mb, int nbuf);
LIBXDIFF_EXPORT void *xdl_mmfile_writeallocate(mmfile_t *mmf, long size);
LIBXDIFF_EXPORT long xdl_mmfile_ptradd(mmfile_t *mmf, char *ptr, long size, unsigned long flags);
LIBXDIFF_EXPORT long xdl_copy_mmfile(mmfile_t *mmf, long size, xdemitcb_t *ecb);
LIBXDIFF_EXPORT void *xdl_mmfile_first(mmfile_t *mmf, long *size);
LIBXDIFF_EXPORT void *xdl_mmfile_next(mmfile_t *mmf, long *size);
LIBXDIFF_EXPORT long xdl_mmfile_size(mmfile_t *mmf);
LIBXDIFF_EXPORT int xdl_mmfile_cmp(mmfile_t *mmf1, mmfile_t *mmf2);
LIBXDIFF_EXPORT int xdl_mmfile_compact(mmfile_t *mmfo, mmfile_t *mmfc, long bsize, unsigned long flags);
//---OPENCOR--- END

/*---OPENCOR---
int xdl_diff(mmfile_t *mf1, mmfile_t *mf2, xpparam_t const *xpp,
	     xdemitconf_t const *xecfg, xdemitcb_t *ecb);
int xdl_patch(mmfile_t *mf, mmfile_t *mfp, int mode, xdemitcb_t *ecb,
	      xdemitcb_t *rjecb);
*/
//---OPENCOR--- BEGIN
LIBXDIFF_EXPORT int xdl_diff(mmfile_t *mf1, mmfile_t *mf2, xpparam_t const *xpp,
	     xdemitconf_t const *xecfg, xdemitcb_t *ecb);
LIBXDIFF_EXPORT int xdl_patch(mmfile_t *mf, mmfile_t *mfp, int mode, xdemitcb_t *ecb,
	      xdemitcb_t *rjecb);
//---OPENCOR--- END

/*---OPENCOR---
int xdl_merge3(mmfile_t *mmfo, mmfile_t *mmf1, mmfile_t *mmf2, xdemitcb_t *ecb,
	       xdemitcb_t *rjecb);
*/
//---OPENCOR--- BEGIN
LIBXDIFF_EXPORT int xdl_merge3(mmfile_t *mmfo, mmfile_t *mmf1, mmfile_t *mmf2, xdemitcb_t *ecb,
	       xdemitcb_t *rjecb);
//---OPENCOR--- END

/*---OPENCOR---
int xdl_bdiff_mb(mmbuffer_t *mmb1, mmbuffer_t *mmb2, bdiffparam_t const *bdp, xdemitcb_t *ecb);
int xdl_bdiff(mmfile_t *mmf1, mmfile_t *mmf2, bdiffparam_t const *bdp, xdemitcb_t *ecb);
int xdl_rabdiff_mb(mmbuffer_t *mmb1, mmbuffer_t *mmb2, xdemitcb_t *ecb);
int xdl_rabdiff(mmfile_t *mmf1, mmfile_t *mmf2, xdemitcb_t *ecb);
long xdl_bdiff_tgsize(mmfile_t *mmfp);
int xdl_bpatch(mmfile_t *mmf, mmfile_t *mmfp, xdemitcb_t *ecb);
int xdl_bpatch_multi(mmbuffer_t *base, mmbuffer_t *mbpch, int n, xdemitcb_t *ecb);
*/
//---OPENCOR--- BEGIN
LIBXDIFF_EXPORT int xdl_bdiff_mb(mmbuffer_t *mmb1, mmbuffer_t *mmb2, bdiffparam_t const *bdp, xdemitcb_t *ecb);
LIBXDIFF_EXPORT int xdl_bdiff(mmfile_t *mmf1, mmfile_t *mmf2, bdiffparam_t const *bdp, xdemitcb_t *ecb);
LIBXDIFF_EXPORT int xdl_rabdiff_mb(mmbuffer_t *mmb1, mmbuffer_t *mmb2, xdemitcb_t *ecb);
LIBXDIFF_EXPORT int xdl_rabdiff(mmfile_t *mmf1, mmfile_t *mmf2, xdemitcb_t *ecb);
LIBXDIFF_EXPORT long xdl_bdiff_tgsize(mmfile_t *mmfp);
LIBXDIFF_EXPORT int xdl_bpatch(mmfile_t *mmf, mmfile_t *mmfp, xdemitcb_t *ecb);
LIBXDIFF_EXPORT int xdl_bpatch_multi(mmbuffer_t *base, mmbuffer_t *mbpch, int n, xdemitcb_t *ecb);
//---OPENCOR--- END


#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif /* #if !defined(XDIFF_H) */

