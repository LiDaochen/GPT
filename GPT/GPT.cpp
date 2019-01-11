#include "ctrl.h"

int main()
{
	//*********************电脑自带硬盘信息******************************************//
	//获取磁盘句柄
	HANDLE hDisk = GetDiskHandle(0);
	//获取磁盘分区数量
	int iPartitions = GetPartitionNumber(hDisk);
	printf("磁盘0有效的分区数为%d个\n", iPartitions);
	//分配磁盘列表所需的内存空间
	Partition_Info* PartitionList = (Partition_Info*)malloc(sizeof(Partition_Info)*iPartitions);
	//获取分区列表
	GetPartitions(hDisk, PartitionList);
	//显示分区列表
	ShowPartitions(PartitionList, iPartitions);

	//*********************移动硬盘信息******************************************//
	//获取磁盘句柄
	HANDLE hDisk2 = GetDiskHandle(1);
	//获取磁盘分区数量
	int iPartitions2 = GetPartitionNumber(hDisk2);
	printf("\n磁盘1有效的分区数为%d个\n", iPartitions2);
	//分配磁盘列表所需的内存空间
	Partition_Info* PartitionList2 = (Partition_Info*)malloc(sizeof(Partition_Info)*iPartitions2);
	//获取分区列表
	GetPartitions(hDisk2, PartitionList2);
	//显示分区列表
	ShowPartitions(PartitionList2, iPartitions2);
	//释放磁盘列表
	free(PartitionList);
	free(PartitionList2);
	getchar();
	return 0;
}