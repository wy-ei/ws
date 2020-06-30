#include <unistd.h>
#include "../utils/perf.h"

int perf_test(){
    {
        ScopeRunningTime SRT;
        double b = 3.14;
        for(int i=0;i<1000000000;i++){
            b += 3.14 * 3.14;
        }
    }

    {
        ScopeRunningTime SRT;
        sleep(1);
    }
}