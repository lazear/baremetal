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

void elf_objdump(void* data) {
	elf32_ehdr *ehdr = (elf32_ehdr*) data;

	/* Make sure the file ain't fucked */
	assert(ehdr->e_ident[0] == ELF_MAGIC);
	assert(ehdr->e_machine 	== EM_386);
	assert(ehdr->e_type		== ET_EXEC);

	/* Parse the program headers */
	elf32_phdr* phdr 		= (uint32_t) data + ehdr->e_phoff;
	elf32_phdr* last_phdr 	= (uint32_t) phdr + (ehdr->e_phentsize * ehdr->e_phnum);
	while(phdr < last_phdr) {
		printf("LOAD:\toff 0x%x\tvaddr\t0x%x\tpaddr\t0x%x\n\t\tfilesz\t%d\tmemsz\t%d\talign\t%d\t\n",
		 	phdr->p_offset, phdr->p_vaddr, phdr->p_paddr, phdr->p_filesz, phdr->p_memsz, phdr->p_align);
		phdr++;
	} 

	uint32_t* buf = ext2_file_seek(ext2_inode(1,14), BLOCK_SIZE, ehdr->e_shoff);

	/* Parse the section headers */
	elf32_shdr* shdr 		= (uint32_t) buf + ehdr->e_shoff;
	elf32_shdr* sh_str		= (uint32_t) shdr + (ehdr->e_shentsize * ehdr->e_shstrndx);
	elf32_shdr* last_shdr 	= (uint32_t) shdr + (ehdr->e_shentsize * ehdr->e_shnum);

	char* string_table 		= (uint32_t) data + sh_str->sh_offset;

	shdr++;					// Skip null entry
	int q = 0;

	printf("Sections:\nIdx   Name\tSize\t\tAddress \tOffset\tAlign\n");
	while (shdr < last_shdr) {	
		printf("%d:   %s\t%x\t%x\t%d\t%x\n", 
			q++, string_table + shdr->sh_name, shdr->sh_size,
			shdr->sh_addr, shdr->sh_offset, shdr->sh_addralign);
		shdr++;
	}

	free(buf);
}


void elf_load() {
	uint32_t* data = ext2_open(ext2_inode(1,14));


	elf32_ehdr * ehdr = (elf32_ehdr*) data; 

	assert(ehdr->e_ident[0] == ELF_MAGIC);
	printf("ELF ident\t%x\t",ehdr->e_ident[0]);     
	printf("Type\t%x\t", ehdr->e_type);                
	printf("Machine\t%x\n", ehdr->e_machine);              
	printf("Version  \t%x\t",ehdr->e_version);              
	printf("Entry\t%x\t",ehdr->e_entry);                
	printf("PH off\t%x\n",ehdr->e_phoff);                
	printf("SH off\t\t%x\t",ehdr->e_shoff);                
	printf("Flags\t%x\t",ehdr->e_flags);           
	printf("EH size\t%x\n", ehdr->e_ehsize);               
	printf("PH ent size\t%x\t", ehdr->e_phentsize);            
	printf("PH #\t%x\t", ehdr->e_phnum);                
	printf("SH ent size\t%x\n", ehdr->e_shentsize);            
	printf("SH #\t%x\t", ehdr->e_shnum);               
	printf("SH str\t%x\n", ehdr->e_shstrndx);    


	elf_objdump(data);

	free(data);
	return;


	elf32_phdr* phdr = (elf32_phdr*) ((uint32_t) data + ehdr->e_phoff);
	printf("LOAD: off 0x%x vaddr 0x%x paddr 0x%x filesz 0x%x memsz 0x%x\n",
		 				phdr->p_offset, phdr->p_vaddr, phdr->p_paddr, phdr->p_filesz, phdr->p_memsz);
	k_paging_map(k_page_alloc(), phdr->p_vaddr, 0x7);
	memcpy(phdr->p_vaddr, (uint32_t)data + phdr->p_offset, phdr->p_memsz);

	phdr++;
	printf("LOAD: off 0x%x vaddr 0x%x paddr 0x%x filesz 0x%x memsz 0x%x\n",
		 				phdr->p_offset, phdr->p_vaddr, phdr->p_paddr, phdr->p_filesz, phdr->p_memsz);
	k_paging_map(k_page_alloc(), phdr->p_vaddr, 0x7);
	memcpy(phdr->p_vaddr, (uint32_t)data + phdr->p_offset, phdr->p_memsz);
	


	void (*entry)(void);
	entry = (void(*)(void))(ehdr->e_entry);

	free(data);

	printf("entry: %x\n", entry);

	entry();

}