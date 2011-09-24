#include <libgadu.h>

int main(int argc, char *argv[])
{
#ifdef GG_CONFIG_HAVE_PTHREAD
    return 1;
#else
    return 0;
#endif
}

