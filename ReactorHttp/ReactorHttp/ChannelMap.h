#pragma once
#include <stdbool.h>
struct ChannelMap
{
    int size;   // 记录指针指向的数组的元素总个数
    // struct Channel* list[];
    struct Channel** list;
};

// 初始化
struct ChannelMap* channelMapInit(int size);
// 清空map
void ChannelMapClear(struct ChannelMap* map);
// 重新分配内存空间
bool makeMapRoom(struct ChannelMap* map, int newSize, int unitSize);