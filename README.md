# archi_lab8

# Task 0: <br />
* main: nothing unusual, menu and function calls as same as in task 7.<br />
* Examine ELF File: - func that suplly information about the ELF file (error if not an ELF).<br />
                    </t>- modyfied the first 40 lines from the code that shown in the lab session.</br>
                    - map_start = mmap(0, (size_t) Currfd_stat.st_size... is where creating the header(Elf64_Ehdr *).</br>
                    - header->e_ident[1,2,3] - magic numbers.</br>
                    - header->e_ident[5] - lottle/big endian. </br>
                    - all other prints- by the pdf description.</br>
* reset_fd: an private func that used for safe-clean the Currentfd (GLOBAL VAR)</br>
*GLOBAL VARS: as descripted in the site.</br>
                    
--------------------------------------------------------------------------
