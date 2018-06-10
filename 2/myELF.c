#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <elf.h>
#include <unistd.h>
#include <memory.h>

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
Elf64_Ehdr* header;

void quit(){
    if(Currentfd!=-1){
        munmap(map_start, (size_t) Currfd_stat.st_size);
        close( Currentfd );
    }
    FREE(fileName);
    exit(EXIT_SUCCESS);
}

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

void pad_space(char* str,int num){
    size_t i=strlen(str);
    for(;i<num;i++)
        printf(" ");
}

void pad_space_number(int num,int pad){
    int i=0;
    while(num>9){
        i++;
        num=num/10;
    }
    int j=pad-i;
    for(;j>0;j--)
        printf(" ");
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
    printf("\nELF Header:\n");
    printf("\t1. Bytes 1,2,3 of the magic number:");
    pad_space("1. Bytes 1,2,3 of the magic number:",65);
    printf("%c %c %c\n", header->e_ident[1], header->e_ident[2], header->e_ident[3]);  //Bytes 1,2,3 of the magic number (in ASCII)
    printf("\t2. The data was encoded in:");
    pad_space("2. The data was encoded in:",65);
    printf("%s endian\n",endian);
    printf("\t3. Entry point (hexadecimal address):");
    pad_space("3. Entry point (hexadecimal address):",65);
    printf("%lx \n", header->e_entry); //Entry point (in hexadecimal)
    printf("\t4. The file offset :");
    pad_space("4. The file offset :",65);
    printf("%lx\n", header->e_shoff);
    printf("\t5. Number of section header entries:");
    pad_space("5. Number of section header entries:",65);
    printf("%d\n", header->e_shnum);
    printf("\t6. Size of each section header entry:");
    pad_space("6. Size of each section header entry:",65);
    printf("%x\n", header->e_shentsize);
    printf("\t7. File offset in which the program header table resides:");
    pad_space("7. File offset in which the program header table resides:",65);
    printf("%lx\n",header->e_phoff);
    printf("\t8. Number of program header entries:");
    pad_space("8. Number of program header entries:",65);
    printf("%x\n",header->e_phnum);
    printf("\t9. Size of each program header entry:");
    pad_space("9. Size of each program header entry:",65);
    printf("%x\n", header->e_phentsize);

    printf("\n");
}

void Print_section_names(){
    char *str_table;
    Elf64_Shdr *section_header;
    Elf64_Shdr *curr_sec;
    Elf64_Ehdr *header = (Elf64_Ehdr *) map_start; //*header is pointing to the starting mapped file

    //if Currentfd is invalid, just print an error message and return.
    if(Currentfd==-1){
        perror("Currentfd is -1 -> not valid!");
        exit(EXIT_FAILURE);
    }

    printf("\nSection Header:\n");
    printf("\t[index]  section     section     section   section    section\n");
    printf("\t          name       address     offset     size       type\n");
    printf("--------------------------------------------------------------------------\n");

    /*
     * move pointer to the first section:
     *section_header= header+offset+(Section name string table index)*sizeOrEntry
     * also move str_table pointer to the names table:
     * str_table=header+offset
     */

    section_header= (Elf64_Shdr *) (map_start + header->e_shoff + ((header->e_shstrndx) * (header->e_shentsize)));
    str_table = ( map_start + ( section_header -> sh_offset ) );
    for(int i=0; i< (header->e_shnum);i++){
        //move curr_sec to the current sector:
        //curr_sec= header+offset+i*(sizeOfSectionHeader)
        curr_sec= (Elf64_Shdr *) (map_start + header->e_shoff + (i * (header->e_shentsize)));
        //prints
        if(i<10)
            printf("\t [0%d]\t",i );
        else
            printf("\t [%d]\t",i);
        /*if(strlen(str_table + curr_sec -> sh_name)<2)
            printf("\t");*/
        printf("%s   ",(str_table + curr_sec -> sh_name) );
        pad_space((str_table + curr_sec -> sh_name),10);
        printf("%08lx \t %06lx    %06lx    %06x\n",curr_sec -> sh_addr ,curr_sec -> sh_offset ,curr_sec -> sh_size,curr_sec -> sh_type);

    }
    printf("\n");

}

char* get_name(int i){
    int shdr= sizeof(Elf64_Shdr);
    Elf64_Shdr* table= (Elf64_Shdr *) (map_start + header->e_shoff + (shdr * header->e_shstrndx));
    return map_start+table->sh_offset+i;
}

void Print_symbols(){
    //if Currentfd is invalid, just print an error message and return.
    if(Currentfd==-1){
        perror("Currentfd is -1 -> not valid!");
        exit(EXIT_FAILURE);
    }
    header = (Elf64_Ehdr *) map_start; //*header is pointing to the starting mapped file
    Elf64_Shdr* seek_table = (Elf64_Shdr *) (map_start + header->e_shoff);
    //Elf64_Shdr* name_ptr;
    int sym_size=24;//sizeof(Elf64_Sym)
    long offset=0;
    printf("\nSymbol Table:\n");
    printf("\t                              section     section        symbol\n");
    printf("\t[index]    value                id         name           name\n");
    printf("--------------------------------------------------------------------------------\n");

    for(int i=0; i<header->e_shnum;i++) {
        //printf("seek_table->sh_type: %d, \n",seek_table->sh_type);
        if (seek_table->sh_type==SHT_SYMTAB || seek_table->sh_type==SHT_DYNSYM) {//if TRUE ->sym_table has found
            //name_ptr = (Elf64_Shdr *) (map_start + header->e_shoff+ (sizeof(Elf64_Shdr)*header->e_shstrndx));
            unsigned long s_offset=0;
            Elf64_Sym *sym_table = (Elf64_Sym *) (map_start + seek_table->sh_offset);
            int table_size = ((int) seek_table->sh_size) / sym_size;
            char *sec_name;
            char *sym_name;
            //prints iteration
            for (int j = 0; j < table_size; j++) {
                //prints
                //index
                if (j < 10)
                    printf("\t [0%d]\t", j);
                else
                    printf("\t [%d]\t", j);
                //value
                printf("%016lx\t", sym_table->st_value);
                //pad_space_number((int) sym_table->st_value, 20);
                //section index
                int shndx=sym_table->st_shndx;
                if(shndx==0){
                    printf("UND");
                    pad_space("UND",10);
                }
                else if(shndx==65521) {
                    printf("ABS");
                    pad_space("ABS",10);
                }
                else {
                    printf("%d", sym_table->st_shndx);
                    pad_space_number(sym_table->st_shndx, 10);
                }
                //name
                if (sym_table->st_shndx == 65521 || sym_table->st_shndx == 0) //check NULL
                    sec_name = "";
                else { //go to table+place in the table
                   Elf64_Shdr* loc= (Elf64_Shdr *) (map_start + header->e_shoff);
                   sec_name=get_name(loc[sym_table->st_shndx].sh_name);
                }
                printf("%s", sec_name);
                pad_space(sec_name, 15);
                //symbol
                Elf64_Shdr *loc = (Elf64_Shdr *) (map_start + header->e_shoff);
                sym_name = map_start + loc[seek_table->sh_link].sh_offset + sym_table->st_name;
                printf("%s", sym_name);

                //prepare next iteration in the table
                s_offset+=sym_size;
                sym_table= (Elf64_Sym *) (map_start + seek_table->sh_offset + s_offset);
                printf("\n");
            }
        }
        //prepare next iteration
        offset+= sizeof(Elf64_Shdr);
        seek_table= (Elf64_Shdr *) (map_start + header->e_shoff + offset);
    }
    printf("\n");
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
                                 {"2-Print Section Names",Print_section_names},
                                 {"3-Print Symbols",Print_symbols},
                                 {"4-Quit", quit}
    };

    //int size_arr = sizeof(menu)/sizeof(action_menu);
    while(cond){
        printf("Choose an action: \n");
        for(int i=0;i<4;i++) //print menu
            printf("%s \n", action_menu[i].func_name);
        scanf("%d", &input);
        if(input>0 &&input <5)//check input
            action_menu[input-1].func();
        else
            perror("illegal input!");
    }
    return 0;
}


