# Dictionary Sync

## Work in Progress

I wanted to synchronise the *vim* personal dictionary so that it would be the same on all the computers that I use.  I work on a server, a desktop computer and a laptop (that connects over a VPN).

Searching suggested that some use *Dropbox* to provide this service, but as I didn't wish to use it, I search for Open Source alternatives.

There were a number of solutions that required the use of a GUI environment to set them up, but as I wanted to use it on a server, where I don't have a GUI installed, these weren't possible.  I found one command line option, which I experimented with, but could get to work.

It occurred to me that *git* could be used for this purpose, but that I would need to provide the automation to make it work, which led to the development of  this application.

I wanted to:
1. Push the dictionary up to the central repository every time the local file changed.
2. Pull the dictionary down to the workstation every time the server version wasn't the same as the local version.

It has worked for me (so far), though it hasn't been tested more widely.  It doesn't have a lot of dependencies, the c code is just standard functionality, without additional libraries and the rest is dependant on *git*.  You need to have *git* installed as it uses the *git* executable, rather than using *git* libraries.
  
I currently have it running on *Ubuntu server* and two instances of *Kubuntu*.

I often work, both on my local computer and over *ssh* on the server and will be editing files on both and the dictionaries are kept synchronised.

It should work synchronising any set of files that are text based (else git won't be able to merge them) and that can be stored in a single folder structure.

---

I use the *.gitignore* file, to ensure that only the text version of the dictionary is synchronised, so I have mapped a function key in *vim* that will regenerate the binary version of the dictionary when I press it.

        :map  <F4> :mkspell! ~/.vim_spell/en.utf-8.add

The above line is added to my *.vimrc* file along with the configuration for the way I use the dictionary.

When the **F4** key is pressed, the command mapped to the key becomes the current command and pressing Enter executes it. This creates a binary dictionary file from the text file, that is up to date.

---

## Setting it up

Because of  *Linux* permissions you will need to use *sudo* or be logged on as *root* to run most of the commands I suggest.

I give examples of commands that you need to execute in a terminal. The examples will include using the *nano* editor which is considered simple to use. Though if you are setting this up to work with *vim* I suggest you use that.

I will set out the steps you need to go through to set up the synchronised folders, some of these steps only need to take place on one machine, the first one you set up. At the end I will detail the steps that you only need to carry on the additional machines.

---

1. You need to have a folder to put the files in you wish to have synchronised, you can use an existing folder or create a new one and call it anything you want.  I called mine *.vim_spell* and will use that in the examples.  You could use a folder that already has a *git* repository, providing it can be used similarly to  way the way this works.

Create or move into the folder you wish to use and go into it with a terminal.  If that folder isn't a *git* repository you will need to use the following commands to create one.  The second command names the branch (there will only be one) *main*

      git init
      git branch -m main

As this repository isn't intended to be of interest to anyone else it doesn't need anything like a *readme.md* file.

---

2. Because I was wanting to synchronise very specific files I added a *.gitignore* file within the git repository with the following content:

       *
       !en.utf-8.add
       !.gitignore
       en.utf-8.add.spl

The single * excludes everything, the next two lines include the specific files they name and the last line specifically excludes the binary version of the dictionary (yes, I know the * will also cause this to be ignored).

---

3. Next you need to set up your application, in my case the *vim* editor, to store the files you wish in the folder you created.

---

4. As you have a basic set of files in the repository run the following commands within your folder:

       git add .
       git commit -m 'Initial commit'

---

5. You now need to set up an online *git* repository, making it private if you can.  You need set it up so you can use *ssh* to push and pull from it and take a copy of the URL you would use for this.

Connect the local repository to the remote one using the following command with the URL you just copied/noted in place of the word *URL*. This command needs to be executed within the folder you created.
 

       git remote add origin *URL*


Push the content of the folder up to the online repository with the following command.


      git push -u origin main


You will then be able to push subsiqent changes using the command.


      git push
      git push -u origin main
   
--- 

6. To build/compile the program you need to have the *gcc* compiler installed.


         gcc -pthread -o dic_sync dic_sync.c


This will create the executable file called *dic_sync* in the folder were you run the command, which needs to contain the *dic_sync.c* source file.

---

7. You need to put the executable file in the folder */usr/local/bin/* with the command:

       sudo mv dic_sync /usr/local/bin/

---

8. program the permissions to be executed.

       sudo chmod 777 /usr/local/bin/dic_sync


---

9.  You need to set up a file in */etc* called *git_sync.conf* that tells the application were to find the *git* repository that contains the relevant files.

It needs to contains a line similar to this:

      /home/mike/.vim_spell/

To create/edit it use the following command:

      sudo nano /etc/git_sync.conf 

Add the path to the folder you created, that contains the local *git* repository as a single line, with a trailing slash (/) to the file and save and exit. 

---

10. Create the unit file for the program to be run in the background as a service

       sudo nano /etc/systemd/system/dic_sync.service

Paste the text below into it and save it:

     Description=Dictionary Sync 

     [Service] 
     User=mike 
     Type=simple 
     ExecStart=/usr/local/bin/dic_sync 
     Restart=on-failure 
     RestartSec=10 
     KillMode=process 

     [Install] 
     WantedBy=multi-user.target
---

11. Execute the following commands:

       sudo systemctl daemon-reload
       sudo systemctl start  dic_sync.service 
       sudo systemctl status dic_sync.service
       sudo systemctl enable dic_sync.service

It these run without error the program will be run and continue to run whilst the computer it is installed on, is running.

---

12. I can be disabled using the command:

      sudo systemctl disable example.service

---

This works for me and is here to be used and modified as required.  It probably isn't the best solution for this problem.  It is more a quick and dirty solution rather than something for wider distribution I'm open to suggestions on how it can be improved.


