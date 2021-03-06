/*
** Copyright (C) 2016 Airbus Group, stephane duverger <stephane.duverger@airbus.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License along
** with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef __NESTED_PAGING_H__
#define __NESTED_PAGING_H__

#include <config.h>
#include <types.h>
#include <cr.h>
#include <pagemem.h>

#ifdef CONFIG_ARCH_AMD
#include <svm_vm.h>
#else
#include <vmx_vm.h>
#endif

/*
** Nested Paging mapping operators
*/
typedef enum
{
   NPG_OP_MAP = 0,
   NPG_OP_UNMAP,
   NPG_OP_REMAP,

} npg_op_type_t;

typedef void (*npg_op_fnc_t)(offset_t, uint64_t);

typedef struct npg_operator
{
   npg_op_fnc_t        fnc[3];
   size_t              sz, shf;
   struct npg_operator *nxt;

} __attribute__((packed)) npg_op_t;


/*
** Controlling active nested paging
*/
#define npg_get_default_paging()  (&info->vm.cpu.dflt_npg)

#define npg_get_active_paging()   (info->vm.cpu.active_npg)
#define npg_set_active_paging(_x) (info->vm.cpu.active_npg = (_x))

#define npg_set_active_paging_cpu()                             \
   ({                                                           \
      npg_cr3_set(npg_get_active_paging()->pml4);               \
      npg_set_asid(npg_get_active_paging()->asid);              \
   })

/*
** Nested Paging mapping functions
*/
npg_pte64_t* _npg_remap_finest_4K(offset_t);
npg_pte64_t* _npg_get_pte(offset_t);

void npg_map(offset_t, offset_t, uint64_t);
void npg_unmap(offset_t, offset_t);
void npg_setup_a20();


/*
** Legacy && Nested walking functions
*/

/*
** Paging walk information
** valid for Legacy and Nested
*/
#define PG_WALK_TYPE_PML4E     0
#define PG_WALK_TYPE_PDPE      1
#define PG_WALK_TYPE_PDPE_PAE  2
#define PG_WALK_TYPE_PDE64     3
#define PG_WALK_TYPE_PDE32     4
#define PG_WALK_TYPE_PTE64     5
#define PG_WALK_TYPE_PTE32     6

#define PG_WALK_ATTR_USER      (1<<0)
#define PG_WALK_ATTR_READ      (1<<1)
#define PG_WALK_ATTR_WRITE     (1<<2)
#define PG_WALK_ATTR_EXEC      (1<<3)

typedef struct page_walk_info
{
   void     *entry;
   offset_t  addr;
   size_t    size;

   union
   {
      struct
      {
         uint8_t  u:1;
         uint8_t  r:1;
         uint8_t  w:1;
         uint8_t  x:1;
         uint8_t  type:3;

      } __attribute__((packed));

      uint8_t attr;

   } __attribute__((packed));

} __attribute__((packed)) pg_wlk_t;

#ifndef __INIT__
struct vm_paging;

int  __pg_walk(cr3_reg_t*, offset_t, pg_wlk_t*);
int  __npg_walk(struct vm_paging*, offset_t, pg_wlk_t*);

#define pg_walk(v,w)           __pg_walk(&__cr3,v,w)
#define npg_walk(v,w)          __npg_walk(npg_get_active_paging(),v,w)
#endif

#endif
