#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define MAX_FRINFO 60000 

static void
usage(void)
{
    printf(1, "usage: memdump [-a] [-p PID]\n");
    exit();
}

int main(int argc, char *argv[])
{
    if (argc == 1)
        usage();

    int is_show_all = 0;	//-a 옵션
    int target_pid = -1;	//-p PID 옵션값

    // 옵션 처리
    if(strcmp(argv[1], "-a") == 0) {
	    is_show_all = 1;
    } else if (strcmp(argv[1], "-p") == 0) {
	    if(argc < 3) usage();

	    target_pid = atoi(argv[2]);
	    if(target_pid < 0) usage();
    }

    static struct physframe_info buf[MAX_FRINFO];	//시스템콜로 복사받을 버퍼
    int n = dump_physmem_info((void *)buf, MAX_FRINFO);	//시스템콜 호출
    if (n < 0)	//복사 count 
    {
        printf(1, "memdump: dump_physmem_info failed\n");
        exit();
    }

    //현재 프로세스 정보 출력
    printf(1, "[memdump] pid=%d\n", getpid());
    printf(1, "[frame#]\t[alloc]\t[pid]\t[start_tick]\n");
    
    for (int i = 0 ; i < n ; i++) {
	//출력 조건 : -a 옵션이거나, pid 옵션값이랑 같고, 실제 할당된 프레임만 출력
        if(is_show_all || (buf[i].pid == target_pid && buf[i].allocated == 1)) {
        printf(1, "%d\t\t%d\t%d\t%d\n"
	    , buf[i].frame_index
	    , buf[i].allocated
	    , buf[i].pid
	    , buf[i].start_tick);
    
	}
    }

    exit();
}
