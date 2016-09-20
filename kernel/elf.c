/*
elf.c
===============================================================================
MIT License
Copyright (c) 2007-2016 Michael Lazear

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
===============================================================================
*/

#include <assert.h>
#include <types.h>
#include <ext2.h>
#include <ide.h>
#include <elf.h>


char* elf_findsymbol_by_address(void* addr) {

}

void* elf_findsymbol_by_name(char* symbol) {

}

void* elf_objdump(void* data) {
	elf32_ehdr *ehdr = (elf32_ehdr*) data;

	/* Make sure the file ain't fucked */
	assert(ehdr->e_ident[0] == ELF_MAGIC);
	assert(ehdr->e_machine 	== EM_386);
	assert(ehdr->e_type		== ET_EXEC);

	printf("OBJDUMP\n");
	printf("ELF ident\t%x\t",ehdr->e_ident[0]);     
	printf("Type\t%x\t", ehdr->e_type);                
	printf("Machine\t%x\n", ehdr->e_machine);              
	printf("Version \t%x\t",ehdr->e_version);              
	printf("Entry\t%x\t",ehdr->e_entry);                
         
	printf("Flags\t%x\n",ehdr->e_flags);           
	

	/* Parse the program headers */
	elf32_phdr* phdr 		= (uint32_t) data + ehdr->e_phoff;
	elf32_phdr* last_phdr 	= (uint32_t) phdr + (ehdr->e_phentsize * ehdr->e_phnum);
	while(phdr < last_phdr) {
		printf("LOAD:\toff 0x%x\tvaddr\t0x%x\tpaddr\t0x%x\n\t\tfilesz\t%d\tmemsz\t%d\talign\t%d\t\n",
		 	phdr->p_offset, phdr->p_vaddr, phdr->p_paddr, phdr->p_filesz, phdr->p_memsz, phdr->p_align);
		phdr++;
	} 

	//uint32_t* buf = ext2_file_seek(ext2_inode(1,14), BLOCK_SIZE, ehdr->e_shoff);

	/* Parse the section headers */
	elf32_shdr* shdr 		= (uint32_t) data + ehdr->e_shoff;
	elf32_shdr* sh_str		= (uint32_t) shdr + (ehdr->e_shentsize * ehdr->e_shstrndx);
	elf32_shdr* last_shdr 	= (uint32_t) shdr + (ehdr->e_shentsize * ehdr->e_shnum);

	elf32_shdr* strtab 		= NULL;
	elf32_shdr* symtab		= NULL;

	char* string_table 		= (uint32_t) data + sh_str->sh_offset;

	shdr++;					// Skip null entry
	int q = 0;

	printf("Sections:\nIdx   Name\tSize\t\tAddress \tOffset\tAlign\n");
	while (shdr < last_shdr) {	
		printf("%d:   %s\t%x\t%x\t%d\t%x\n", 
			q++, string_table + shdr->sh_name, shdr->sh_size,
			shdr->sh_addr, shdr->sh_offset, shdr->sh_addralign);
		if (strcmp(string_table + shdr->sh_name, ".symtab") == 0) 
			symtab = shdr;
		if (strcmp(string_table + shdr->sh_name, ".strtab") == 0)
			strtab = shdr;
		shdr++;
	}

	if (!strtab || !symtab) {
		vga_pretty("ERROR: Could not load symbol table", 0x4);
		return;
	}

	elf32_sym* sym 		= (uint32_t) data + symtab->sh_offset;
	elf32_sym* last_sym = (uint32_t) sym + symtab->sh_size;
	void* strtab_d 		= (uint32_t) data + strtab->sh_offset;
	void* func;
	/* Output symbol information*/
	/*
	while(sym < last_sym) {
		if (sym->st_name) 
			printf("%s\t0x%x\n", sym->st_name + strtab_d, sym->st_value);
		sym++;
	}
	*/
}


void elf_load() {
	uint32_t* elf_pd = k_create_pagedir(0, 0, 0x7);	

	k_map_kernel(elf_pd);
	k_swap_pd(elf_pd);

	uint32_t* data = ext2_open(ext2_inode(1,12));

	printf("Data @ 0x%x\n", data);
	elf32_ehdr * ehdr = (elf32_ehdr*) data; 

	assert(ehdr->e_ident[0] == ELF_MAGIC);

	elf_objdump(data);
	elf32_phdr* phdr 		= (uint32_t) data + ehdr->e_phoff;
	elf32_phdr* last_phdr 	= (uint32_t) phdr + (ehdr->e_phentsize * ehdr->e_phnum);
	
	uint32_t off = (phdr->p_vaddr - phdr->p_paddr);


	// Make a new pagedirectory for the process to execute in.



//	k_paging_map(elf_pd, P2V(elf_pd), 0x3);


	printf("%x\n", data);
	while(phdr < last_phdr) {
		// printf("LOAD:\toff 0x%x\tvaddr\t0x%x\tpaddr\t0x%x\n\t\tfilesz\t%d\tmemsz\t%d\talign\t%d\t\n",
		//  	phdr->p_offset, phdr->p_vaddr, phdr->p_paddr, phdr->p_filesz, phdr->p_memsz, phdr->p_align);
		
		uint32_t phys = k_page_alloc();					// Allocate a physical page

		k_paging_map(phys, phdr->p_vaddr, 0x7);			// Map that page in KPD
		//_paging_map(elf_pd, phys, phdr->p_vaddr, 0x7);		// Map page in new PD
		printf("Mapping %x to %x\n", phys, phdr->p_vaddr);
		// We are still in the kernel page directory, so copy data now
		
		memcpy(phdr->p_vaddr, (uint32_t)data + phdr->p_offset, phdr->p_memsz);

		// Unmap in kernel?
	//	k_paging_unmap(phdr->p_vaddr);
		phdr++;
	}
	int (*entry)(int, char**);
	entry = (void(*)(void))(ehdr->e_entry);

	//printf("func2 result: %d\n", func(2));
	char* d[] = { "user.elf", "HELLO WORLD" };
	entry(2, d);
	//free(data);

	extern uint32_t* KERNEL_PAGE_DIRECTORY;
	//k_swap_pd(KERNEL_PAGE_DIRECTORY);
	printf("HELLO?\n");
}