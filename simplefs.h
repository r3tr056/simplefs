
#ifndef SIMPLEFS_H
#define SIMPLEFS_H

#define SIMPLEFS_MAGIC 0xDEAD

#define SIMPLEFS_SB_BLOCK_NR 0

#define SIMPLEFS_BLOCK_SIZE (1 << 12)
#define SIMPLEFS_MAX_EXTENTS ((SIMPLEFS_BLOCK_SIZE - sizeof(uint32_t)) / sizeof(struct simplefs_extent))
#define SIMPLEFS_MAX_BLOCKS_PER_EXTENT 8
#define SIMPLEFS_MAX_SIZES_PER_EXTENT (SIMPLEFS_MAX_BLOCKS_PER_EXTENT * SIMPLEFS_BLOCK_SIZE)
#define SIMPLEFS_MAX_FILESIZE ((uint64_t) SIMPLEFS_MAX_BLOCKS_PER_EXTENT * SIMPLEFS_BLOCK_SIZE * SIMPLEFS_MAX_EXTENTS)
#define SIMPLEFS_FILENAME_LEN 255
#define SIMPLEFS_FILES_PER_BLOCK (SIMPLEFS_BLOCK_SIZE / sizeof(struct simplefs_file))
#define SIMPLEFS_FILES_PER_EXT (SIMPLEFS_FILES_PER_BLOCK * SIMPLEFS_MAX_BLOCKS_PER_EXTENT)
#define SIMPLEFS_MAX_SUBFILES (SIMPLEFS_FILES_PER_EXT * SIMPLEFS_MAX_EXTENTS)

/*
 * SIMPLE FS Partition Layout
 * __________________
 * |   superblock   |
 * |----------------|
 * |  inode store   |
 * |----------------|
 * |  ifree bitmap  |
 * |----------------|
 * |  bfree bitmap  |
 * |----------------|
 * |  data blocks   |
 * |----------------|
 *
 */

struct simplefs_inode {
  uint32_t i_mode;
  uint32_t i_uid;
  uint32_t i_gid;
  uint32_t i_size;
  uint32_t i_ctime;
  uint32_t i_atime;
  uint32_t i_mtime;
  uint32_t i_blocks;
  uint32_t i_nlink;
  uint32_t i_nlink;
  uint32_t ei_block;
  char i_data[32];
};

#define SIMPLEFS_INODES_PER_BLOCK (SIMPLEFS_BLOCK_SIZE / sizeof(struct simplefs_inode))

struct simplefs_sb_info {
  uint32_t magic; // maigc number

  uint32_t nr_blocks; // total no of block (including the sb & inodes)
  uint32_t nr_inodes; // total no of inodes

  uint32_t nr_istore_blocks; // number of inode store blocks
  uint32_t nr_ifree_blocks; // number of inode free bitmap blocks
  uint32_t nr_bfree_blocks; // number of block free bitmap blocks

  uint32_t nr_free_inodes; // number of free inodes
  uint32_t nr_free_blocks; // number of free blocks
  
#ifdef __KERNEL__
    unsigned long *ifree_bitmap;
    unsigned long *bfree_bitmap;
#endif

}

#ifdef __KERNEL__
#include <linux/version.h>

#define SIMPLEFS_AT_LEAST(major, minor, rev) LINUX_VERSION_CODE >= KERNEL_VERSION(major, minor, rev)
#define SIMPLEFS_LESS_EQUAL(major, minor, rev) LINUX_VERSION_CODE <= KERNEL_VERSION(major, minor, rev)

struct simplefs_inode_info {
  uint32_t ei_block;
  char i_data[32];
  struct inode vfs_inode;
};

struct simplefs_extent {
  uint32_t ee_block;
  uint32_t ee_len;
  uint32_t ee_start;
};

struct simplefs_file_ei_block {
  uint32_t nr_files;
  struct simplefs_extent extents[SIMPLEFS_MAX_EXTENTS];
};

struct simplefs_file {
  uint32_t inode;
  char filename[SIMPLEFS_FILENAME_LEN];
};

struct simplefs_dir_block {
  struct simplefs_file files[SIMPLEFS_FILES_PER_BLOCK];
};

/* superblock methods */
int simplefs_fill_super(struct super_block *sb, void *ata, int silent);
void simplefs_kill_sb(struct super_block *sb);

// inode methods
int simplefs_init_inode_cache(void);
void simplefs_destroy_inode_cache(void);
struct inode *simplefs_iget(struct super_block *sb, unsigned long ino);

// dentry methods
struct dentry *simplefs_mount(struct file_system_type *fs_type, int flags, const char *dev_name, void *data);

extern const struct file_operations simplefs_file_ops;
extern const struct file_operations simplefs_dir_ops;
extern const struct address_space_operations simplefs_aops;

extern uint32_t simplefs_ext_search(struct simplefs_file_ei_block *index, uint32_t iblock);

#define SIMPLEFS_SB(sb) (sb->s_fs_info)
#define SIMPLEFS_INODE(inode) (container_of(inode, struct simplefs_inode_info, vfs_inode))

#endif

#endif 

