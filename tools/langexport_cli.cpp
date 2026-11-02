#include "../src/launcher/langexport.h"
#include <cstdio>
#include <cstring>
int main(int argc,char **argv) {
    if(argc!=3) { fprintf(stderr,"Usage: sdw_langexport GAME_DIR OUTPUT_PACK\n"); return 2; }
    char error[2048]={};
    if(!Export_OpenSource(argv[1],error,sizeof(error))) { fprintf(stderr,"%s\n",error);return 1; }
    Export_Init(argv[1]); int english=-1;
    for(int i=0;i<Export_Count();++i) { printf("Language %d: %s\n",i,Export_NameUtf8(i)); if(!strcmp(Export_NameUtf8(i),"English"))english=i; }
    if(english<0 || !Export_Language(english,argv[2],"Русский",false,false,error,sizeof(error))) {
        fprintf(stderr,"%s\n",error);return 1;
    }
    return 0;
}
