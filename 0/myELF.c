#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <elf.h>
#include <unistd.h>

#define FREE(X) if(X) free((void*)X)

typedef struct menu{
    char *func_name;
    void (*func)();
}menu;


//GLOBAL vars
char* map_start; //indicate the memory location of the mapped file
int Currentfd; //currently open file (indicated by global variable Currentfd)
struct stat Currfd_stat; //needed to  the size of the file
char* fileName;
int debug_mode;

void reset_fd (int num){
    switch (num){
        case 1:
            munmap(map_start, (size_t) Currfd_stat.st_size);
            map_start = 0;
            break;
        case 2:
            close( Currentfd );
            break;
        case 3: //case 1+2
            munmap(map_start , (size_t) Currfd_stat.st_size);
            map_start = 0;
            close( Currentfd );
        default:
            break;
    }
    Currentfd = -1;
}

void examine_ELF_file(){
    Elf64_Ehdr *header; /* this will point to the header structure */
    char* endian; //little/big endian indicator

    printf("enter file name\n");
    scanf("%s", fileName);
    fflush(stdin);
    if(!fileName){
        perror("filename is NULL!");
        reset_fd(1);
        exit(EXIT_FAILURE);
    }
    //init curr_fd
    if( Currentfd != -1 )
        reset_fd( 1 );

    if( (Currentfd = open(fileName, O_RDWR)) < 0 ) {
        perror("error in open");
        reset_fd(2);
        exit(EXIT_FAILURE);
    }

    if( fstat(Currentfd, &Currfd_stat) != 0 ) {
        perror("stat failed");
        reset_fd(2);
        exit(EXIT_FAILURE);
    }

    if ((map_start = mmap(0, (size_t) Currfd_stat.st_size, PROT_READ , MAP_SHARED, Currentfd, 0)) == MAP_FAILED ) {
        perror("mmap failed");
        reset_fd(2);
        exit(EXIT_FAILURE);
    }

    /* now, the file is mapped starting at map_start.
     * all we need to do is tell *header to point at the same address:
     */
    header = (Elf64_Ehdr *) map_start;
    //ELF - check that the number is consistent with an ELF file, and refuse to continue if it is not.
    if(header->e_ident[1]!='E' ||header->e_ident[2]!='L' || header->e_ident[3]!='F'){
        perror("Not an ELF-file!");
        reset_fd(3);
        exit(EXIT_FAILURE);
    }
    //check big/little endian
    if(header->e_ident[5]==1) //El_DATA=1 ->Object file data structures are little-endian
        endian="little";
    else                        //El_DATA=2 ->Object file data structures are big-endian
        endian="big";

    //prints
    printf("\nExamine ELF File:\n");
    printf("\t1. Bytes 1,2,3 of the magic number: %c %c %c\n", header->e_ident[1], header->e_ident[2], header->e_ident[3]);  //Bytes 1,2,3 of the magic number (in ASCII)
    printf("\t2. The data was encoded in %s endian.\n",endian);
    printf("\t3. Entry point (hexadecimal address): %lx \n", header->e_entry); //Entry point (in hexadecimal)
    printf("\t4. The file offset : %lx\n", header->e_shoff);
    printf("\t5. Number of section header entries: %d\n", header->e_shnum);
    printf("\t6. Size of each section header entry: %x\n", header->e_shentsize);
    printf("\t7. File offset in which the program header table resides: %lx\n",header->e_phoff);
    printf("\t8. Number of program header entries: %x\n",header->e_phnum);
    printf("\t9. Size of each program header entry: %x\n", header->e_phentsize);

    printf("\n");
}

void quit(){
    if(Currentfd!=-1){
        munmap(map_start, (size_t) Currfd_stat.st_size);
        close( Currentfd );
    }
    FREE(fileName);
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {

    int cond=1,input;
    //init GLOBALS
    fileName = malloc(100);
    debug_mode = 0;
    Currentfd = -1;
    map_start = 0;
    //define the menu struct
    struct menu action_menu[] = {{"1-Examine ELF File", examine_ELF_file},
                          {"2-Quit", quit},
    };

    //int size_arr = sizeof(menu)/sizeof(action_menu);
    while(cond){
        printf("Choose an action: \n");
        for(int i=0;i<2;i++) //print menu
            printf("%s \n", action_menu[i].func_name);
        scanf("%d", &input);
        if(input>0 &&input <3)//check input
            action_menu[input-1].func();
        else
            perror("illegal input!");
    }
    return 0;
}


