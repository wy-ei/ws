#include <unistd.h>
#include "../utils/perf.h"

void perf_test(){
    {
        ScopeRunningTime srt;
        double b = 3.14;
        for(int i=0;i<1000000000;i++){
            b += 3.14 * 3.14;
        }
    }

    {
        ScopeRunningTime srt;
        sleep(1);
    }
}