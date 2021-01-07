# bashenger
## Michael Nath, Victor Siu, Jin Cheng Zhang PERIOD 10

## High-Level Description
In this project, we are imitating Messenger and making it more systemy.

## Basic Walkthrough
0. (First time) running the program -> creates a config file.
    1. Prompts user to add a name.
    2. Adds name to config file.
    
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
- Users can remove "friends."

## Technical design
### Topic Implementations
- Use pipes to transfer messages between users.
- Track past messages and users by writing them into files.
- Finding information about files (check time stamps).
- Use forking and processes for interacting with cilents.

### Delegations
Victor:  learn networking\
Michael: learn networking\
Jin:     learn networking

### Data Structure
Array: buffer for data (friend names & stuff)\
Queue: buffer for the messages\
Tree (pre-fix):  compressing messages [extra]

### Algorithms
Message Compression Algorithm [extra]
Message Decompression Algorithm [extra]

## Timeline
Will be expected to be completed in 14 years 2 months 3 days at 3:35 PM EST

## Extra Stuff (to add?)
- Talking with mutliple people at once.
- Friend requests (accept/deny).
- Block users from messaging (refuse messages).
- Extra encryption/decryption
