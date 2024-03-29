#include "ft_nm.h"

char *sec_names[] = {
	".text", ".code", ".data", ".rodata", ".sdata",
	".bss", ".sbss", ".debug", ".fini", ".init",
	"*DEBUG*", ".drectve", ".edata", ".idata", ".pdata",
	"rdata", ".scommon", "vars", "zerovars"
};

char sec_types[] = {
	't',	't',	'd',	'r',	'g',
	'b',	's', 	'N',	't',	't',
	'N',	'i',	'e',	'i',	'p',
	'r',	'c',	'd',	'b'
};

char *get_name(char *strtab, size_t idx, size_t shsize) {
	size_t len = ft_strnlen(strtab + idx, shsize - idx);
	if (len + idx >= shsize)
		return (NULL);
	return (strtab + idx);
}

int check_opts(data_t *data, symbol_t sym) {
	if (data->opt & OUND && sym.type != 'U')
		return (0);
	else if (data->opt & OGLOB && ELF_ST_BIND(sym.info) != STB_GLOBAL)
		return (0);
	else if (!(data->opt & OALL)) {
		if (ft_strchr("NAa", sym.type))
			return (0);
		if (sym.name && sym.name[0] == '.')
			return (0);
	}
	return (1);
}

//64 BITS

char *symbol_name_64(Elf64_Sym *sym, Elf64_Shdr *sh_symtab, Elf64_Shdr *sh_strtab, fdata_t *fdata) {
	Elf64_Shdr	*sh;

	//if the symbol is a section
	if (ELF_ST_TYPE(sym->st_info) == STT_SECTION) {
		//check section index
		if (sym->st_shndx > fdata->shtab.count) {
			SET_ERROR(ERWFFMT);
			return (NULL);
		}

		//get related section header
		sh = (Elf64_Shdr*)SHDR(fdata, sym->st_shndx);

		//check section header
		if (check_shdr(sh, fdata) == -1)
			return (NULL);

		//get related string table section header
		sh_strtab = (Elf64_Shdr*)SHDR(fdata, fdata->shtab.sh_stidx);

		//check section header
		if (check_shdr(sh_strtab, fdata) == -1)
			return (NULL);

		//get string table
		return (get_name(STRTAB(fdata, sh_strtab), sh->sh_name, sh_strtab->sh_size));
	}
	else {
		//get related string table section header
		sh_strtab = (Elf64_Shdr*)SHDR(fdata, sh_symtab->sh_link);

		//check section header
		if (check_shdr(sh_strtab, fdata) == -1)
			return (NULL);

		//get string table
		return (get_name(STRTAB(fdata, sh_strtab), sym->st_name, sh_strtab->sh_size));
	}
}

char sec_type_64(fdata_t *fdata, Elf64_Sym *sym, Elf64_Shdr *sh_strtab) {
	char		*sh_name;
	Elf64_Shdr	*sh;
	int			i = 0;

	//get related section header
	sh = (Elf64_Shdr*)SHDR(fdata, sym->st_shndx);

	//check section header
	if (check_shdr(sh, fdata) == -1)
		return (0);

	//get section name
	sh_name = get_name(STRTAB(fdata, sh_strtab), sh->sh_name, sh_strtab->sh_size);

	//check section name
	if (sh_name == NULL)
		return (0);

	while (i < 19) {
		if (!ft_strncmp(sh_name, sec_names[i], ft_strlen(sec_names[i])))
			return (sec_types[i]);
		i++;
	}
	if (sh->sh_flags & SHF_ALLOC) {
		if (!(sh->sh_flags & SHF_WRITE))
			return ('r');
		if (sh->sh_flags & SHF_EXECINSTR)
			return ('t');
		return ('d');
	}
	if ((sh->sh_flags & SEC_HAS_CONTENTS) == 0) {
		if (sh->sh_flags & SEC_SMALL_DATA)
			return ('s');
		else
			return ('b');
	}
	if (sh->sh_flags & SEC_DEBUGGING)
		return ('N');
	if ((sh->sh_flags & SEC_HAS_CONTENTS) && (sh->sh_flags & SEC_READONLY))
		return ('n');
	return ('?');
}

uint8_t symbol_type_64(Elf64_Sym *sym, Elf64_Shdr *sh_strtab, fdata_t *fdata) {
	uint8_t 	type;
	char		*sh_name;

	//if section is common
	if (sym->st_shndx == SHN_COMMON)
		return ('C');
	//if section is undefined
	if (sym->st_shndx == SHN_UNDEF) {
		//check if symbol is weak
		if (ELF_ST_BIND(sym->st_info) == STB_WEAK) {
			//check if symbol is an object
			if (ELF_ST_TYPE(sym->st_info) == STT_OBJECT)
				return ('v');
			else
				return ('w');
		}
		return ('U');
	}
	if (ELF_ST_TYPE(sym->st_info) == STT_GNU_IFUNC)
		return ('I');
	if (ELF_ST_BIND(sym->st_info) == STB_WEAK) {
		if (ELF_ST_TYPE(sym->st_info) == STT_OBJECT)
			return ('V');
		else
			return ('W');
	}
	if (!(ELF_ST_BIND(sym->st_info) == STB_GLOBAL)
		&& !(ELF_ST_BIND(sym->st_info) == STB_LOCAL))
		return ('?');
	if (sym->st_shndx == SHN_ABS)
		type = 'a';
	else {
		type = sec_type_64(fdata, sym, sh_strtab);
	}
	if (ft_strchr(MINTYPE, type) && ELF_ST_BIND(sym->st_info) == STB_GLOBAL)
		type = ft_toupper(type);

	return (type);
}


int parse_symtab_64(data_t *data, fdata_t *fdata, Elf64_Shdr *sh_symtab) {
	Elf64_Sym	*sym;
	size_t		sym_off;
	symbol_t	symbol;
	Elf64_Shdr	*sh_strtab;

	//get section header string table
	sh_strtab = (Elf64_Shdr*)SHDR(fdata, fdata->shtab.sh_stidx);

	//check section header
	if (check_shdr(sh_strtab, fdata) == -1)
		return (-1);

	for (int i = 1; i < (sh_symtab->sh_size / sh_symtab->sh_entsize); i++) {
		//get current symbol
		sym_off = sh_symtab->sh_offset + i * sh_symtab->sh_entsize;
		sym = (Elf64_Sym*)((byte_t*)fdata->map + sym_off);

		//get symbol name
		symbol.name = symbol_name_64(sym, sh_symtab, sh_strtab, fdata);

		//get symbol type
		symbol.type = symbol_type_64(sym, sh_strtab, fdata);

		symbol.value = sym->st_value;

		symbol.info = sym->st_info;

		if (check_opts(data, symbol))
			add_symbol(fdata, symbol);
	}

	return (0);
}

//32 BITS

char *symbol_name_32(Elf32_Sym *sym, Elf32_Shdr *sh_symtab, Elf32_Shdr *sh_strtab, fdata_t *fdata) {
	Elf32_Shdr	*sh;

	//if the symbol is a section
	if (ELF_ST_TYPE(sym->st_info) == STT_SECTION) {
		//check section index
		if (sym->st_shndx > fdata->shtab.count) {
			SET_ERROR(ERWFFMT);
			return (NULL);
		}

		//get related section header
		sh = (Elf32_Shdr*)SHDR(fdata, sym->st_shndx);

		//check section header
		if (check_shdr(sh, fdata) == -1)
			return (NULL);

		//get related string table section header
		sh_strtab = (Elf32_Shdr*)SHDR(fdata, fdata->shtab.sh_stidx);

		//check section header
		if (check_shdr(sh_strtab, fdata) == -1)
			return (NULL);

		//get string table
		return (get_name(STRTAB(fdata, sh_strtab), sh->sh_name, sh_strtab->sh_size));
	}
	else {
		//get related string table section header
		sh_strtab = (Elf32_Shdr*)SHDR(fdata, sh_symtab->sh_link);

		//check section header
		if (check_shdr(sh_strtab, fdata) == -1)
			return (NULL);

		//get string table
		return (get_name(STRTAB(fdata, sh_strtab), sym->st_name, sh_strtab->sh_size));
	}
}

char sec_type_32(fdata_t *fdata, Elf32_Sym *sym, Elf32_Shdr *sh_strtab) {
	char		*sh_name;
	Elf32_Shdr	*sh;
	int			i = 0;

	//get related section header
	sh = (Elf32_Shdr*)SHDR(fdata, sym->st_shndx);

	//check section header
	if (check_shdr(sh, fdata) == -1)
		return (0);

	//get section name
	sh_name = get_name(STRTAB(fdata, sh_strtab), sh->sh_name, sh_strtab->sh_size);

	//check section name
	if (sh_name == NULL)
		return (0);

	while (i < 19) {
		if (!ft_strncmp(sh_name, sec_names[i], ft_strlen(sec_names[i])))
			return (sec_types[i]);
		i++;
	}
	if (sh->sh_flags & SHF_ALLOC) {
		if (!(sh->sh_flags & SHF_WRITE))
			return ('r');
		if (sh->sh_flags & SHF_EXECINSTR)
			return ('t');
		return ('d');
	}
	if ((sh->sh_flags & SEC_HAS_CONTENTS) == 0) {
		if (sh->sh_flags & SEC_SMALL_DATA)
			return ('s');
		else
			return ('b');
	}
	if (sh->sh_flags & SEC_DEBUGGING)
		return ('N');
	if ((sh->sh_flags & SEC_HAS_CONTENTS) && (sh->sh_flags & SEC_READONLY))
		return ('n');
	return ('?');
}

uint8_t symbol_type_32(Elf32_Sym *sym, Elf32_Shdr *sh_strtab, fdata_t *fdata) {
	uint8_t 	type;
	char		*sh_name;

	//if section is common
	if (sym->st_shndx == SHN_COMMON)
		return ('C');
	//if section is undefined
	if (sym->st_shndx == SHN_UNDEF) {
		//check if symbol is weak
		if (ELF_ST_BIND(sym->st_info) == STB_WEAK) {
			//check if symbol is an object
			if (ELF_ST_TYPE(sym->st_info) == STT_OBJECT)
				return ('v');
			else
				return ('w');
		}
		return ('U');
	}
	if (ELF_ST_TYPE(sym->st_info) == STT_GNU_IFUNC)
		return ('I');
	if (ELF_ST_BIND(sym->st_info) == STB_WEAK) {
		if (ELF_ST_TYPE(sym->st_info) == STT_OBJECT)
			return ('V');
		else
			return ('W');
	}
	if (!(ELF_ST_BIND(sym->st_info) == STB_GLOBAL)
		&& !(ELF_ST_BIND(sym->st_info) == STB_LOCAL))
		return ('?');
	if (sym->st_shndx == SHN_ABS)
		type = 'a';
	else {
		type = sec_type_32(fdata, sym, sh_strtab);
	}
	if (ft_strchr(MINTYPE, type) && ELF_ST_BIND(sym->st_info) == STB_GLOBAL)
		type = ft_toupper(type);

	return (type);
}


int parse_symtab_32(data_t *data, fdata_t *fdata, Elf32_Shdr *sh_symtab) {
	Elf32_Sym	*sym;
	size_t		sym_off;
	symbol_t	symbol;
	Elf32_Shdr	*sh_strtab;

	//get section header string table
	sh_strtab = (Elf32_Shdr*)SHDR(fdata, fdata->shtab.sh_stidx);

	//check section header
	if (check_shdr(sh_strtab, fdata) == -1)
		return (-1);

	for (int i = 1; i < (sh_symtab->sh_size / sh_symtab->sh_entsize); i++) {
		//get current symbol
		sym_off = sh_symtab->sh_offset + i * sh_symtab->sh_entsize;
		sym = (Elf32_Sym*)((byte_t*)fdata->map + sym_off);

		//get symbol name
		symbol.name = symbol_name_32(sym, sh_symtab, sh_strtab, fdata);

		//get symbol type
		symbol.type = symbol_type_32(sym, sh_strtab, fdata);

		symbol.value = sym->st_value;

		symbol.info = sym->st_info;

		if (check_opts(data, symbol))
			add_symbol(fdata, symbol);
	}

	return (0);
}
