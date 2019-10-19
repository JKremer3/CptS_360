dd if=/dev/zero of=mydisk bs=1024 count=1440
mkfs -b 1024 mydisk 1440
mount mydisk /mnt
umount /mnt
