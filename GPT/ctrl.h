#include <windows.h>
#include <stdio.h>
#include "data.h"

#define PDT_GPT 0
#define PDT_MBR 0

#define DPT_NULL 0
#define DPT_GPT  1
#define DPT_MBR  2
#define DPT_EBR  3

#define SECTOR_SIZE 512

// 分区表中只有一项主分区数据, 在0x1C2处记录的系统信息如果是0xEE则硬盘采用GPT分区，否则就是MBR分区
bool PartitionTableDetermination(unsigned char* SectorData);

uint16_t ArrayToU16LittleEnd(uint8_t *str);
uint16_t ArrayToU16BigEnd(uint8_t *str);
uint32_t ArrayToU32LittleEnd(uint8_t *str);
uint64_t ArrayToU48LittleEnd(uint8_t *str);
uint64_t ArrayToU48BigEnd(uint8_t *str);
uint64_t ArrayToU64LittleEnd(uint8_t *str);
uint64_t ArrayToU64BigEnd(uint8_t *str);

// 通过给定磁盘的编号，获取到磁盘的句柄
HANDLE GetDiskHandle(int iDiskNo);

// 读取扇区数据
BOOL ReadSectorData(HANDLE& hDevice, uint64_t redpos, uint8_t* lpOutBuffer, uint32_t ReadLen);

// 将按紧密字节排列的DPT_Byte转换为4个稀疏结构体DPT_Info以便于程序获取信息
void GetDPTInfo(DPT_Byte* src, DPT_Info* dest);

// 获取指定磁盘的磁盘分区表DPT
void GetDPT(HANDLE hDisk, DPT_Info* DPT);

// 根据DPT的分区类型判断磁盘分区表是GPT或MBR
int DPTDetermination(DPT_Info* DPT);

// 获取一个MBR格式的磁盘中有效分卷数，暂时不带EBR，也就是说最多4个MBR
int GetPartitionNumberOfMBR(HANDLE hDisk, DPT_Info* DPT);

// 将按紧密字节排列的PGPTH_Byte转换为稀疏结构体PGPTH_Info以便于程序获取信息
void GetPGPTHInfo(PGPTH_Byte* src, PGPTH_Info* dest);

static inline GUID_Info ArrayToGUID(uint8_t* str);

// 获取指定磁盘的主GPT头(Primary GPT Header,程序中简化为PGPTH)
void GetPGPTH(HANDLE hDisk, PGPTH_Info* PGPTH);

// 读取磁盘数据，不可以打破扇区读取
BOOL ReadDiskData(HANDLE& hDevice, uint64_t redpos, uint8_t* lpOutBuffer, uint32_t ReadLen);

// 获取一个GPT格式的磁盘中有效分卷数
int GetPartitionNumberOfGPT(HANDLE hDisk, DPT_Info* DPT);

// 获取磁盘分区数量， 不带EBR
int GetPartitionNumber(HANDLE hDisk);

// 获取指定磁盘的指定LBA信息
void GetGPTE(HANDLE hDisk, PGPTH_Info* PGPTH, GPTEntry_Info* GPTE, uint8_t GPTESerial);

// 获取分卷信息，暂时不带EBR
int GetPartitions(HANDLE hDisk, Partition_Info* Partitions);

//显示分区列表
void ShowPartitions(Partition_Info* Partitions, int len);