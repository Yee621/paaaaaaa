#include "common.h"
#include "syscall.h"
#include "fs.h"

uintptr_t sys_write(int fd, const uint8_t *buf, size_t count){
	uintptr_t i = 0;
	if (fd == 1 || fd == 2){
			for(; i < count; i++)
				_putc(((char*)buf)[i]);
		}
	return i;
}


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
		Log();
		//r->eax = sys_write(a[1], (uint8_t *)a[2], a[3]);
		r->eax = fs_write(a[1], (uint8_t *)a[2], a[3]);
		break;
	case SYS_brk:
		r->eax = 0;
		break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  //SYSCALL_ARG1(r) = result;

  return NULL;
}
