#include "common.h"
#include "syscall.h"

extern ssize_t fs_write(int fd, const void *buf, size_t len);

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  //result = 1;
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);
  switch (a[0]) {
	case SYS_none:
		r->eax = 1;
		break;
	case SYS_exit:
		_halt(a[1]);
		break;
	case SYS_write:
		//Log("call the write\n");
		r->eax = fs_write(a[1], (void *)a[2], a[3]);
		break;
	case SYS_brk:
		r->eax = 0;
		break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  //SYSCALL_ARG1(r) = result;

  return NULL;
}
