#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

static void
usage(void) {
  printf(1, "usage: memstress [-n pages] [-t ticks] [-w]\n");
  exit();
}

int
main(int argc, char *argv[])
{
  int pages = 10;	//기본 확보 페이지
  int hold_ticks = 200;	//기본 유지 틱
  int do_write = 0;	//쓰기 수행 여부

  // 옵션 처리 - 반복문으로 순서상관없게 함
  for(int i = 1 ; i < argc ; i++){
    if (strcmp(argv[i], "-n") == 0 && (i+1 < argc)) {
      pages = atoi(argv[++i]);
    } else if (strcmp(argv[i], "-t") == 0 && (i+1 < argc)) {
      hold_ticks = atoi(argv[++i]);
    } else if (strcmp(argv[i], "-w") == 0) {
      do_write = 1;
    } else {
      usage();
    }
  }

  //현재 프로세스 ID 얻고, 정보 출력
  int pid = getpid();
  printf(1, "[memstress] pid=%d pages=%d hold=%d ticks write=%d\n", pid, pages, hold_ticks, do_write);

  //실제 메모리 확보 (동적 확보)
  int inc = pages * 4096; 	//페이지 크기 4KB 이므로 *4096
  char *base = sbrk(inc);	//sbrk() : 힙을 확장하여 페이지 확보 (내부에서 allocuvm -> kalloc하게 됨)
  if (base == (char*)-1) {
    printf(1, "[memstress] sbrk failed\n");
    exit();
  }

  //각 페이지 첫번째 바이트에 0xff 저장
  if (do_write) {
    for (int p = 0; p < pages; p++) {
      base[p*4096] = (char)(p & 0xff);
    }
  }

  //지정된 tick수 동안 대기 > 프레임 일정 시간 보유
  sleep(hold_ticks);

  //완료 출력 후 종료
  printf(1, "[memstress] pid=%d done\n", pid);
  exit();
}
