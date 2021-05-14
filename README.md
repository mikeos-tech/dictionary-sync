# git_sync
Detect changes to files that are part of the git repository found in the path and push and pull to keep them synchronised.
The idea is to keep my vim dictionary files synchronised for all instances.

  * I want to be able to read the local git repository and get the list of files and only act in response to activity relating to those files and really only to file change events.

  * If a file changes I want to carry out an add, commit and push to the online repository

  * I want to monitor the on repository and if the file changes there (changed on another machine) and carry out a pull to keep the different file folders synchronised.

Command line to compile:
    gcc git_sync.c -o bin/git_sync

