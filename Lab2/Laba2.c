#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#define BLOCK 64

int main (int argc, char ** argv){
  struct sembuf semoper; //для операции
  union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;} sem;
  char* charshm;
  int flg = 1;
  
  if (argc !=2){
      fprintf(stderr, "Error. Enter a file name.\n");
      return 1;
  }

  int sem_id = semget (IPC_PRIVATE, 2, IPC_CREAT | 0666);
  if (sem_id == -1){
    fprintf(stderr, "Initialization semaphore error");
    return 1;
  }
  sem.val = 1;
  semctl(sem_id,0,SETVAL,sem);
  sem.val = 0;
  semctl(sem_id,1,SETVAL,sem);
  semctl(sem_id,2,SETVAL,sem);
      
  int shm_id = shmget (IPC_PRIVATE, BLOCK, IPC_CREAT | 0666);
  if (shm_id == -1){
    fprintf(stderr, "Initialization shared memory error");
    return 1;
  }
 
  pid_t son_pid = fork();

  if (son_pid != 0){
    //предок
    ssize_t charswr;
    
    charshm = (char *) shmat(shm_id, 0, 0);
    while (flg == 1){

      semoper.sem_num = 1;
      semoper.sem_op = -1;
      semoper.sem_flg = 0;
      semop(sem_id,&semoper,1);

      if (*charshm != '\0'){
	charswr = write(STDOUT_FILENO, (void *) charshm, BLOCK);
      }
      else {
	return 0;}

      semoper.sem_num = 0;
      semoper.sem_op = 1;
      semoper.sem_flg = 0;
      semop(sem_id,&semoper,1);
    }
  }
  else {
    //потомок
    int FILE;
    ssize_t charsrd;
  
    if ((FILE = open (argv[1],O_RDONLY)) == -1){
      fprintf(stderr, "Error. File is not open.");
      return 1;
    }

    charshm = (char *) shmat(shm_id, 0, 0);

    while (flg == 1){

      semoper.sem_num = 0;
      semoper.sem_op = -1;
      semoper.sem_flg = 0;
      semop(sem_id,&semoper,1);

      charsrd = read (FILE, (void *) charshm, BLOCK);

      if (charsrd == BLOCK) { }
      else if (charsrd > 0) {
	memset(charshm+charsrd, '\0', BLOCK - (int)charsrd);
	close (FILE);
      }
      else{
	memset(charshm, '\0', BLOCK);
	flg = 0;
      }

      semoper.sem_num = 1;
      semoper.sem_op = 1;
      semoper.sem_flg = 0;
      semop(sem_id,&semoper,1);
    }
    return 0;
  }
}
