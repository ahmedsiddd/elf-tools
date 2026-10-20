# elf-tools

Two small Linux/x86-64 tools in C for understanding what a compiler and linker produce.

- **elfinfo** parses an ELF64 file by hand (no libelf): file header, section headers, and the `.symtab` symbol table (value, size, type, binding, name).
- **minidbg** is a minimal debugger on `ptrace`: plants `int3` breakpoints at given addresses, prints registers at each hit, restores the original byte, single-steps over it and continues.

```
make
./elfinfo target                  # find the address of sq, e.g. 0x401136
./minidbg ./target 401136         # breakpoint at sq(): prints registers on each of the 3 calls
```
`target` is built with `-no-pie` so symbol addresses are the runtime addresses.

Limitations: x86-64 Linux only, no PIE support, no symbol lookup in the debugger (give it hex addresses), up to 16 breakpoints.
