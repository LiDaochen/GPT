#include"ctrl.h"

// 分区表中只有一项主分区数据, 在0x1C2处记录的系统信息如果是0xEE则硬盘采用GPT分区，否则就是MBR分区
bool PartitionTableDetermination(unsigned char* SectorData)

{
	if (SectorData[0x1C2] == 0xEE)

		return PDT_GPT;
	else
		return PDT_MBR;
}

uint16_t ArrayToU16LittleEnd(uint8_t *str)
{
	return (str[0] + (str[1] << 8));
}

uint16_t ArrayToU16BigEnd(uint8_t *str)
{
	return (str[1] + (str[0] << 8));
}

uint32_t ArrayToU32LittleEnd(uint8_t *str)
{
	return (str[0] + (str[1] << 8) + (str[2] << 16) + (str[3] << 24));
}

uint32_t ArrayToU32BigEnd(uint8_t *str)
{
	return (str[3] + (str[2] << 8) + (str[1] << 16) + (str[0] << 24));
}

uint64_t ArrayToU48LittleEnd(uint8_t *str)
{
	return  ((uint64_t)str[0] +
		((uint64_t)str[1] << 8) +
		((uint64_t)str[2] << 16) +
		((uint64_t)str[3] << 24) +
		((uint64_t)str[4] << 32) +
		((uint64_t)str[5] << 40));
}

uint64_t ArrayToU48BigEnd(uint8_t *str)
{
	return  ((uint64_t)str[5] +
		((uint64_t)str[4] << 8) +
		((uint64_t)str[3] << 16) +
		((uint64_t)str[2] << 24) +
		((uint64_t)str[1] << 32) +
		((uint64_t)str[0] << 40));
}

uint64_t ArrayToU64LittleEnd(uint8_t *str)
{
	return  ((uint64_t)str[0] +
		((uint64_t)str[1] << 8) +
		((uint64_t)str[2] << 16) +
		((uint64_t)str[3] << 24) +
		((uint64_t)str[4] << 32) +
		((uint64_t)str[5] << 40) +
		((uint64_t)str[6] << 48) +
		((uint64_t)str[7] << 56));
}

uint64_t ArrayToU64BigEnd(uint8_t *str)
{
	return  ((uint64_t)str[7] +
		((uint64_t)str[6] << 8) +
		((uint64_t)str[5] << 16) +
		((uint64_t)str[4] << 24) +
		((uint64_t)str[3] << 32) +
		((uint64_t)str[2] << 40) +
		((uint64_t)str[1] << 48) +
		((uint64_t)str[0] << 56));
}

// 通过给定磁盘的编号，获取到磁盘的句柄
HANDLE GetDiskHandle(int iDiskNo)
{
	char szDriverBuffer[128];
	memset(szDriverBuffer, 0, 128);
	//格式化设备文件名称
	sprintf(szDriverBuffer, "\\\\.\\PhysicalDrive%d", iDiskNo);
	HANDLE m_hDevice = NULL;
	//CreateFile获取到设备句柄
	m_hDevice = CreateFileA(
		szDriverBuffer,// 设备名称,这里指第一块硬盘，多个硬盘的自己修改就好了
		GENERIC_READ, // 指定读访问方式
		FILE_SHARE_READ | FILE_SHARE_WRITE, // 共享模式为读|写，0表示不能共享
		NULL, // NULL表示该句柄不能被子程序继承
		OPEN_EXISTING, // 打开已经存在的文件，文件不存在则函数调用失败
		NULL, // 指定文件属性
		NULL);
	if (m_hDevice == INVALID_HANDLE_VALUE)
	{
		m_hDevice = NULL;
		//无效
		return INVALID_HANDLE_VALUE;
	}
	//设备句柄
	return m_hDevice;
}

// 读取扇区数据
BOOL ReadSectorData(HANDLE& hDevice, uint64_t redpos, uint8_t* lpOutBuffer, uint32_t ReadLen)
{
	memset(lpOutBuffer, 0, ReadLen);
	LARGE_INTEGER li;
	li.QuadPart = redpos * 512;//求出扇区的字节地址，通过设置读取的地址和长度进行read
	SetFilePointer(hDevice, li.LowPart, &li.HighPart, FILE_BEGIN);
	DWORD DCount = 0; //计数
	BOOL bResult = ReadFile(hDevice, lpOutBuffer, ReadLen, &DCount, NULL);
	return (BOOL)bResult;
}

// 将按紧密字节排列的DPT_Byte转换为4个稀疏结构体DPT_Info以便于程序获取信息
void GetDPTInfo(DPT_Byte* src, DPT_Info* dest)
{
	for (int i = 0; i < 4; i++)
	{
		dest[i].ActivePartition = src->DPT_Table[i].ActivePartition == 0x80 ? true : false;//0x80是活动扇区，0x00是非活动扇区
		dest[i].StartMagneticHead = src->DPT_Table[i].StartInfo[0];
		dest[i].StartSector = src->DPT_Table[i].StartInfo[1] & 0x3F;
		dest[i].StartCylinder = src->DPT_Table[i].StartInfo[2] + ((src->DPT_Table[i].StartInfo[1] & 0xC0) << 2);
		dest[i].PartitionType = (Partition_Type)src->DPT_Table[i].PartitionType;
		dest[i].EndMagneticHead = src->DPT_Table[i].EndInfo[0];
		dest[i].EndSector = src->DPT_Table[i].EndInfo[1] & 0x3F;
		dest[i].EndCylinder = src->DPT_Table[i].EndInfo[2] + ((src->DPT_Table[i].EndInfo[1] & 0xC0) << 2);
		dest[i].UsedSector = ArrayToU32LittleEnd(src->DPT_Table[i].UsedSector);
		dest[i].TotalSector = ArrayToU32LittleEnd(src->DPT_Table[i].TotalSector);

		dest[i].MBROffset = dest[i].UsedSector;
	}
}

// 获取指定磁盘的磁盘分区表DPT
void GetDPT(HANDLE hDisk, DPT_Info* DPT)
{
	DPT_Byte DPTbuffer;//定义DPT扇区作为buffer，注意MCU的栈尺寸要大于512字节
	ReadSectorData(hDisk, 0, (uint8_t*)(void*)&DPTbuffer, sizeof(DPT_Byte));//载入DPT
	GetDPTInfo(&DPTbuffer, DPT);
}

// 根据DPT的分区类型判断磁盘分区表是GPT或MBR
int DPTDetermination(DPT_Info* DPT)
{
	switch (DPT->PartitionType)
	{
	case fsptNullType:
		return DPT_NULL;
	case fsptGPT:
		return DPT_GPT;
	case fsptExtended:
	case fsptWin95_Extended_8GB:
		return DPT_EBR;
	default:
		return DPT_MBR;
	}
}

// 获取一个MBR格式的磁盘中有效分卷数，暂时不带EBR，也就是说最多4个MBR
int GetPartitionNumberOfMBR(HANDLE hDisk, DPT_Info* DPT)
{
	DPT_Byte DPTsector;
	DPT_Info DPTinfo[4];
	uint64_t sectorsoffset = 0;
	ReadSectorData(hDisk, 0, (uint8_t*)(void*)&DPTsector, 512);//载入DPT
	GetDPTInfo(&DPTsector, DPTinfo);//小端模式转换扇区数据为分区信息
	int ValidPartitions = 0;
	for (int i = 0; i < 4; i++)
	{
		if (DPTDetermination(&(DPTinfo[i])) == DPT_NULL)
			break;
		else
			ValidPartitions++;
	}
	return ValidPartitions;
}

static inline GUID_Info ArrayToGUID(uint8_t* str)
{
	GUID_Info guid;
	guid.Part1 = ArrayToU32LittleEnd(&(str[0]));
	guid.Part2 = ArrayToU16LittleEnd(&(str[4]));
	guid.Part3 = ArrayToU16LittleEnd(&(str[6]));
	guid.Part4 = ArrayToU16BigEnd(&(str[8]));
	guid.Part5 = ArrayToU48BigEnd(&(str[10]));
	return guid;
}

// 将按紧密字节排列的PGPTH_Byte转换为稀疏结构体PGPTH_Info以便于程序获取信息
void GetPGPTHInfo(PGPTH_Byte* src, PGPTH_Info* dest)
{
	memcpy(dest->GPTVersion, src->GPTVersion, 4);
	dest->PrimaryGPTHeaderLength = ArrayToU32LittleEnd(src->PrimaryGPTHeaderLength);
	dest->PrimaryGPTHeaderCRC32 = ArrayToU32LittleEnd(src->PrimaryGPTHeaderCRC32);
	dest->PrimaryGPTHeaderStart = ArrayToU64LittleEnd(src->PrimaryGPTHeaderStart);
	dest->PrimaryGPTHeaderBackup = ArrayToU64LittleEnd(src->PrimaryGPTHeaderBackup);
	dest->GPTPartitionStart = ArrayToU64LittleEnd(src->GPTPartitionStart);
	dest->GPTPartitionEnd = ArrayToU64LittleEnd(src->GPTPartitionEnd);
	dest->DiskGUID = ArrayToGUID(src->DiskGUID);
	dest->PartitionStart = ArrayToU64LittleEnd(src->PartitionStart);
	dest->PartitionTables = ArrayToU32LittleEnd(src->PartitionTables);
	dest->BytesPerPartitionTable = ArrayToU32LittleEnd(src->BytesPerPartitionTable);
	dest->PartitionTableCRC32 = ArrayToU32LittleEnd(src->PartitionTableCRC32);
}

// 获取指定磁盘的主GPT头(Primary GPT Header,程序中简化为PGPTH)
void GetPGPTH(HANDLE hDisk, PGPTH_Info* PGPTH)

{
	PGPTH_Byte PGPTHbuffer;
	ReadSectorData(hDisk, 1, (uint8_t*)(void*)&PGPTHbuffer, sizeof(PGPTH_Byte));
	GetPGPTHInfo(&PGPTHbuffer, PGPTH);
}

// 读取磁盘数据，不可以打破扇区读取
BOOL ReadDiskData(HANDLE& hDevice, uint64_t redpos, uint8_t* lpOutBuffer, uint32_t ReadLen)
{
	memset(lpOutBuffer, 0, ReadLen);
	uint8_t buffer[SECTOR_SIZE];
	BOOL bReturn = ReadSectorData(hDevice, redpos / SECTOR_SIZE, buffer, SECTOR_SIZE);
	for (int i = 0; i < ReadLen; i++)
		lpOutBuffer[i] = buffer[i + (redpos % SECTOR_SIZE)];
	return bReturn;
}

// 将按紧密字节排列的GPTEntry_Byte转换为稀疏结构体GPTEntry_Info以便于程序获取信息
void GetGPTEInfo(GPTEntry_Byte* src, GPTEntry_Info* dest)
{
	for (int i = 0; i < 1; i++)
	{
		dest[i].TypeGUID = ArrayToGUID(src[i].TypeGUID);
		dest[i].UniqueGUID = ArrayToGUID(src[i].UniqueGUID);
		dest[i].SectorStart = ArrayToU64LittleEnd(src[i].SectorStart);
		dest[i].SectorEnd = ArrayToU64LittleEnd(src[i].SectorEnd);
		dest[i].PartitionAttrib = ArrayToU64BigEnd(src[i].PartitionAttrib);
		memcpy(dest[i].PartitionName, src[i].PartitionName, 72);
	}
}

// 判断两个GUID_Info是否相等
bool GUIDcmp(GUID_Info* guid1, GUID_Info* guid2)
{
	if (guid1->Part1 != guid2->Part1)
		return false;
	if (guid1->Part2 != guid2->Part2)
		return false;
	if (guid1->Part3 != guid2->Part3)
		return false;
	if (guid1->Part4 != guid2->Part4)
		return false;
	if (guid1->Part5 != guid2->Part5)
		return false;
	return true;
}

// 获取一个GPT格式的磁盘中有效分卷数
int GetPartitionNumberOfGPT(HANDLE hDisk, DPT_Info* DPT)
{
	if (DPTDetermination(DPT) == DPT_MBR)
		return -1;
	GPTEntry_Byte GPTEbuffer;
	GPTEntry_Info GPTEinfo;
	PGPTH_Info PGPTH;
	GetPGPTH(hDisk, &PGPTH);
	int ValidPartitions = 0;
	for (int i = 0; i < PGPTH.PartitionTables; i++)
	{
		ReadDiskData(
			hDisk, //读取磁盘句柄
			SECTOR_SIZE * PGPTH.PartitionStart + i * PGPTH.BytesPerPartitionTable, //计算读取的GPTE位置
			(uint8_t*)(void*)&GPTEbuffer, //缓冲区地址
			sizeof(GPTEntry_Byte)); //字节数
		GetGPTEInfo(&GPTEbuffer, &GPTEinfo);
		if (GUIDcmp(&(GPTEinfo.TypeGUID), (GUID_Info*)&GUID_ptUnuse))
			break;
		else
			ValidPartitions++;
	}
	return ValidPartitions;
}

// 获取磁盘分区数量， 不带EBR
int GetPartitionNumber(HANDLE hDisk)
{
	DPT_Info DPT[4];
	GetDPT(hDisk, DPT);
	if (DPTDetermination(DPT) == DPT_MBR)
		return GetPartitionNumberOfMBR(hDisk, DPT);
	else if (DPTDetermination(DPT) == DPT_GPT)
		return GetPartitionNumberOfGPT(hDisk, DPT);
	else
		return -1;
}

// 获取指定磁盘的指定LBA信息
void GetGPTE(HANDLE hDisk, PGPTH_Info* PGPTH, GPTEntry_Info* GPTE, uint8_t GPTESerial)
{
	GPTEntry_Byte GPTEbuffer;
	ReadDiskData(hDisk, PGPTH->PartitionStart * SECTOR_SIZE + PGPTH->BytesPerPartitionTable * GPTESerial, (uint8_t*)(void*)&GPTEbuffer, PGPTH->BytesPerPartitionTable);
	GetGPTEInfo(&GPTEbuffer, GPTE);
}

// 获取分卷信息，暂时不带EBR
int GetPartitions(HANDLE hDisk, Partition_Info* Partitions)
{
	DPT_Info DPT[4];
	GetDPT(hDisk, DPT);
	int iPartitions = GetPartitionNumber(hDisk);
	if (DPTDetermination(&(DPT[0])) == DPT_GPT)
	{
		PGPTH_Info PGPTH;
		GetPGPTH(hDisk, &PGPTH);
		GPTEntry_Info GPTE;
		for (int i = 0; i < iPartitions; i++)
		{
			GetGPTE(hDisk, &PGPTH, &GPTE, i);
			Partitions[i].Type = (Partition_Type)0;
			Partitions[i].SectorStart = GPTE.SectorStart;
			Partitions[i].SectorEnd = GPTE.SectorEnd;
		}
	}
	else if (DPTDetermination(&(DPT[0])) == DPT_MBR)
	{
		for (int i = 0; i < iPartitions; i++)
		{
			Partitions[i].Type = (Partition_Type)0;
			Partitions[i].SectorStart = DPT[i].UsedSector;
			Partitions[i].SectorEnd = DPT[i].UsedSector + DPT[i].TotalSector;
		}
	}

	return iPartitions;
}

// 显示分区列表
void ShowPartitions(Partition_Info* Partitions, int len)
{
	printf("----------------------------------------------------------------------------\n");
	printf("分区    类型      扇区总数      起始扇区      终止扇区              分区大小\n");
	printf("----------------------------------------------------------------------------\n");
	for (int i = 0; i < len; i++)
	{
		printf("%3d", i);
		printf("%8d", Partitions[i].Type);
		printf("%15llu", Partitions[i].SectorEnd - Partitions[i].SectorStart);
		printf("%14llu", Partitions[i].SectorStart);
		printf("%14llu", Partitions[i].SectorEnd);
		uint64_t PartitionSize = (Partitions[i].SectorEnd - Partitions[i].SectorStart) * 512;
		printf("%16llu Bytes\n", PartitionSize);
	}
}