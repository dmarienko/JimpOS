#ifndef __MPAGING_H
#define __MPAGING_H

#include <types.h>
#include <defs.h>

#define	PRIV_PRESENT	0x001
#define	PRIV_WRITABLE	0x002
#define	PRIV_USER		0x004
#define	PRIV_FOO		0x200	/* user-defined (e.g. for COW) */
#define	PRIV_BAR		0x400	/* user-defined */
#define	PRIV_BAZ		0x800	/* user-defined */
#define	PRIV_ALL		0xFFF

/* - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * get index into page directory from virtual address */
#define	PDI(X) 	((X >> 22) & 0x3FF)

/* - - - - - - - - - - - - - - - - - - - - - - - - - 
 * get index into page table from virtual address */
#define	PTI(X) 	((X >> 12) & 0x3FF)

extern uint32_t *pg_tab_0;
extern uint32_t *pg_dir;

static	int install_page_table( uint32_t *dir, uint32_t table, uint32_t virt);
static	int map_page( uint32_t *dir, uint16_t priv, uint32_t virt, uint32_t phys);
int			init_paging();
uint32_t	unmap_page( uint32_t *dir, uint32_t virt );
int			map_mem( uint32_t *dir, uint16_t priv, uint32_t virt, uint32_t phys, uint32_t len );
void		setup_paging_mode( uint32_t pd );

uint32_t    count_memory(void);
void		dump_page_tables(uint32_t *page_dir);

#endif

