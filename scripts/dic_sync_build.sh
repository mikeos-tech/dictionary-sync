# dic_sync_build.sh 
# creator: Mike O'Shea 
# Updated: 21/05/2021 
# Compiles the source code to create the executable. 
# Adds it to the git repository and copies the executable to the
# Home folder on my workstation.

# GNU General Public License, version 3

#!/bin/bash

gcc -pthread -o ~/projects/git_sync_project/bin/dic_sync ~/projects/git_sync_project/dic_sync.c
sudo chmod 777 ~/projects/git_sync_project/bin/dic_sync
cd ~/projects/git_sync_project/
git add dic_sync.c
scp ~/projects/git_sync_project/bin/dic_sync mike@192.168.2.26:~
sudo cp ~/projects/git_sync_project/bin/dic_sync /usr/local/bin
sudo chmod 777 /usr/local/bin/dic_sync
sudo systemctl edit /usr/local/bin/dic_sync
sudo systemctl daemon-reload
sudo systemctl enable dic_sync
