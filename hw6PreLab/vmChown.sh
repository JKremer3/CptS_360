echo "this mounts the disk for mac parallels relative to the dir"
sudo mount -a
echo $'\nChange to username:username to make it own properly for your machine\n'
sudo chown -R jeff:jeff mydisk
sudo chmod -R 755 mydisk
echo "done"
