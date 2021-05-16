/*This is the sample program to notify us for the file creation and file deletion takes place in “/tmp” directory

// Mike O'Shea - 15/05/2021
sudo apt install libgit2-1.1
sudo apt install libgit2-dev

gcc git_sync.c -o bin/git_sync

cd .. && gcc git_sync.c -o bin/git_sync && cd bin/ && ./git_sync

I got this from:
https://www.thegeekstuff.com/2010/04/inotify-c-program-example/
https://linuxhint.com/inotify_api_c_languageysi
*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
// #include <sys/inotify.h>
#include <linux/inotify.h>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

void clear_return(char *string)
{
      if(string[(strlen(string) - 1)] == '\n') { // git seems to append a newline character to the end of this
	  string[(strlen(string) - 1)] = 0;
      }
}

void watch_remote_changes(const char *path)
{
    char branch[120];
    char local[1024];
    char remote[44];
    char buffer[2048];
    char url[1024];
    FILE *fp;

    // Read the branch for the current local
    char command_branch[40 + strlen(path)];
    strcpy(command_branch, "cd ");
    strcat(command_branch, path);
    strcat(command_branch, " && git rev-parse --abbrev-ref HEAD");
    fp = popen(command_branch, "r"); // open the command like a file
    if (fp != NULL) {
       fgets(branch, sizeof(branch), fp);
    }
    pclose(fp);

    // Read the local head
    char command_local[30 + strlen(path)];
    strcpy(command_local, "cd ");
    strcat(command_local, path);
    strcat(command_local, " && git rev-parse HEAD");
    fp = popen(command_local, "r"); // open the command like a file                                    
    if (fp != NULL) {
        fgets(local, sizeof(local), fp);
	clear_return(local);
   }
   pclose(fp);

   // get the URL for the remote repository 
   char command_url[44 + strlen(path)];
   strcpy(command_url, "cd ");
   strcat(command_url, path);
   strcat(command_url, " && git config --get remote.origin.url");
   fp = popen(command_url, "r"); // open the command like a file                                    
   if (fp != NULL) {
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
   fp = popen(command_remote, "r"); // open the command like a file                                    
   if (fp != NULL) {
       fgets(buffer, sizeof(buffer), fp);
       int i = 0;
       while((buffer[i] != '\t') && (i < strlen(buffer))) { // The character between the two elements is a tab
	   remote[i] = buffer[i];
	   ++i;
       }
       remote[i] = 0;
   }
   pclose(fp);

//   printf("\nlocal:  %s\nRemote: %s\n", local, remote);

   if(strcmp(local, remote) != 0) {
   	char command[34 + strlen(path)];
   	strcpy(command, "cd ");
   	strcat(command, path);
   	strcat(command, " && git config pull.rebase false");
	system(command);
   }
}

void watch_local_changes(const char *path)
{
  do {
      int length, i = 0;
      int fd;
      int wd;
      char buffer[EVENT_BUF_LEN];

      printf("\nStarted infinite loop\n");
      // creating an INOTIFY instance
      fd = inotify_init();

      // check for errors
      if ( fd < 0 ) {
          perror( "inotify_init" );
      }
      // Adding the folder to be watch to the watch list. 
      wd = inotify_add_watch( fd, path, IN_CREATE | IN_DELETE | IN_MODIFY );

      // Read to until a change event happens in the directory, it blocks until something happens. 
      length = read(fd, buffer, EVENT_BUF_LEN); 

      /*checking for error*/
      if (length < 0) {
          perror( "read" );
      }  

      /*actually read return the list of change events happens. Here, read the change event one by one and process it accordingly.*/
      while (i < length) {    
          struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
          if (event->len) {
      	    if(event->mask & IN_MODIFY) {
//		printf("\n\tFile changed: %s.\n", event->name);
		char command[(58 + strlen(path))];
		strcpy(command, "cd ");
		strcat(command, path);
		strcat(command, " && git add . && git commit -m 'updated' && git push &");
		system(command); 
//		printf("\nExecuted git command\n");
      	    }
   	    i += EVENT_SIZE + event->len;
          }
//	  printf("\nStill in inner loop - i: %d\n", i);
      }
      inotify_rm_watch(fd, wd); // remove directory from watch list
      close(fd); // close the INOTIFY
  }  while(1); // loop that doesn't end naturally
}

int main( )
{
  // Check the folder exists
  int retval = 0;
  char config_path[20] = "/etc/git_sync.conf";
  char folder_path[1024];
  FILE *config;

  config = fopen(config_path, "r");
  if(config) {
      if(fgets(folder_path, 1024, config) == NULL) {
          printf("Configuration file needs to point to the git repository you want to keep synchronised!\n");
          return -1;
      }
      clear_return(folder_path);
      fclose(config); 
  } else {
      printf("Configuration file not found: %s\nThis file needs to point to the git repository you want to keep synchronised!\n", config_path);
      return(-1);
  }

  DIR* dir = opendir(folder_path);
  if(dir) { // Directory exists. */
    closedir(dir);
//    printf("The folder: '%s' exists.\n", folder_path);
    watch_remote_changes(folder_path);
    watch_local_changes(folder_path);
    retval = 0;
  } else if (ENOENT == errno) {
    printf("The folder: '%s' does not exist.\n", folder_path);
    retval = -1;
  } else {
    printf("The folder: '%s' can't be accessed.\n", folder_path);
    retval = -2; // Can't access folder for some other reason
  }
  return retval;
}
