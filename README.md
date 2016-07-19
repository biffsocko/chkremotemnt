# chkremotemnt

WHAT IT DOES:
This program just reads /proc/mounts looking for NFS and CIFS mount points.  it then splits into two threads 
   1) thread1 does a df on the mounted filesystem
   2) thread2 is a timer (5 seconds).
   
if the timer reaches its limit, it exits the program (not allowing the hang that df would create on its own)

WHERE YO CAN GET IT
https://github.com/biffsocko/chkremotemnt.git

HOW TO COMPILE IT:

gcc -lpthread -o chkmount  chkmount.c
