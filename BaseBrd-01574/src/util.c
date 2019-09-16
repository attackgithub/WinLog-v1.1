#include <windows.h> // HKEY, LPCTSTR ect ...
#include <stdio.h> // printf()
#include <time.h>   // time(), localtime(), struct tm, clock_t

#include "../includes/util.h"

void wait_time_end(float total_time)
{
    clock_t end_of_record = clock() + ((clock_t)total_time * CLOCKS_PER_SEC);
    while(clock() < end_of_record);

    return;
}

