# bashenger
## Michael Nath, Victor Siu, Jin Cheng Zhang PERIOD 10

## High-Level Description
In this project, we are imitating Messenger and making it more systemy.

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
    Messaging is continuous until [exit].
    
2. Return to step 1

## Program Usage
- Like Messenger, there will be a list of people a user can talk to. 
- Users will be able to send to and receive messages from other users. 
- Users can look up available users.
- Users can add "friends" to talk to.
- Users can remove "friends." [extra]

## Technical design
### Topic Implementations
- Use pipes to transfer messages between users.
- Track past messages and users by writing them into files.
- Finding information about files (check time stamps).
- Use forking and processes for interacting with cilents.

### Delegations
Victor:  Making friend list (adding, searching, deleting). \
Michael: Sending and receiving messages. \
Jin:     Designing chat window, message history/metadata. 

### Data Structure
Array: buffer for data (friend names & stuff).\
Queue: buffer for the messages.\
Tree (pre-fix):  compressing messages [extra].\
Structs: File manipulation and tree.

### Algorithms
Message Compression Algorithm [extra]. \
Message Decompression Algorithm [extra].

## Timeline
1. Give a name, get a friend (pre-adding friend)  [1/11 - 1/14]
2. Implement friend list (adding/removing)        [1/11 - 1/14]
3. Implement basic chatting                       [1/14 - 1/16]
4. Group chat                                     [1/16 - 1/18]
5. Bug fixing/implement extra features            [1/18 - 1/25]

Will be expected to be completed in 14 years 2 months 3 days at 3:35 PM EST

## Extra Stuff (to add?)
- Talking with mutliple people at once.
- Friend requests (accept/deny).
- Block users from messaging (refuse messages).
- Extra encryption/decryption.
- Notifications.


# DEVLOG
1/12 - Victor - Added functions for adding, deleting, and listing friends for a given friend
1/13 - Michael - Implemented handshaking between server and client(s). Implemented client-client FIFOs.
1/14 - Michael - Implemented shared memory segments for each client-client pair.
1/15 - Michael - Finished group chat functionality assuming all clients are connected. 
1/20 - Victor - Start bugfix where you must start with all clients connected.
1/21 - Victor - Finish logic where new clients handshake and is sent to other clients.
