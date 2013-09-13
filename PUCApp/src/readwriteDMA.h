#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdint.h>

#include "/usr/include/pciDriver/lib/pciDriver.h"//TODO: smaller include
#include "/usr/include/pciDriver/driver/pciDriver.h"

void DMAKernelClearBuffer(uint32_t *bar0, uint32_t *bar1, const unsigned long test_len);
void DMAKernelMemoryWrite(uint32_t *bar0, uint32_t *bar1, uint64_t *bar2, pd_kmem_t *km, const unsigned long test_len, void *kernel_memory,int block);
void DMAKernelMemoryRead(uint32_t *bar0, uint32_t *bar1, uint64_t *bar2, pd_kmem_t *km, const unsigned long test_len, void *kernel_memory,int block);
