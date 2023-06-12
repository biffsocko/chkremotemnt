/*****************************************************************
 * TR Murphy
 * chkmount.c
 *
 * checks for hanging nfs or cifs mountpoints
 *
 * complie:
 * gcc -lpthread -o chkmount  chkmount.c
 *****************************************************************/
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <mntent.h>


/****************************************************************************
 * DEBUG MODE:                                                              *
 * To turn DBUG OFF, change the #define DEBUG statement to 0.  to turn      *
 * DEBUG ON, change #define DEBUG to 1                                      *
 ****************************************************************************/
#define DEBUG          0
#define MAX            1024

/****************************************************************************
 * a few global variables - yuck - but they are used inside the threads
 ****************************************************************************/
char mountpt[MAX];       /* mount point to check */
char mountfs[MAX];       /* remote host:filesystem */
pthread_t check_mount;   /* thread for checking mount point */
pthread_t time_out;      /* thread for keeping time */



/****************************************************************************
 * function declarations (for the threads)
 ****************************************************************************/
void *checkmount(void *x);
void *timeout();


/****************************************************************************
 * MAIN
 ****************************************************************************/
int main (int argc, char *argv[]){
    struct mntent *ent;      /* structure for getmntent function */
    FILE *aFile;             /* File for getmntent function */



    /*
     * check for usage 
     */
    if(argc != 1){
       printf("usage: %s \n", argv[0]);
       exit(1);
    }

    /* 
     * add data to the mntext structure
     */
    aFile = setmntent("/proc/mounts", "r");
    if (aFile == NULL) {
        perror("setmntent");
        exit(1);
    }

    
    /* 
     * loop through the nfs and cifs filesystems
     */

    while (NULL != (ent = getmntent(aFile))) {
        if((strcmp(ent->mnt_type, "nfs") == 0) || (strcmp(ent->mnt_type, "cifs") == 0)){
            sprintf(mountpt, ent->mnt_dir);
            sprintf(mountfs, ent->mnt_fsname);

            /* thread 1 - check the mount point*/ 
            if((pthread_create( &check_mount,NULL,checkmount,NULL))!= 0){
                if(DEBUG){
                    printf("can't create new thread check_mount.. exiting\n");
                }
                printf("can't create new thread check_mount.. exiting\n");
                exit(1);
            }

            /* thread 2 - start the timer - timeout in 5 seconds */
            if((pthread_create( &time_out,NULL,timeout,NULL))!= 0){
                if(DEBUG){
                    printf("can't create new thread time_out .. exiting\n");
                }
                printf("can't create new thread time_out .. exiting\n");
                exit(1);
            }
       
            pthread_join( check_mount,NULL);
            pthread_join( time_out,NULL);
        }
    }

    exit(0);
}
    
/****************************************************************************
  * chkmount()
 ****************************************************************************/
void *checkmount(void *x){
    char cmd[MAX];

    sprintf(cmd, "df -h %s > /dev/null 2>&1",mountpt);
    if(DEBUG ){
        printf("starting %s\n",mountpt);
    }else{
       printf("testing %s\n",mountpt);
    }
    system(cmd);
   
    /* 
     * the system() returns control to the program
     * flow upon completion of the shell command - 
     * if if the df command hangs - the timeout thread will 
     * complete first and exit with an error - otherwise THIS
     * thread will complete first - but we need to cancel the 
     * timeout thread to avoid exiting
     */       

    pthread_cancel(time_out);
    return(0);
}

/****************************************************************************
 * timeout()
 ****************************************************************************/
void *timeout(){
    if(DEBUG ){
        printf("starting sleep\n");
    }
    sleep(5); /*timeout in 5 seconds */
    /*
     * if I reach this point - the chkmount thread has not cancled me
     * and I likley have a hung mount point .. I need to exit and report
     * the error
     */
    printf("check of %s on %s failed\n", mountpt, mountfs);
    exit(1);
}


