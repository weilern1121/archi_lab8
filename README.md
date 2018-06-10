# archi_lab8

# Task 0: <br />
* main: nothing unusual, menu and function calls as same as in task 7.<br />
* Examine ELF File: </br>- func that suplly information about the ELF file (error if not an ELF).<br />
                    - modyfied the first 40 lines from the code that shown in the lab session.</br>
                   </emsp> - map_start = mmap(0, (size_t) Currfd_stat.st_size... is where creating the header(Elf64_Ehdr *).</br>
                    - header->e_ident[1,2,3] - magic numbers.</br>
                    - header->e_ident[5] - lottle/big endian. </br>
                    - all other prints- by the pdf description.</br>
* reset_fd: an private func that used for safe-clean the Currentfd (GLOBAL VAR)</br>
* GLOBAL VARS: as descripted in the site.</br>
                    
--------------------------------------------------------------------------
# Task 1: <br />
* header-         pointer to the beginning of the starting mapped file.</br>
* section_header-</br>
pointer to the first section.</br>
valued: header+offest+(Section name string table index)*sizeOrEntry </br>
* str_table -</br>
pointer to the section names table.</br>
(map_start + header->e_shoff + ((header->e_shstrndx) * (header->e_shentsize)</br>
* curr_sec -</br>
An iterative pointer that points on the cuurent section for frinting, changes in the for loop.</br>
(map_start + header->e_shoff + (i * (header->e_shentsize)   NOTE:i is the foor var.</br>
* pad_space: a local func that print space (to align the rows).</br>

--------------------------------------------------------------------------
# Task 2: <br />
* moved header to GLOBAL (because of the use in local fun).</br>
* seek_table - itarates on all of the sections, if section type== symbol table -> print.</br>
* sym_size=24 - because this is the total size of fields in the Elf64_Sym struct (descripted in PDF).</br>
* sym_table - local usage of seek_table.
