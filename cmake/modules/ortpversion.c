#include <ortp/ortp.h>

int main(int argc, char *argv[])
{
        int major = 0;
        int minor = 0;
        int micro = 0;
        switch (argc)
        {
        case 1 :
                return EXIT_FAILURE;
        case 2 :
                major = atoi(argv[1]);
                break;
        case 3 :
                major = atoi(argv[1]);
                minor = atoi(argv[2]);
                break;
        case 4 :
                major = atoi(argv[1]);
                minor = atoi(argv[2]);
                micro = atoi(argv[3]);
                break;
        }
        if (ortp_min_version_required(major, minor, micro))
                return EXIT_SUCCESS;
        else
                return EXIT_FAILURE;
}
