#include <stdint.h>
typedef unsigned __int64 uint48_t;

//**********************GPT相关结构体******************************//
// LBA1即GPT头结构体
typedef struct
{
	uint8_t EFIPARTSign[8];			   //固定字符串“EFI PART”
	uint8_t GPTVersion[4];			   //GPT版本号
	uint8_t PrimaryGPTHeaderLength[4]; //主GPT头大小
	uint8_t PrimaryGPTHeaderCRC32[4];  //主GPT头的CRC32校验数据
	uint8_t Reserve1[4];			   //保留
	uint8_t PrimaryGPTHeaderStart[8];  //主GPT头起始扇区号
	uint8_t PrimaryGPTHeaderBackup[8]; //主GPT头备份位置扇区号
	uint8_t GPTPartitionStart[8];	   //GPT分区起始扇区号
	uint8_t GPTPartitionEnd[8];		   //GPT分区结束扇区号
	uint8_t DiskGUID[16];			   //磁盘的GUID
	uint8_t PartitionStart[8];		   //分区表起始扇区号
	uint8_t PartitionTables[4];		   //分区表项数目
	uint8_t BytesPerPartitionTable[4]; //每个分区表占用字节数
	uint8_t PartitionTableCRC32[4];	   //分区表CRC校验数据
	uint8_t Reserve2[420];			   //保留
}PGPTH_Byte;

typedef struct
{
	uint32_t Part1;					 //GUID第1部分
	uint16_t Part2;					 //GUID第2部分
	uint16_t Part3;					 //GUID第3部分
	uint16_t Part4;					 //GUID第4部分
	uint48_t Part5;					 //GUID第5部分
}GUID_Info;

// LBA1(GPT头结构体)（小端模式转换为大端模式之后的结构体）
typedef struct
{
	uint8_t   GPTVersion[4];          //GPT版本号
	uint32_t  PrimaryGPTHeaderLength; //主GPT头大小
	uint32_t  PrimaryGPTHeaderCRC32;  //主GPT头的CRC32校验数据
	uint64_t  PrimaryGPTHeaderStart;  //主GPT头起始扇区号
	uint64_t  PrimaryGPTHeaderBackup; //主GPT头备份位置扇区号
	uint64_t  GPTPartitionStart;      //GPT分区起始扇区号
	uint64_t  GPTPartitionEnd;        //GPT分区结束扇区号
	GUID_Info DiskGUID;				  //磁盘的GUID
	uint64_t  PartitionStart;         //分区表起始扇区号
	uint32_t  PartitionTables;        //分区表项数目
	uint32_t  BytesPerPartitionTable; //每个分区表占用字节数
	uint32_t  PartitionTableCRC32;    //分区表CRC校验数据
}PGPTH_Info;
//********************************************************************//

//**********************GPTEntry相关结构体******************************//
typedef struct
{
	uint8_t TypeGUID[16];           //用GUID表示的分区类型
	uint8_t UniqueGUID[16];			//用GUID表示的分区唯一标示符
	uint8_t SectorStart[8];			//该分区的起始扇区，用LBA值表示
	uint8_t SectorEnd[8];			//该分区的结束扇区(包含)，用LBA值表示，通常是奇数
	uint8_t PartitionAttrib[8];		//该分区的属性标志
	WCHAR   PartitionName[36];		//UTF-16LE编码的人类可读的分区名称，最大32个字符。
}GPTEntry_Byte;

typedef struct
{
	GUID_Info TypeGUID;				//用GUID表示的分区类型
	GUID_Info UniqueGUID;			//用GUID表示的分区唯一标示符
	uint64_t  SectorStart;			//该分区的起始扇区，用LBA值表示
	uint64_t  SectorEnd;			//该分区的结束扇区(包含)，用LBA值表示，通常是奇数
	uint64_t  PartitionAttrib;		//该分区的属性标志
	WCHAR     PartitionName[36];	//UTF-16LE编码的人类可读的分区名称，最大32个字符。
}GPTEntry_Info;
//********************************************************************//

//**********************Partition相关结构体***************************//
// 磁盘分区类型结构体
typedef enum
{
	fsptNullType = 0x00,
	fsptFAT32 = 0x01,
	fsptXENIX__root = 0X02,
	fsptXENIX_usr = 0X03,
	fsptFAT16_32M = 0X04,
	fsptExtended = 0X05,
	fsptFAT16 = 0X06,
	fsptHPFS_NTFS = 0X07,
	fsptAIX = 0X08,
	fsptAIX_bootable = 0X09,
	fsptOS_2_Boot_Manage = 0X0A,
	fsptWin95_FAT32 = 0X0B,
	fsptWin95_Fat32 = 0X0C,
	fsptWin95_FAT16 = 0X0E,
	fsptWin95_Extended_8GB = 0X0F,
	fsptOPUS = 0X10,
	fsptHidden_FAT12 = 0X11,
	fsptCompaq_diagnost = 0X12,
	fsptHidden_FAT16 = 0X16,
	fsptHidden_FAT16_32GB = 0X14,
	fsptHidden_HPFS_NTFS = 0X17,
	fsptAST_Windows_swap = 0X18,
	fsptHidden_FAT32 = 0X1B,
	fsptHidden_FAT32_partition = 0X1C,
	fsptHidden_LBA_VFAT_partition = 0X1E,
	fsptNEC_DOS = 0X24,
	fsptPartition_Magic = 0X3C,
	fsptVenix_80286 = 0X40,
	fsptPPC_PreP_Boot = 0X41,
	fsptSFS = 0X42,
	fsptQNX4_x = 0X4D,
	fsptQNX4_x_2nd_part = 0X4E,
	fsptQNX4_x_3rd_part = 0X4F,
	fsptOntrack_DM = 0X50,
	fsptOntrack_DM6_Aux = 0X51,
	fsptCP_M = 0X52,
	fsptOnTrack_DM6_AUX = 0X53,
	fsptOnTrack_DM6 = 0X54,
	fsptEZ_Drive = 0X55,
	fsptGolden_Bow = 0X56,
	fsptPriam_Edisk = 0X5C,
	fsptSpeed_Stor = 0X61,
	fsptGNU_HURD_or_Sys = 0X63,
	fsptNovell_Netware = 0X64,
	fsptNovell_NetWare = 0X65,
	fsptDisk_Secure_Mult = 0X70,
	fsptPC_IX = 0X75,
	fsptOld_Minix = 0X80,
	fsptMinix_Old_Linux = 0X81,
	fsptLinux_swap = 0X82,
	fsptLinux = 0X83,
	fsptOS_2_hidden_C = 0X84,
	fsptLinux_extended = 0X85,
	fsptNTFS_volume_set = 0X86,
	fsptNTFS_Volume_Set = 0X87,
	fsptAmoeba = 0X93,
	fsptAmoeba_BBT = 0X94,
	fsptIBM_Thinkpad_hidden = 0XA0,
	fsptBSD_386 = 0XA5,
	fsptOpen_BSD = 0XA6,
	fsptNextSTEP = 0XA7,
	fsptBSDI_fs = 0XB7,
	fsptBSDI_swap = 0XB8,
	fsptSolaris_boot_partition = 0XBE,
	fsptDRDOS_NovellDOS_secured_Partition = 0XC0,
	fsptDRDOS_sec = 0XC1,
	fsptDRDOS_Sec = 0XC4,
	fsptDRDOS_SEC = 0XC6,
	fsptSyrinx = 0XC7,
	fsptCP_M_CTOS = 0XDB,
	fsptDOS_access = 0XE1,
	fsptDOS_R_O = 0XE3,
	fsptSpeedStor = 0XE4,
	fsptBeOS_fs = 0XEB,
	fsptGPT = 0XEE,
	fsptSpeedstor = 0XF1,
	fsptDOS3_3_secondary_partition = 0XF2,
	fsptSpeed_stor = 0XF4,
	fsptLAN_step = 0XFE,
	fsptBBT = 0XFF
}Partition_Type;

// 磁盘分区信息结构体
typedef struct
{
	Partition_Type Type;        //分区类型
	uint64_t       SectorStart; //起始扇区
	uint64_t       SectorEnd;   //终止扇区
}Partition_Info;
//********************************************************************//

//**********************DPT相关结构体********************************//
typedef struct
{
	uint8_t ActivePartition;    //活动分区
	uint8_t StartInfo[3];       //本分区的起始磁头号、扇区号、柱面号
	uint8_t PartitionType;      //分区类型
	uint8_t EndInfo[3];         //本分区的结束磁头号、扇区号、柱面号
	uint8_t UsedSector[4];      //本分区前已使用的扇区数
	uint8_t TotalSector[4];     //本分区的总扇区数
}DPT_Type;

typedef struct
{
	uint8_t  BootCode[446];     //前面446字节
	DPT_Type DPT_Table[4];
	uint8_t  EndSign[2];        //结束标记
}DPT_Byte;

// 分区表DPT结构体
typedef struct
{
	bool           ActivePartition;      //活动分区
	uint8_t        StartMagneticHead;    //起始磁头号
	uint8_t        StartSector;          //起始扇区号
	uint16_t       StartCylinder;        //起始柱面号
	Partition_Type PartitionType;        //分区类型
	uint8_t        EndMagneticHead;      //结束磁头号
	uint8_t        EndSector;            //结束扇区号
	uint16_t       EndCylinder;          //结束柱面号
	uint32_t       UsedSector;           //本分区前已使用的扇区数
	uint32_t       TotalSector;          //本分区的总扇区数
	uint32_t       MBROffset;            //本分区中MBR偏移量
}DPT_Info;
//********************************************************************//

static const GUID_Info GUID_ptUnuse = { 0x00000000, 0x0000, 0x0000, 0x0000, 0x000000000000 }; //未使用