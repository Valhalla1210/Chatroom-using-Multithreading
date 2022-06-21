# Chatroom Using Mutlithreading
I have created a chatroom in C language where multiple people can chat with each other. People can enter the chatroom using their names and can exit at any point during the chat.
## Procedure to run:
**Note**: This project was created in linux environment because of libraries like sys/socket.h that work only with linux. If you're working on windows, you can install WSL extension for VS code or dual boot windows with linux.\
\
Make sure your system has C/C++ libraries installed that come up with most compilers.
### Instructions:
1) Clone my respository to your system.
2) In the terminal open the directory where repo has been downloaded.
3) For compiling server.c, enter this command in the terminal.
```
gcc server.c -pthread -o server
```
4) To execute server.c , specify the port no. along with the execution command as
```
./server 8080
```
5) Open another terminal window and similarly compile and execute the client file client.c as
 ```
 gcc client.c -pthread -o client
 ./client 8080
 ```
 6) Enter your name in the terminal, now you have entered into the chat room.
 7) For more people to enter into the chatroom run the above command in another terminal window.
 ## Working of Project:
 
 ### Demo Video
 [Video]() describing the working of this project.
 
 ## Learning Outcome:
 1) Learnt the concept of multithreading and its use to reduce waiting time for client requests.
 2) Learnt socket programming by implementing a multithreaded chatroom
 3) Learnt basic linux commands as I had to dual boot linux with windows in my PC.\
 ## References
 1) GeeksforGeeks for [socket programming](https://www.geeksforgeeks.org/socket-programming-cc/?ref=lbp),[mutex lock](https://www.geeksforgeeks.org/mutex-lock-for-linux-thread-synchronization/), threads and server.
 2) [Documentation of C libraries](https://publications.opengroup.org/) used.
 3) [Guide](https://medium.com/linuxforeveryone/how-to-install-ubuntu-20-04-and-dual-boot-alongside-windows-10-323a85271a73) for installation of linux.
