#define NOBUILD_IMPLEMENTATION
#include <./nobuild.h>

#define CFLAGS "-Wall", "-Wextra", "-Wswitch-enum", "-pedantic", \
				"-pedantic-errors", "-Wmissing-include-dirs", "-O3"

#define BIN "br"
#define DEBUGGER "gdb"
#define SOURCE "main.c"
#define DESTDIR "/usr/local/bin"
#define CC "clang"

const char *cc(void){
    const char *result = getenv("CC");
    return result ? result : "cc";
}

int posix_main(int argc, char **argv){
    CMD(CC, CFLAGS, "-o", BIN, SOURCE);

    if (argc > 1){
        if (strcmp(argv[1], "run") == 0){
            CMD(BIN);
        }else if (strcmp(argv[1], "gdb") == 0){
            CMD(DEBUGGER, "./BIN");
        }else if (strcmp(argv[1], "valgrind") == 0){
            CMD("valgrind", "--error-exitcode=1", BIN);
        }else if (strcmp(argv[1], "install") == 0){
			CMD("doas", "cp", BIN, DESTDIR);
        }else{
            PANIC("%s is unknown subcommand", argv[1]);
        }
    }

    return 0;
}

int main(int argc, char **argv){
    GO_REBUILD_URSELF(argc, argv);

    return posix_main(argc, argv);
}

