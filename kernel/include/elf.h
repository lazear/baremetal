// Format of an ELF executable file

#include <stdint.h>

#define ELF_MAGIC		0x464C457FU  // "\x7FELF" in little endian

typedef struct {
	uint32_t 	e_ident[4];     /* Magic number and other info */
	uint16_t 	e_type;                 /* Object file type */
	uint16_t 	e_machine;              /* Architecture */
	uint32_t 	e_version;              /* Object file version */
	uint32_t 	e_entry;                /* Entry point virtual address */
	uint32_t 	e_phoff;                /* Program header table file offset */
	uint32_t 	e_shoff;                /* Section header table file offset */
	uint32_t 	e_flags;                /* Processor-specific flags */
	uint16_t 	e_ehsize;               /* ELF header size in bytes */
	uint16_t 	e_phentsize;            /* Program header table entry size */
	uint16_t 	e_phnum;                /* Program header table entry count */
	uint16_t 	e_shentsize;            /* Section header table entry size */
	uint16_t 	e_shnum;                /* Section header table entry count */
	uint16_t 	e_shstrndx;             /* Section header string table index */
} elf32_ehdr;

typedef struct {
	uint32_t sh_name;
	uint32_t sh_type;
	uint32_t sh_flags;
	uint32_t sh_addr;
	uint32_t sh_offset;
	uint32_t sh_size;
	uint32_t sh_link;
	uint32_t sh_info;
	uint32_t sh_addralign;
	uint32_t sh_entsize;
} elf32_shdr;

typedef struct {
	uint32_t p_type;
	uint32_t p_offset;
	uint32_t p_vaddr;
	uint32_t p_paddr;
	uint32_t p_filesz;
	uint32_t p_memsz;
	uint32_t p_flags;
	uint32_t p_align;
} elf32_phdr;

// Values for Proghdr type
#define ELF_PROG_LOAD           1

// Flag bits for Proghdr flags
#define ELF_PROG_FLAG_EXEC      1
#define ELF_PROG_FLAG_WRITE     2
#define ELF_PROG_FLAG_READ      4
