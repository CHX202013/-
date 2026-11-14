#ifndef MW_FILE_WRAPPER_H
#define MW_FILE_WRAPPER_H

#include <linux/file.h>
#include <linux/fs.h>

int mw_install_file_wrapper(int fd);
void mw_file_wrapper_init(void);

#endif // MW_FILE_WRAPPER_H
