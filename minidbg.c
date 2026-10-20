// minidbg: tiny x86-64 Linux debugger on ptrace. Usage: minidbg <prog> [hex-address ...]
// Sets an int3 (0xCC) breakpoint at each address (use non-PIE binaries, addresses from elfinfo), runs, and at every
// hit prints rip/rax/rbx/rcx/rdx, restores the original byte, rewinds rip and single-steps over it.
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
int main(int ac, char **av) {
  if (ac < 2) { fprintf(stderr, "usage: %s <prog> [hex-addr ...]\n", av[0]); return 2; }
  pid_t c = fork();
  if (!c) { ptrace(PTRACE_TRACEME, 0, 0, 0); execv(av[1], (char *[]){av[1], 0}); perror("exec"); _exit(1); }
  int w; waitpid(c, &w, 0);                                                 // child stopped at exec
  int nb = ac - 2; long addr[16], orig[16]; if (nb > 16) nb = 16;
  for (int i = 0; i < nb; i++) {
    addr[i] = strtol(av[i + 2], 0, 16); orig[i] = ptrace(PTRACE_PEEKTEXT, c, addr[i], 0);
    ptrace(PTRACE_POKETEXT, c, addr[i], (orig[i] & ~0xffL) | 0xcc);        // plant int3
  }
  ptrace(PTRACE_CONT, c, 0, 0);
  while (waitpid(c, &w, 0) > 0 && WIFSTOPPED(w)) {
    struct user_regs_struct r; ptrace(PTRACE_GETREGS, c, 0, &r);
    int hit = -1; for (int i = 0; i < nb; i++) if ((long)r.rip - 1 == addr[i]) hit = i;
    if (hit < 0) { ptrace(PTRACE_CONT, c, 0, WSTOPSIG(w) == SIGTRAP ? 0 : WSTOPSIG(w)); continue; }
    printf("breakpoint @0x%lx  rip=0x%llx rax=0x%llx rbx=0x%llx rcx=0x%llx rdx=0x%llx\n", addr[hit], r.rip - 1, r.rax, r.rbx, r.rcx, r.rdx);
    ptrace(PTRACE_POKETEXT, c, addr[hit], orig[hit]); r.rip--; ptrace(PTRACE_SETREGS, c, 0, &r);  // restore byte, rewind
    ptrace(PTRACE_SINGLESTEP, c, 0, 0); waitpid(c, &w, 0);                                         // step over original instruction
    ptrace(PTRACE_POKETEXT, c, addr[hit], (orig[hit] & ~0xffL) | 0xcc);                            // re-arm
    ptrace(PTRACE_CONT, c, 0, 0);
  }
  printf("program exited\n"); return 0;
}
