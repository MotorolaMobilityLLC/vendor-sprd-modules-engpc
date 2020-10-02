#ifndef _PARTINFO_RW_H
#define _PARTINFO_RW_H

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long uint64;
// @param - cmd:
// 0: Query partition size,  1: Load partition data, 2: Save partition data
typedef struct
{
    uint32  cmd;    // 0: Query partition size
    char partition_name[32];
}diag_query_partition_info_req_t;

typedef struct
{
    uint32  cmd;    // 0: Query partition size
    uint32  status; // 0: pass, otherwise failure
    uint32  size;   // return the partition size
}diag_query_partition_info_ack_t;

typedef struct
{
    uint32  cmd;    // 1: read partition data
    char partition_name[32]; // Specifies the partition name
    uint32 offset;  // Specifies offset in bytes to read
    uint32 size;    // Specifies the size to read, maximum is 32KBytes.
} diag_read_partition_req_t;

typedef struct
{
    uint32  cmd;    // 1: read partition data
    uint32  status; // 0: pass, otherwise failure
    uint8   data[]; // return the read data in bytes.
} diag_read_partition_ack_t;


typedef struct
{
    uint32  cmd;    // 2: write partition data
    char partition_name[32]; // Specifies the partition name
    uint32 offset;  // Specifies offset in bytes to write
    uint32 size;    // Specifies the size to write, maximum is 32KBytes.
    uint8  data[];  // Specifies the data to write
} diag_write_partition_req_t;

typedef struct
{
    uint32  cmd;    // 2: write partition data
    uint32  status; // 0: pass, otherwise failure
} diag_write_partition_ack_t;

#endif
