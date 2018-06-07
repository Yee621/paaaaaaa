#include "fs.h"

extern void fb_write(const void *buf, off_t offset, size_t len);
extern void ramdisk_write(const void *buf, off_t offset, size_t len);
extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern void dispinfo_read(void *buf, off_t offset, size_t len);
extern size_t events_read(void *buf, size_t len);

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
  file_table[FD_FB].size = _screen.height * _screen.width * 4;
  file_table[FD_FB].open_offset = 0;
}

int fs_open(const char *pathname, int flags, int mode){
	for(int i = 0; i < NR_FILES; i++){
		if(strcmp(file_table[i].name, pathname) == 0){
			file_table[i].open_offset = 0;
			//Log("in the open, filename:%s, fd = %d, file size = %d, file open_offset = %d, NR_FILES: %d\n",pathname, i, file_table[i].size, file_table[i].open_offset,NR_FILES);
			return i;
		}
	}
	assert(0);
	return -1;
}

ssize_t fs_read(int fd, void *buf, size_t len){
    ssize_t size , rlen;
	//Log("in the read, fd = %d, file size = %d, len = %d, file open_offset = %d\n", fd, file_table[fd].size, len, file_table[fd].open_offset);
    size = file_table[fd].size - file_table[fd].open_offset;
    rlen = len > size ? size : len;
    switch (fd) {
    	case FD_STDOUT:
	    case FD_STDERR:
		    return -1;
		case FD_EVENTS:
			return events_read(buf, len);
	    case FD_DISPINFO:
		    dispinfo_read(buf, file_table[fd].open_offset, rlen);
			break;
		default:
			if (fd < 6 || fd >= NR_FILES)
				return -1;
			ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, rlen);
			break;
	}
	file_table[fd].open_offset += rlen;
	return rlen;
}

ssize_t fs_write(int fd, uint8_t *buf, size_t len){
    ssize_t size , wlen;
	//Log("in the write, fd = %d, file size = %d, len = %d, file open_offset = %d\n", fd, file_table[fd].size, len, file_table[fd].open_offset);
    size = file_table[fd].size - file_table[fd].open_offset;
    wlen = len > size ? size : len;
    switch (fd){
    	case FD_STDOUT:
	    case FD_STDERR:
			for(int i = 0; i < len; i++)
				_putc(((char*)buf)[i]);
		    return len;
	    case FD_FB:
		    fb_write(buf, file_table[fd].open_offset, wlen);
			break;
		default:
			if (fd < 6 || fd >= NR_FILES)
				return -1;
			ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, wlen);
			break;
	}
	file_table[fd].open_offset += wlen;
	return wlen;
}

off_t fs_lseek(int fd, off_t offset, int whence){
	//Log("fd = %d, NR_FILES = %d, offset = %d, open_offset = %d",fd,NR_FILES,offset,file_table[fd].open_offset);
	if(fd >= NR_FILES)
		return 0;
	switch(whence){
		case SEEK_SET:
			//Log("1");
			if(offset >= 0 && offset <= file_table[fd].size){
				file_table[fd].open_offset = offset;
				return file_table[fd].open_offset;
			}
			else
				return -1;
		case SEEK_CUR:
			//Log("2");
			offset += file_table[fd].open_offset;
			break;
		case SEEK_END:
			//Log("3");
			offset += file_table[fd].size;
			break;
		default:
			//Log("4");
			return -1;
	}
	if(offset < 0 || offset > file_table[fd].size)
		return -1;
	//Log("%d",file_table[fd].open_offset);
	file_table[fd].open_offset = offset;
	return file_table[fd].open_offset;
/*	Finfo *fp = &file_table[fd];
	if(fd >= NR_FILES) return 0;
	switch(whence){
		case SEEK_SET:
			break;
		case SEEK_CUR:
			offset = fp->open_offset + offset;
            break;
		case SEEK_END:
			offset = fp->size + offset;
			break;
	    default: 
			return -1;
	}
	if(offset < 0 || offset > fp->size) 
		return -1;
	fp->open_offset = offset;
	return fp->open_offset;*/
}

int fs_close(int fd){
	return 0;
}

size_t fs_filesz(int fd){
	return file_table[fd].size;
}
