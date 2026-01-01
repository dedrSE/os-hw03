#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int pid;

  //자식 프로세스 생성 후 검증(pid = 4)
  pid = fork();
  if(pid < 0){
    printf(1, "fork failed\n");
    exit();
  }

  //자식 프로세스인 경우 memstress 실행
  //31페이지 확보, 500틱 동안 유지 
  if(pid == 0){
    char *args[] = { "memstress", "-n", "31", "-t", "500", 0 };
    exec("memstress", args);	//현재 프로세스(pid = 4)를 memstress로 대체
    printf(1, "exec memstress failed\n");
    exit();
  }

  //부모 프로세스는 잠시 대기 > 첫번째 memstress가 먼저 페이지 확보 시작
  sleep(100);  

  //두번째 memstress 프로세스 실행 (pid = 5)
  int pid2 = fork();
  if(pid2 == 0){	//자식 프로세스인 pid = 5 memstress로 대체
    char *args2[] = { "memstress", "-n", "31", "-t", "500", 0 };
    exec("memstress", args2);
    printf(1, "exec memstress failed\n");
    exit();
  }

  //두번째 memstress가 일정 부분 실행되도록 다시 대기
  sleep(100);

  //pid = 6 프로세스 생성
  int pid3 = fork();
  if(pid3 < 0){
    printf(1, "fork failed\n");
    exit();
  }

  //pid = 6프로세스에서 첫번째 memdump 실행->첫번째 memstress 프로세스 (pid = 4) pfinfo 조회
  if(pid3 == 0){
    char *args3[] = { "memdump", "-p", "4", 0 };
    exec("memdump", args3);
    printf(1, "exec memdump failed\n");
    exit();
  }

  sleep(100);

  //pid = 7 프로세스 생성
  int pid4 = fork();
  if(pid4 < 0){
    printf(1, "fork failed\n");
    exit();
  }

  //pid = 7 프로세스에서 두번째 memdump 실행-> 두번째 memstress 프로세스 (pid = 5) pfinfo 조회
  if(pid4 == 0){
    char *args4[] = { "memdump", "-p", "5", 0 };
    exec("memdump", args4);
    printf(1, "exec memdump failed\n");
    exit();
  }

  //위에서 fork한 자식들 정리 (wait)로 부모가 블록되어  자식 종료 대기
  wait();
  wait();
  wait();
  wait();

  //모든 프로세스 종료까지 잠시 대기
  sleep(100);

  //pid = 8 생성
  int pid5 = fork();
  if(pid5 < 0){
    printf(1, "fork failed\n");
    exit();
  }

  //pid = 8에서 memdump 실행 이때 pid= 5는 이미 종료되어 allocated가 0, 0이므로 출력 안됨
  if(pid5 == 0){
    char *args5[] = { "memdump", "-p", "5", 0 };
    exec("memdump", args5);
    printf(1, "exec memdump failed\n");
    exit();
  }

  //마지막 프로세스 종료 대기
  wait();  

  exit();
}

