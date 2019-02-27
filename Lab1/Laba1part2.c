#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#define BLOCK 64

int main (int argc, char ** argv){

  //для потомка, в ктр помещается сообщение, а для предка - принимается
  struct Message { long mtype; ssize_t size; char buff[BLOCK];} message;
  
  if (argc !=2){
      fprintf(stderr, "Error. Enter a file name.\n");
      return 1;
  }

  //создание нового экземпляра механизма очереди сообщений
  int msg_id = msgget (IPC_PRIVATE, IPC_CREAT | 0666);
  if (msg_id == -1){
    fprintf(stderr, "Initialization message queue error");
    return 1;
  }

  //порождение потомка
  pid_t son_pid = fork();

  if (son_pid != 0){ //предок
    
    ssize_t charswr = BLOCK;
    message.mtype = 1; //тэг сообщения, ктр надо принять
    while (1){
      //прием из очереди сообщений с ключом в структуру данных размером с тэгом в порядке очереди постановки (0)
      msgrcv(msg_id,&message, BLOCK * sizeof(char) + sizeof (ssize_t), message.mtype, 0);
      //закончилась передача 
      if (message.size == 0){
	return 0;
      }
      //вывод на экран полученного сообщения размером
      write(STDOUT_FILENO, message.buff, message.size);
    }
  }
  else {//потомок
    int FILE;
    ssize_t charsrd;

    //попытка открыть файл
    if ((FILE = open (argv[1],O_RDONLY)) == -1){
      fprintf(stderr, "Error. File is not open.");
      return 1;
    }

    //назначаем сообщения с тэгом для очереди сообщений
    message.mtype = 1;
    //подготавливаем сообщение для передачи
    while ((charsrd = read(FILE, message.buff, BLOCK)) > 0){
      message.size = charsrd;
      msgsnd(msg_id, &message, BLOCK * sizeof(char) + sizeof (ssize_t), 0);
    }
    message.size = 0;
    msgsnd(msg_id, &message, BLOCK * sizeof(char) + sizeof(ssize_t),0);

    return 0;
  }
}
