/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <limits.h>
#include <metal/io.h>
#include <metal/sys.h>

int metal_io_normal_mem_block_read(struct metal_io_region *io,
					 unsigned long offset,
					 void *restrict dst,
					 memory_order order,
					 int len)
{
	(void)order;
	memcpy(dst, metal_io_virt(io, offset), len);
	return len;
}

int metal_io_normal_mem_block_write(struct metal_io_region *io,
					   unsigned long offset,
					   const void *restrict src,
					   memory_order order,
					   int len)
{
	(void)order;
	memcpy(metal_io_virt(io, offset), src, len);
	return len;
}

void metal_io_normal_mem_block_set(struct metal_io_region *io,
					  unsigned long offset,
					  unsigned char value,
					  memory_order order,
					  int len)
{
	(void)order;
	memset(metal_io_virt(io, offset), value, len);
}

struct metal_io_ops metal_weak metal_io_normal_mem_ops = {
	.read = NULL,
	.write = NULL,
	.block_read = metal_io_normal_mem_block_read,
	.block_write = metal_io_normal_mem_block_write,
	.block_set = metal_io_normal_mem_block_set,
	.close = NULL,
};

int metal_io_device_mem_block_read(struct metal_io_region *io,
					 unsigned long offset,
					 void *restrict dst,
					 memory_order order,
					 int len)
{
	void *ptr = metal_io_virt(io, offset);
	int retlen;

	retlen = len;
	atomic_thread_fence(order);
	while (len && (((uintptr_t)dst % sizeof(int)) ||
		       ((uintptr_t)ptr % sizeof(int)))) {
		*(unsigned char *)dst = *(const unsigned char *)ptr;
		dst++;
		ptr++;
		len--;
	}
	for (; len >= (int)sizeof(int); dst += sizeof(int),
	     ptr += sizeof(int), len -= sizeof(int))
		*(unsigned int *)dst = *(const unsigned int *)ptr;
	for (; len != 0; dst++, ptr++, len--)
		*(unsigned char *)dst = *(const unsigned char *)ptr;
	return retlen;
}

int metal_io_device_mem_block_write(struct metal_io_region *io,
					   unsigned long offset,
					   const void *restrict src,
					   memory_order order,
					   int len)
{
	void *ptr = metal_io_virt(io, offset);
	int retlen;

	retlen = len;
	while (len && (((uintptr_t)ptr % sizeof(int)) ||
		       ((uintptr_t)src % sizeof(int)))) {
		*(unsigned char *)ptr = *(const unsigned char *)src;
		ptr++;
		src++;
		len--;
	}
	for (; len >= (int)sizeof(int); ptr += sizeof(int),
	     src += sizeof(int), len -= sizeof(int))
		*(unsigned int *)ptr = *(const unsigned int *)src;
	for (; len != 0; ptr++, src++, len--)
		*(unsigned char *)ptr = *(const unsigned char *)src;
	atomic_thread_fence(order);
	return retlen;
}

void metal_io_device_mem_block_set(struct metal_io_region *io,
					  unsigned long offset,
					  unsigned char value,
					  memory_order order,
					  int len)
{
	void *ptr = metal_io_virt(io, offset);
	unsigned int cint = value;
	unsigned int i;

	for (i = 1; i < sizeof(int); i++)
		cint |= ((unsigned int)value << (8 * i));

	for (; len && ((uintptr_t)ptr % sizeof(int)); ptr++, len--)
		*(unsigned char *)ptr = (unsigned char) value;
	for (; len >= (int)sizeof(int); ptr += sizeof(int),
	     len -= sizeof(int))
		*(unsigned int *)ptr = cint;
	for (; len != 0; ptr++, len--)
		*(unsigned char *)ptr = (unsigned char) value;
	atomic_thread_fence(order);
}

struct metal_io_ops metal_weak metal_io_device_block_mem_ops = {
	.read = NULL,
	.write = NULL,
	.block_read = metal_io_device_mem_block_read,
	.block_write = metal_io_device_mem_block_write,
	.block_set = metal_io_device_mem_block_set,
	.close = NULL,
};
void metal_io_init(struct metal_io_region *io, void *virt,
	      const metal_phys_addr_t *physmap, size_t size,
	      unsigned page_shift, unsigned int mem_flags,
	      const struct metal_io_ops *ops)
{
	const struct metal_io_ops nops = {NULL, NULL, NULL, NULL, NULL, NULL};

	io->virt = virt;
	io->physmap = physmap;
	io->size = size;
	io->page_shift = page_shift;
	if (page_shift >= sizeof(io->page_mask) * CHAR_BIT)
		/* avoid overflow */
		io->page_mask = -1UL;
	else
		io->page_mask = (1UL << page_shift) - 1UL;
	io->mem_flags = mem_flags;
	io->ops = ops ? *ops : nops;
	metal_sys_io_mem_map(io);
}

int metal_io_block_read(struct metal_io_region *io, unsigned long offset,
	       void *restrict dst, int len)
{
	if (offset > io->size)
		return -ERANGE;
	if ((offset + len) > io->size)
		len = io->size - offset;
	if (io->ops.block_read)
		return io->ops.block_read(io, offset, dst,
					  memory_order_seq_cst, len);
	else
		/* block read is not supported on this I/O region */
		return -ERANGE;
}

int metal_io_block_write(struct metal_io_region *io, unsigned long offset,
	       const void *restrict src, int len)
{
	if (offset > io->size)
		return -ERANGE;
	if ((offset + len) > io->size)
		len = io->size - offset;
	if (io->ops.block_write)
		return io->ops.block_write(io, offset, src,
					   memory_order_seq_cst, len);
	else
		/* block write is not supported on this I/O region */
		return -ERANGE;
}

int metal_io_block_set(struct metal_io_region *io, unsigned long offset,
	       unsigned char value, int len)
{
	if (offset > io->size)
		return -ERANGE;
	if ((offset + len) > io->size)
		len = io->size - offset;
	if (io->ops.block_set) {
		io->ops.block_set(io, offset, value,
				  memory_order_seq_cst, len);
		return len;
	} else {
		/* block set is not supported on this I/O region */
		return -ERANGE;
	}
}

