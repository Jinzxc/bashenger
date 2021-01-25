# bashenger
## Michael Nath, Victor Siu, Jin Cheng Zhang PERIOD 10

## High-Level Description
In this project, we are imitating Messenger and making it more systemy.

## How to Run Program
To run the program, run `make` and subsequently `make run`. `make run` must be run for every user (i.e. different terminal windows) if they wish to join in.

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
3. Adding, listing, and removing a friend is one step
    - If user wishes to add, then he or she will be prompted to add the username of the person. This person must have already registered.
    - If user wishes to remove, then it's the same procedure as adding. 
4. Talking to a Friend
    - The user will be prompted to either start a new chatroom, or join an existing friend's room. (Note: as long as an user added a given room owner as a friend, he/she can join that owner's room)
    - If user wants to join another room, he/she must type the exact name of the room (which will be in format "[friend's username] room"). 
    - When user starts a new chatroom, he or she waits for other clients to join before they start typing. User can exit the chatroom by either clicking ctrl-c or by typing "exit()" in the chat. 
    - If user exits, the user will have to run `make run` again in order to move on to other chatrooms or create one. 
    - Return to step 1.

#BUGLIST
- On Michael's system (Mac OSX), sometimes there are segmentation faults when there are more than 2 clients in a room.
- On Victor's system (Linux), occasionally available chat rooms are not displayed.

