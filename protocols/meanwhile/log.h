#include <stdio.h>
#include <iostream>

#define LOG(msg) \
{                \
    FILE *fp = fopen("/tmp/mean.out","a"); \
    fprintf(fp,"%s\n",msg);                \
    fclose(fp);                            \
}
