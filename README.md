Comparison of `__sync_add_and_fetch` vs mutex vs spin lock
==============================================================
Program to compare the performance of `__sync_add_and_fetch` vs mutex vs spin lock when incrementing a global variable by multiple threads.

The graph shows that spin locks are faster when the number of threads is small (< 4):
![Comparison](threads.png?raw=true "Comparison")

Zoom:
![Zoom](zoom.png?raw=true "Zoom")


The test has been performed in an Intel i7 (4 cores, 8 threads):
```
$ lscpu 
Architecture:          x86_64
CPU op-mode(s):        32-bit, 64-bit
Byte Order:            Little Endian
CPU(s):                8
On-line CPU(s) list:   0-7
Thread(s) per core:    2
Core(s) per socket:    4
Socket(s):             1
NUMA node(s):          1
Vendor ID:             GenuineIntel
CPU family:            6
Model:                 58
Model name:            Intel(R) Core(TM) i7-3630QM CPU @ 2.40GHz
Stepping:              9
CPU MHz:               1513.500
CPU max MHz:           3400.0000
CPU min MHz:           1200.0000
BogoMIPS:              4788.97
Virtualization:        VT-x
L1d cache:             32K
L1i cache:             32K
L2 cache:              256K
L3 cache:              6144K
NUMA node0 CPU(s):     0-7
```

Kernel:
```
$ uname -a
Linux debian 3.16.0-4-amd64 #1 SMP Debian 3.16.7-ckt25-2 (2016-04-08) x86_64 GNU/Linux
```

Compiler:
```
$ gcc --version
gcc (Debian 4.9.2-10) 4.9.2
```

Compiled with:
```
gcc -O3 -Wall --pedantic -Wno-long-long -Wno-format -o threads -lpthread threads.c
```

Executed as:
```
./threads > threads.dat
```

The graph has been performed with `gnuplot` as follows:
```
set xlabel "Threads"
set ylabel "Time (microseconds)"
set xtics add 1
plot "threads.dat" using 1:2 title '\_\_sync\_add\_and\_fetch' with lines, "threads.dat" using 1:3 title 'mutex' with lines, "threads.dat" using 1:4 title 'spinlock' with lines
```

File `threads.dat`:
```
# Threads                __sync_add_and_fetch     mutex                    spinlock                 
2                        70977                    197878                   98813                    
3                        80476                    354231                   291902                   
4                        102664                   461337                   621058                   
5                        119961                   556652                   791256                   
6                        132376                   778103                   1016090                  
7                        179626                   965425                   1204594                  
8                        190651                   1089236                  1469542                  
9                        216281                   1241860                  1845462                  
10                       239592                   1388527                  2051816                  
11                       254490                   1545882                  2720259                  
12                       289512                   1680452                  3116106                  
13                       310595                   1844968                  3617209                  
14                       333621                   1985225                  4305381                  
15                       344508                   2133497                  4741892                  
16                       370712                   2287872                  5554342                  
17                       407957                   2437609                  6254307                  
18                       437474                   2589524                  7144674                  
19                       455584                   2742479                  7742627                  
20                       477795                   2892815                  8765340                  
21                       501818                   3042208                  9166648                  
22                       529629                   3189476                  9738211                  
23                       549650                   3337107                  10347301                 
24                       573642                   3487802                  11310559                 
25                       600736                   3642055                  12741959                 
26                       623713                   3793208                  13493917                 
27                       645876                   3958156                  14228948                 
28                       674060                   4102978                  15604453                 
29                       691310                   4260943                  16346946                 
30                       716741                   4411153                  16973479                 
31                       742418                   4561745                  19047024                 
32                       766855                   4719611                  19848293                 
```
