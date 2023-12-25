#define NOBUILD_IMPLEMENTATION
#include "./nobuild.h"

#define BIN "br"
#define SRC "main.c"
#define DESTDIR "/usr/local/bin/"
#define CFLAGS	"-Wall",		\
				"-pedantic",	\
				"-std=c2x"

char *cc(void);
void BuildBin(void);
void Install(void);
void Remove(void);
void Clean(void);

int main(int argc, char *argv[]) {
    GO_REBUILD_URSELF(argc, argv);
	
    if (argc < 2 ){
		BuildBin();
		return EXIT_SUCCESS;
	}

    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];

        if (arg[0] == '-') {
            for (int j = 1; j < strlen(arg); j++) {

                switch (arg[j]) {
                    case 'i':
						Install();
                        break;
                    case 'r':
                    	Remove(); 
                        break;
                    case 'f':
                       	Clean();
                        break;
                    default:
                        printf("Unknown option: %c\n", arg[j]);
                        break;
                }
            }
        } else {
    		printf("Usage: %s [-d] [-r] [-f]\n", argv[0]);
        }
    }
    return EXIT_SUCCESS;
}

char *cc(void){
    char *result = getenv("CC");
    return result ? result : "cc";
}

void BuildBin(void) {
	CMD(cc(), "-o", BIN, SRC);
}

void Install(void) {
	CMD("doas", "cp", BIN, DESTDIR);
}

void Remove(void) {
	CMD("doas", "rm", "-v", DESTDIR""BIN);
}

void Clean(void) {
	CMD("rm", BIN, "c.old");
}
