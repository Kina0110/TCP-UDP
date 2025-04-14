# README 
Created a network echo server (echo_s) that supports both TCP and UDP connections, handles multiple clients using processes, and sends client messages to the log server.
Developed an echo client (echo_c) to test the echo server and communicate with it using TCP or UDP, depending on the input.
Implemented a log server (log_s) to record client messages, timestamps, and IP addresses in a file (echo.log) in the required format, creating logging and operation.

I made a directory called Assignment7 on my home directory and executed all my files within it  
Any process will terminate with Ctrl+C if an exiting process is not explicitly mentioned 

Compilation: 
gcc -o log_s log_s.c
gcc -o echo_c echo_c.c
gcc -o echo_s echo_s.c

Then, execution [on separate tabs, using cs2.utdallas.edu] 
1. Run  ./log_s
2. Run  ./echo_s 1234
3. Run  ./echo_c 10.176.92.16 1234 UDP    [Put in any messages] 
4. Run  ./echo_c 10.176.92.16 1234 TCP    [Put in any messages] 
5. Run   cat echo.log                     [To see the logging] 

