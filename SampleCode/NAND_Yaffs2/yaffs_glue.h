

void cmd_yaffs_devconfig(char *_mp, int flash_dev, int start_block, int end_block);
void cmd_yaffs_dev_ls(void);
void cmd_yaffs_mount(char *mp);
void cmd_yaffs_umount(char *mp);
void cmd_yaffs_write_file(char *yaffsName, char bval, int sizeOfFile);
void cmd_yaffs_read_file(char *fn);
void cmd_yaffs_mread_file(char *fn, char *addr);
void cmd_yaffs_mwrite_file(char *fn, char *addr, int size);
void cmd_yaffs_ls(const char *mountpt, int longlist);
void cmd_yaffs_mkdir(const char *dir);
void cmd_yaffs_rmdir(const char *dir);
void cmd_yaffs_rm(const char *path);
void cmd_yaffs_mv(const char *oldPath, const char *newPath);

