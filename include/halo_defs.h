/* date = September 29th 2023 4:08 am */

#ifndef HALO_DEFS_H
#define HALO_DEFS_H

#define PROGRAM_MEMORY_OFFSET 0x0000
#define VARIABLE_MEMORY_OFFSET 0x4001
#define HEAP_MEMORY_OFFSET 0x8001
#define FREE_MEMORY_OFFSET 0xb001
#define MEMORY_LIMIT 0xffff

#define CLOCK_SPEED_HZ 5
#define CLOCK_SPEED_MS 1000 / CLOCK_SPEED_HZ

#define RA 0
#define RB 1
#define RD 2
#define RE 3
#define RF 4
#define RC 5
#define RT 6
#define RSP 7
#define RDP 8

#define HIN 0
#define HOUT 1

#define IO 0

#endif //HALO_DEFS_H
