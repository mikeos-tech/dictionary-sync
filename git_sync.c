/*This is the sample program to notify us for the file creation and file deletion takes place in “/tmp” directory

// Mike O'Shea - 15/05/2021
sudo apt install libgit2-1.1
sudo apt install libgit2-dev


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

int main( )
{
  int length, i = 0;
  int fd;
  int wd;
  char buffer[EVENT_BUF_LEN];

  DIR* dir = opendir(folder_path);
  if (dir) { // Directory exists. */
    closedir(dir);
    printf("The folder: '%s' exists.\n", folder_path);
  } else if (ENOENT == errno) {
    printf("The folder: '%s' does not exist.\n", folder_path);
	return -1;
    /* Directory does not exist. */
  } else {
    printf("The folder: '%s' can't be accessed.\n", folder_path);
	return -2; 
    /* opendir() failed for some other reason. */
  }

  /*creating the INOTIFY instance*/
  fd = inotify_init();

  /*checking for error*/
  if ( fd < 0 ) {
    perror( "inotify_init" );
  }
  do {
      /*adding the “/tmp” directory into watch list. Here, the suggestion is to validate the existence of the directory before adding into monitoring list.*/
      wd = inotify_add_watch( fd, folder_path, IN_CREATE | IN_DELETE | IN_MODIFY );

      /*read to determine the event change happens on “/tmp” directory. Actually this read blocks until the change event occurs*/ 

      length = read( fd, buffer, EVENT_BUF_LEN ); 

      /*checking for error*/
      if ( length < 0 ) {
          perror( "read" );
      }  

      /*actually read return the list of change events happens. Here, read the change event one by one and process it accordingly.*/
      while ( i < length ) {     
          struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
          if ( event->len ) {
      	    if(event->mask & IN_MODIFY) {
		printf("\n\tFile changed: %s.\n", event->name);
		char command[512];
		strcpy(command, "cd ");
		strcat(command, folder_path);
		strcat(command, " && git add . && git commit -m 'updated' && git push ");

		system(command); 
      	    }
   	    i += EVENT_SIZE + event->len;
          }
          inotify_rm_watch( fd, wd ); // remove directory from watch list

          close( fd ); // close the INOTIFY
      } 
  }  while(1); // loop continues

return(0);
}
