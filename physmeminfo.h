struct physframe_info {
    uint frame_index;   // 물리 프레임 번호 (0 ~ MAX_FRAMES-1)
    int allocated;      // 1: 사용 중, 0: free
    int pid;            // 소유 프로세스 PID (-1 또는 0은 커널)
    uint start_tick;    // 이 프레임을 사용하기 시작한 시점 (ticks 단위)
};

#define PFNNUM 60000
