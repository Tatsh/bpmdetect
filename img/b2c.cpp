#include <stdio.h>

static const char digit[] = {
    '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'
};

int main(int argc, char* argv[]) {
    if(argc < 2) return 1;

    FILE* file = fopen(argv[1], "rb");
    if(!file) return 1;

    fprintf(stdout, "static const unsigned char data[] = {");

    bool firstline = true;
    while(!feof(file)) {
    #define SIZE 16
        unsigned char buffer[SIZE];
        int nread = fread(buffer, 1, SIZE, file);
        if(nread) {
            if(!firstline) fprintf(stdout, ",");
            else firstline = false;
            fprintf(stdout, "\n    ");
        }

        for(int i = 0; i < nread; ++i) {
            unsigned char c = buffer[i];
            fprintf(stdout, "0x%c%c", digit[c/16], digit[c%16]);
            if(i < nread-1) fprintf(stdout, ",");
        }
    }

    fprintf(stdout, "\n};\n");
    fclose(file);
}
