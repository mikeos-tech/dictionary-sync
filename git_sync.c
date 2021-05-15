/*This is the sample program to notify us for the file creation and file deletion takes place in “/tmp” directory

// Mike O'Shea - 15/05/2021
sudo apt install libgit2-1.1
sudo apt install libgit2-dev

gcc git_sync.c -o bin/git_sync

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

const char *folder_path = "/home/mike/.vim_spell";

void watch_remote_changes(const char *path)
{
    char branch[120];
    char local[44];
    char remote[44];
    char buffer[1024];
    char url[1024];
    FILE *fp;

    char command_branch[40 + strlen(path)];
    strcpy(command_branch, "cd ");
    strcat(command_branch, path);
    strcat(command_branch, " && git rev-parse --abbrev-ref HEAD");
    fp = popen(command_branch, "r"); // open the command like a file
    if (fp != NULL) {
       while (fgets(branch, sizeof(branch), fp) != NULL) {
           printf("%s", path); // this is just for debugging
       }
    }
    pclose(fp);

    char command_local[30 + strlen(path)];
    strcpy(command_local, "cd ");
    strcat(command_local, path);
    strcat(command_local, " && git rev-parse HEAD");
    fp = popen(command_branch, "r"); // open the command like a file                                    
    if (fp != NULL) {
        while (fgets(local, sizeof(local), fp) != NULL) {
	    printf("%s", local); // this is just for debugging
	}
   }
   pclose(fp);

   char command_url[24 + strlen(path)];
   strcpy(command_url, "cd ");
   strcat(command_url, path);
   strcat(command_url, " && git ls-remote");
   fp = popen(command_url, "r"); // open the command like a file                                    
   if (fp != NULL) {
       int i = 0;
       while (fgets(buffer, sizeof(buffer), fp) != NULL) {
           printf("%s", buffer); // this is just for debugging
	   if(i == 0) {
               strcpy(url buffer[5]);
	       printf("\nURL: %s\n", url);
	   }
	   ++i
       }
   }
   pclose(fp);


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
      printf("\nHave read buffer\n");

      /*checking for error*/
      if (length < 0) {
          perror( "read" );
      }  

      printf("\nBefore Inner loop - length: %d - i: %d\n", length, i);
      /*actually read return the list of change events happens. Here, read the change event one by one and process it accordingly.*/
      while (i < length) {    
	     printf("\nstart of loop - lenght: %d - i: %d\n", length, i); 
          struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
          if (event->len) {
      	    if(event->mask & IN_MODIFY) {
		printf("\n\tFile changed: %s.\n", event->name);
		char command[(58 + strlen(path))];
		strcpy(command, "cd ");
		strcat(command, path);
		strcat(command, " && git add . && git commit -m 'updated' && git push &");
		system(command); 
		printf("\nExecuted git command\n");
      	    }
   	    i += EVENT_SIZE + event->len;
          }
	  printf("\nStill in inner loop - i: %d\n", i);
      }
      inotify_rm_watch(fd, wd); // remove directory from watch list
      close(fd); // close the INOTIFY
  }  while(1); // loop that doesn't end naturally
}

int main( )
{
  // Check the folder exists
  int retval = 0;
  DIR* dir = opendir(folder_path);
  if(dir) { // Directory exists. */
    closedir(dir);
    printf("The folder: '%s' exists.\n", folder_path);
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
