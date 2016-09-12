# .bash_profile

export PATH=\
/bin:\
/sbin:\
/usr/bin:\
/usr/sbin:\
/usr/bin/X11:\
/mnt/mmc/bin:\
/usr/local/bin

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/mnt/mmc/lib

umask 022

if [ -f ~/.bashrc ]; then
    source ~/.bashrc
fi
