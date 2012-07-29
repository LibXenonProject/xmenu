
#include "elf_abi.h"

extern void puthex(int x);
extern void rputc(char c);

static inline void elf_memset(unsigned char *dst, int v, int l)
{
	while (l--) *dst++ = v;
}

static inline void elf_memcpy(unsigned char *dst, const unsigned char *src, int l)
{
	while (l--)
		*dst++ = *src++;
}

#define LINESIZE 16
static inline void sync(volatile void *addr)
{
	asm volatile ("dcbst 0, %0" : : "b" (addr));
	asm volatile ("icbi 0, %0" : : "b" (addr));
}
      
static inline void elf_sync_before_exec(unsigned char *dst, int l)
{
	while (l > LINESIZE)
	{
		sync(dst);
		dst += LINESIZE;
		l -= LINESIZE;
	}
}

unsigned long __attribute__ ((section (".elfldr_b"))) load_elf_image (void *addr) 
{
	Elf32_Ehdr *ehdr;
	Elf32_Shdr *shdr;
	unsigned char *strtab = 0;
	unsigned char *image;
	int i;
	
	ehdr = (Elf32_Ehdr *) addr;
	/* Find the section header string table for output info */
	shdr = (Elf32_Shdr *) (addr + ehdr->e_shoff +
			       (ehdr->e_shstrndx * sizeof (Elf32_Shdr)));

	if (shdr->sh_type == SHT_STRTAB)
		strtab = (unsigned char *) (addr + shdr->sh_offset);

	/* Load each appropriate section */
	for (i = 0; i < ehdr->e_shnum; ++i) {
		shdr = (Elf32_Shdr *) (addr + ehdr->e_shoff +
				       (i * sizeof (Elf32_Shdr)));

		if (!(shdr->sh_flags & SHF_ALLOC) || shdr->sh_size == 0)
			continue;
		
#if 0
		if (strtab) {
			printf ("%sing %s @ 0x%08lx (%ld bytes)\n",
				(shdr->sh_type == SHT_NOBITS) ?
					"Clear" : "Load",
				&strtab[shdr->sh_name],
				(unsigned long) shdr->sh_addr,
				(long) shdr->sh_size);
		}
#endif
/*		rputc('d');
		rputc('=');
		puthex(shdr->sh_addr); rputc(',');
		rputc('s');
		rputc('=');
		puthex((unsigned char *) addr + shdr->sh_offset); rputc(',');
		rputc('l');
		rputc('=');
		puthex(shdr->sh_size); rputc('\n');
	*/	if (shdr->sh_type == SHT_NOBITS) {
			elf_memset ((void *)shdr->sh_addr, 0, shdr->sh_size);
		} else {
			image = (unsigned char *) addr + shdr->sh_offset;
			elf_memcpy ((void *) shdr->sh_addr,
				(const void *) image,
				shdr->sh_size);
		}
		elf_sync_before_exec ((void*)shdr->sh_addr, shdr->sh_size);
	}
/*	rputc('E');
	rputc('P');
	rputc('D');
	rputc('=');
	puthex(*(int*)ehdr->e_entry);
	rputc('\n');
	rputc('\n');
*/
	return ehdr->e_entry;
}
