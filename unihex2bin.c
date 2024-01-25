#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

static inline uint8_t reverse(uint8_t b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

int main(int argc, char *argv[]) {
    int opt;
    char *string = "i:o:p:c:";
    FILE* fin = NULL;
    FILE* fout = NULL;
    FILE* fcomb = NULL;
    bool gen_comb=false;
    int plane_low = 0;
    int plane_high = 0;
    char fline[100];
    char version=0;
    while ((opt = getopt(argc, argv, string)) != -1) {
        switch(opt) {
            case 'i':
            fin = fopen(optarg, "r");
            break;
            case 'o':
            fout = fopen(optarg, "wb+");
            break;
            case 'p':{
                int num1 = (int)atoi(optarg);
                char* ret = strchr(optarg, '-');
                if(ret == NULL) {
                    plane_low = num1;
                    plane_high = num1;
                } else {
                    int num2 = (int)atoi(ret+1);
                    plane_low = MIN(num1, num2);
                    plane_high = MAX(num1, num2);
                }
            }
            break;
            case 'c':
                fcomb=fopen(optarg, "r");
                gen_comb=true;
            break;
        }
    }
    if(fout == NULL) {
        puts("Cannot open bin file\n");
        exit(1);
    }
    if(fin == NULL) {
        puts("Cannot open hex file\n");
        exit(1);
    }
    if(fcomb==NULL&&gen_comb==true){
        puts("Cannot open comb file");
        exit(1);
    }
    fputs("UFB", fout);
    fputc(version, fout);
    long header_length=4+4+2+2+4*(plane_high-plane_low+1)*65536;
    fwrite(&header_length,4,1,fout);
    fputc((char)plane_low,fout);
    fputc((char)plane_high,fout);
    fputc(0xFF,fout);
    fputc(0xFF,fout);
    char* endptr;
    uint32_t prev_p=header_length;
    uint32_t off;
    uint32_t code;
    int8_t comb_off;
    char* tmp;
    while(fgets(fline,100,fin)!=NULL){
        code=(uint32_t)strtol(fline,&endptr,16);
        if(code/0x10000>=plane_low&&code/0x10000<=plane_high){
            endptr++;
            int glyph_width=strlen(endptr)/4;
            char byte_str[3]={0,0,'\0'};
            uint8_t byte;
            fseek(fout,prev_p,SEEK_SET);
            for(int copied=0;copied<glyph_width*2;copied++){
                strncpy(byte_str,endptr,2);
                byte=(uint8_t)strtol(byte_str,&tmp,16);
                byte=reverse(byte);
                fputc(byte,fout);
                endptr+=2;
            }
            prev_p=ftell(fout);
            off=prev_p-header_length-glyph_width*2;
            if(glyph_width==16){
                off|=0x80000000;
            }
            fseek(fout,12+(code-plane_low*0x10000)*4,SEEK_SET);
            fwrite(&off,4,1,fout);
        }
    }
    if(gen_comb==true){
        while(fgets(fline,100,fcomb)!=NULL){
            code=(uint32_t)strtol(fline,&endptr,16);
            if(code/0x10000>=plane_low&&code/0x10000<=plane_high){
                endptr++;
                fseek(fout,12+(code-plane_low*0x10000)*4,SEEK_SET);
                fread(&off,4,1,fout);
                comb_off=(int8_t)strtol(endptr,&tmp,10);
                comb_off/=2;
                off|=((uint32_t)comb_off&0xF)<<26;
                off|=0x40000000;
                fseek(fout,12+(code-plane_low*0x10000)*4,SEEK_SET);
                fwrite(&off,4,1,fout);
            }
        }
        fclose(fcomb);
    }
    fclose(fout);
    fclose(fin);
    return 0;
}