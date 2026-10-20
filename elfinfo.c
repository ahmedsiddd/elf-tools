// elfinfo: minimal ELF64 reader (header, section headers, symbol table). Linux, little-endian ELF64 only.
#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static const char *st(unsigned t) { static const char *n[] = {"NULL","PROGBITS","SYMTAB","STRTAB","RELA","HASH","DYNAMIC","NOTE","NOBITS","REL","SHLIB","DYNSYM"}; return t < 12 ? n[t] : "OTHER"; }
int main(int ac, char **av) {
  if (ac < 2) { fprintf(stderr, "usage: %s <elf64-file>\n", av[0]); return 2; }
  FILE *f = fopen(av[1], "rb"); if (!f) { perror(av[1]); return 1; }
  fseek(f, 0, SEEK_END); long n = ftell(f); rewind(f);
  unsigned char *m = malloc(n); if (fread(m, 1, n, f) != (size_t)n) { fprintf(stderr, "read error\n"); return 1; }
  Elf64_Ehdr *h = (Elf64_Ehdr *)m;
  if ((size_t)n < sizeof *h || memcmp(h->e_ident, ELFMAG, SELFMAG) || h->e_ident[EI_CLASS] != ELFCLASS64) { fprintf(stderr, "not an ELF64 file\n"); return 1; }
  printf("type=%u machine=%u entry=0x%lx\nprogram headers: %u at 0x%lx, section headers: %u at 0x%lx\n\n", h->e_type, h->e_machine, h->e_entry, h->e_phnum, h->e_phoff, h->e_shnum, h->e_shoff);
  Elf64_Shdr *s = (Elf64_Shdr *)(m + h->e_shoff); const char *shs = (char *)m + s[h->e_shstrndx].sh_offset;
  printf("%-3s %-20s %-9s %-10s %-8s %s\n", "#", "name", "type", "addr", "offset", "size");
  for (int i = 0; i < h->e_shnum; i++) printf("%-3d %-20s %-9s 0x%08lx %-8lx %lu\n", i, shs + s[i].sh_name, st(s[i].sh_type), s[i].sh_addr, s[i].sh_offset, s[i].sh_size);
  for (int i = 0; i < h->e_shnum; i++) if (s[i].sh_type == SHT_SYMTAB) {   // symbol table; names come from the linked string table
    Elf64_Sym *y = (Elf64_Sym *)(m + s[i].sh_offset); const char *str = (char *)m + s[s[i].sh_link].sh_offset;
    printf("\nsymbols (%lu):\n%-18s %-6s %-7s %-7s %s\n", s[i].sh_size / sizeof *y, "value", "size", "type", "bind", "name");
    for (size_t k = 0; k < s[i].sh_size / sizeof *y; k++) if (*(str + y[k].st_name))
      printf("0x%016lx %-6lu %-7s %-7s %s\n", y[k].st_value, y[k].st_size, ELF64_ST_TYPE(y[k].st_info) == STT_FUNC ? "FUNC" : ELF64_ST_TYPE(y[k].st_info) == STT_OBJECT ? "OBJECT" : "other", ELF64_ST_BIND(y[k].st_info) == STB_GLOBAL ? "GLOBAL" : ELF64_ST_BIND(y[k].st_info) == STB_LOCAL ? "LOCAL" : "WEAK", str + y[k].st_name);
  }
  return 0;
}
