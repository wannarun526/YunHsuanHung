#include <stdio.h>

#define KM_START   0
#define KM_END     50
#define KM_STEP    10
#define KM_TO_MILE 0.621371f

int main(void)
{
    for (int km = KM_START; km <= KM_END; km += KM_STEP)
        printf("%3d km = %6.2f mi\n", km, km * KM_TO_MILE);
    return 0;
}