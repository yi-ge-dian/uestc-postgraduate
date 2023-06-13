#pragma once

#include "charDevDriver.h"

int DriverOpen(struct inode *pInodeStruct, struct file *pFileStruct);
int DriverRelease(struct inode *pInodeStruct, struct file *pFileStruct);
ssize_t DriverRead(struct file *pFileStruct, char __user *pBuffer, size_t count, loff_t *pOffset);
ssize_t DriverWrite(struct file *pFileStruct, const char __user *pBuffer, size_t count, loff_t *pOffset);
