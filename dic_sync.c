/*H*******************************************************************
  * FILENAME :        fmcompres.c
  *
  * DESCRIPTION :
  *       File compression and decompression routines.
  *
  * PUBLIC FUNCTIONS :
  *
  * NOTES :
  *       These functions are a part of the FM suite;
  *       See IMS FM0121 for detailed description.
  *
  *       Copyright A.N.Other Co. 1990, 1995.  All rights reserved.
  *
  * AUTHOR :    Arthur Other        START DATE :    16 Jan 99
  *
  * CHANGES :
  *
  * REF NO  VERSION DATE    WHO     DETAIL
  * F21/33  A.03.04 22Jan99 JR      Function CalcHuffman corrected
  *
  * H*/

#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <unistd.h>

#define EVENT_SIZE  (sizeof(struct inotify_event))
#define EVENT_BUF_LEN     (1024 * (EVENT_SIZE + 16))

void clear_return(char *string)
{
	if(string[(strlen(string) - 1)] == '\n'){     // git seems to append a newline character to the end of this
		string[(strlen(string) - 1)] = 0;
	}
}

// This function will run concurrently.
void* local_change(void *ptr)
{
	char *path;

	path = ptr;
	while(1){ // start of infinite while loop
		int length = 0;
		int i = 0;
		int fd;
		int wd;
		char buffer[EVENT_BUF_LEN];

		// creating an INOTIFY instance
		fd = inotify_init();
		
		// check for errors
		if(fd < 0){
			perror( "inotify_init" );
		}

		// Adding the folder to be watch to the watch list.
		wd = inotify_add_watch( fd, path, IN_CREATE | IN_DELETE | IN_MODIFY );
		// Read to until a change event happens in the directory, it blocks until something happens.
		length = read(fd, buffer, EVENT_BUF_LEN);
		
		/*checking for error*/
		if(length < 0){
			perror( "read" );
		}
		/*actually read return the list of change events happens. Here, read the change event one by one and process it accordingly.*/
		while(i < length){
	 		struct inotify_event *event = ( struct inotify_event * )&buffer[ i ];
			if(event->len){
				if(event->mask & IN_MODIFY){
					char command[(70 + strlen(path))];
					strcpy(command, "cd ");
					strcat(command, path);
					strcat(command, " && git add en.utf-8.add && git commit -m 'updated' && git push"/* &> /dev/null"*/);
					system(command);
	//                                printf("\n\n\tLocal: %s\n", command);

				}
				i += EVENT_SIZE + event->len;
			}  										  	     }													inotify_rm_watch(fd, wd);               // remove directory from watch list
	        close(fd);                              // close the INOTIFY
	        i = 0; // reset i back to zero
	}  // end of infinite while loop
}

void* remote_change(void *ptr)
{
	FILE *fp;
	char *path;
	char branch[120];
	char buffer[2048];
	char local[1024];
	char remote[1024];
	char url[1024];

	path = ptr;
	while(1){ // Start of infinite while loop
		// Read the branch that the current local is based on
		char command_branch[40 + strlen(path)];
		strcpy(command_branch, "cd ");
		strcat(command_branch, path);
		strcat(command_branch, " && git rev-parse --abbrev-ref HEAD");
		fp = popen(command_branch, "r");     // open the command like a file
		if(fp != NULL){
			fgets(branch, sizeof(branch), fp);
		}
		pclose(fp);

		// Read the local head
		char command_local[30 + strlen(path)];
		strcpy(command_local, "cd ");
		strcat(command_local, path);
		strcat(command_local, " && git rev-parse HEAD");
		fp = popen(command_local, "r");     // open the command like a file
		if(fp != NULL){
			fgets(local, sizeof(local), fp);
			clear_return(local);
		}
		pclose(fp);

		// get the URL for the remote repository
		char command_url[44 + strlen(path)];
		strcpy(command_url, "cd ");
		strcat(command_url, path);
		strcat(command_url, " && git config --get remote.origin.url");
		fp = popen(command_url, "r");     // open the command like a file
		if(fp != NULL){
			fgets(url, sizeof(url), fp);
			clear_return(url);
		}
		pclose(fp);

		// read the remote head that relates to the current local head and trim extraneous characters from end
		char command_remote[4048 + strlen(path)];
		strcpy(command_remote, "cd ");
		strcat(command_remote, path);
		strcat(command_remote, " && git ls-remote ");
		strcat(command_remote, url);
		strcat(command_remote, " -h ");
		strcat(command_remote, branch);
		fp = popen(command_remote, "r");     // open the command like a file
		if(fp != NULL){
			fgets(buffer, sizeof(buffer), fp);
			int i = 0;
			while((buffer[i] != '\t') && (i < strlen(buffer))){ // The character between the two
				// elements is a tab
				remote[i] = buffer[i];
				++i;
			}
			remote[i] = 0;
		}
		pclose(fp);

		if(strcmp(local, remote) != 0){
			char command[20 + strlen(path)];
			strcpy(command, "cd ");
			strcat(command, path);
			strcat(command, " && git pull");
			system(command);
  //                      printf("\n\n\tRemote: %s\n", command);
		}
		sleep(3);
//		printf("Remote: %d\n", ++i);
	} // end of infinite while loop
}

int main()
{
	int retval = 0;
	char config_path[20] = "/etc/git_sync.conf";
	char folder_path[1024];
	FILE *config;
	pthread_t t1;
	pthread_t t2;

	config = fopen(config_path, "r");
	if(config){
		if(fgets(folder_path, 1024, config) == NULL){
			printf("Configuration file needs to point to the git repository you want to keep synchronised!\n");
			return(-1);
		}
		clear_return(folder_path);
		fclose(config);
	} else {
		printf("Configuration file not found: %s\nThis file needs to point to the git repository you want to keep synchronised!\n", config_path);
		return(-1);
	}

	DIR* dir = opendir(folder_path);

	if(dir){     // Check the folder exists
		closedir(dir);
		char gitdir[36];
		strcpy(gitdir, folder_path);
		strcat(gitdir, ".git/");
		DIR* dir = opendir(gitdir);
		if(!dir){   // Check the folder has a git repository
			printf("The folder %s, does not contain a git repository!\n", folder_path);
			return(-3);
		}
		closedir(dir);
		// Start the two threads
		int iret1 = pthread_create(&t1, NULL, local_change, folder_path);
		int iret2 = pthread_create(&t2, NULL, remote_change, folder_path);
	} else if(ENOENT == errno){ // if folder doesn't exist
		printf("The folder: '%s' does not exist.\n", folder_path);
		retval = -1;
	} else {
		printf("The folder: '%s' can't be accessed.\n", folder_path);
		retval = -2;  // Can't access folder for some other reason
	}

	// Code to keep the program running
//	int i = 0;
	while(1){ // Infinite loop
		sleep(10);
	//	printf("Main: %d\n", ++i);
	} // Infinite loop end
	exit(retval);                              //never reached.
}
