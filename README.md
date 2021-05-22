# Dictionary Sync

## Work in Progress

I wanted to synchronise the *vim* personal dictionary so that it would be the same on all the computers that I use.  I work on a server, a desktop computer and a laptop (that connects over a VPN).

Searching suggested that some use *Dropbox* to provide this service, but as I didn't wish to use it, I search for Open Source alternatives.

There were a number of solutions that required the use of a GUI environment to set them up, but as I wanted to use it on a server, where I don't have a GUI installed, these weren't possible.  I found one command line option, which I experimented with, but could get to work.

It occurred to me that *git* could be used for this purpose, but that I would need to provide the automation to make it work, which led to the development of  this application.

I wanted to:
1. Push the dictionary up to the central repository every time the local file changed.
2. Pull the dictionary down to the workstation every time the server version changed.

It has worked for me (so far), though it hasn't been tested elsewhere.  It doesn't have a lot of dependencies, the c code is just standard functionality, without additional libraries and the rest is dependant on *git*.  You need to have *git* installed as it uses the *git* executable, rather than using *git* libraries.
  
I currently have it running on *Ubuntu server* and two instances of *Kubuntu*.

I often work both on my local computer and over *ssh* on the server and will be editing files on both and the dictionaries are kept synchronised.

It should work synchronising any set of files that are text based (else git won't be able to merge them) and that can be stored in a single folder structure.

---

I use the *.gitignore* file, to ensure that only the text version of the dictionary is synchronised, so I have mapped a function key in *vim* that will regenerate the binary version of the dictionary when I press it.

     :map  <F4> :mkspell! ~/.vim_spell/en.utf-8.add

The above line is added to my *.vimrc* file along with the configuration for the way I use the dictionary.

---

You need to create a folder, you can call it what you want, but I called mine *.vim_spell* and initialise it as a *git* repository. You then need to link it with an on-line *git* repository and configure it, so you use a  *ssh* key for synchronisation.  This program  won't work if you manually have to enter a username and password!

---

You need to set up your application, in my case the *vim* editor, to store the files you wish in the folder you created.

---

Because I was wanting to synchronise very specific files I added a *.gitignore* file within the git repository with the following content:

     *
     !en.utf-8.add
     !.gitignore
     en.utf-8.add.spl

The single * excludes everything, the next two lines include the specific files they name and the last line specifically excludes the binary version of the dictionary (yes, I know the * will also cause this to be ignored).

---

Because of file permissions you will need to use *sudo* or be logged on as *root* to carry out most of these steps.

I will give example command lines, using the nano editor which is considered simple to use.

---

1. To build/compile the program you need to have the *gcc* compiler installed.

     gcc -pthread -o dic_sync dic_sync.c

 - This will create the executable file *dic_sync* in the current folder.

---

2.  You need to set up a file in */etc* called *git_sync.conf* that tells the application were to find the git repository that contains the relevant files.

It needs to contains a line similar to this:

      /home/mike/.vim_spell/

To create/edit it use the following command:

      sudo nano /etc/git_sync.conf 

The path in the file can point any were, providing it works as far as any application that work on it are concerned and the permissions allow what you need. It needs to have the trailing slash (/) for my program to work.

---

3. You need to put the executable file in the folder */usr/local/bin/* with the commands:

     sudo mv dic_sync /usr/local/bin/

 - Give the program the permissions to be executed.

     sudo chmod 777 /usr/local/bin/dic_sync

---

4. Create the unit file for the program to be run in the background as a service

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

5. Execute the following commands:

      sudo systemctl daemon-reload
      sudo systemctl start  dic_sync.service 
      sudo systemctl status dic_sync.service
      sudo systemctl enable dic_sync.service

It these run without error the program will be run and continue to run whilst the computer it is installed on, is running.

---

6.I can be disabled using the command:

      sudo systemctl disable example.service

This works for me and is here to be used and modified as required.  It probably isn't the best solution for this problem.  It is more a quick and dirty solution rather than something for wider distribution I'm open to suggestions on how it can be improved.


