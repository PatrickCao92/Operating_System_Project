Project5 Designing a Virtual Memory Manager
Student: Shuaiqi Cao (sc3ez)

This project is to build a virtual memory manager to achieve Address Translation and Handling Page Faults. Input file contains a set of virtual address, and the output will show the contents of the page table, contents of the page table, page-fault rate and TLB hit rate.
Four structs are defined to represent current address, page table, frame table and TLB table. The counter of page fault, TLB hit are defined as global variables. 
The main steps are shown below.
1. The program would read in the address file, and get the address number, then store that into an array.
2. Initialize all pageable, TLB table and frame table, set their page number, frame number and access time to -1.
3. Use for loop to read in the address in order, within the for loop, the operations are:
Update the time counter by plus 1, so, every time a new address is read in, the time will accumulated by 1, this would be used as the time reference to assign the access time page table and TLB table.
Extract the address, calculate the page number and offset.
Load the address to TLB. Within TLB, check if the page is in TLB, if not, call the page table seeking, check wether the page in loaded on frame, if not, page fault happens, apply LRU to do the replacement. For doing the LRU, use the access time of each page in the page table. After the replacement, update the access time. Once load the frame, call the backing store function to load the data into physical memory defined in the program.  By following those procedures, process all the addresses. 
Finally print all results.