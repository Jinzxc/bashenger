# bashenger
## Michael Nath, Victor Siu, Jin Cheng Zhang PERIOD 10

## High-Level Description
In this project, we are imitating Messenger and making it more systemy.

## How to Run Program
To run the program, run `make` and simply run `make run`. This must be done for every user (i.e. different terminal windows) if they wish to join in.

## Required Libraries: 
    - stdlib.h
    - stdio.h
    - string.h
    - fcntl.h
    - unistd.h
    - ctype.h
    - signal.h
    - sys/stat.h
    - sys/types.h
    - sys/ipc.h
    - sys/shm.h
    - sys/stat.h
    - sys/wait.h
    - time.h
    - errno.h

## Basic Walkthrough
0.  Prompts user to type a username.
    - If username is not found then prompts users to sign up.
    - Otherwise user enters a password to log in.
    
1. Lists things for the users to do.
    1. Lists functionalities:
        - Add a friend.
        - List friends.
        - Message a friend.
        - Remove a friend.
2. User chooses to run one.
3. Adding, listing, and removing a friend is one step\
    - If user wishes to add, then he or she will be prompted to add the username of the person. This person must have already registered.
    - If user wishes to remove, then it's the same procedure as adding. 
4. Talking to a Friend
    - The user will be prompted to either start a new chatroom, or join an existing friend's room. (Note: as long as an user added a given room owner as a friend, he/she can join that owner's room)
    - When user starts a new chatroom, he or she waits for other clients to join before they start typing. User can exit the chatroom by either clicking ctrl-c or by typing "exit()" in the chat. 
    - If user exits, the user will have to run `./main` again in order to move on to other chatrooms or create one. 
    - Return to step 1.

#BUGLIST
- On Michael's system (Mac OSX), sometimes there are segmentation faults when there are more than 2 clients in a room.
- On Victor's system (Linux), occasionally available chat rooms are not displayed.


# DEVLOG
=======
1/11 [Start of project]\
1/12 - Victor  - Added functions for adding, deleting, and listing friends for a given friend.\
1/13 - Michael - Implemented handshaking between server and client(s). Implemented client-client FIFOs.\
1/14 - Michael - Implemented shared memory segments for each client-client pair.\
1/15 - Michael - Finished group chat functionality assuming all clients are connected. \
1/20 - Victor  - Start bugfix where you must start with all clients connected.\
1/20 - Jin     - Added the main.c file. Implemented account managment system.\
1/21 - Victor  - Finish logic where new clients handshake and is sent to other clients.\
1/22 - Jin     - Optimized account system. Fixed issue and tested friend.c.\
1/23 - Jin     - Updated main with friend.c implementations. Fixed memory issues with friend.c.\
1/23 - Victor  - Finish messaging between clients (some bugs with newly connected clients).\
1/23 - Jin     - Added util file to hold useful functions. Wrote functions in main to create and list chat rooms.\
1/23 - Michael - Freed all malloc'd segments, and fixed bug with shared mem segments not being properly removed.\
1/24 - Jin     - Wrote code for server/client to interact with main. Fixed chat room bugs in main.
