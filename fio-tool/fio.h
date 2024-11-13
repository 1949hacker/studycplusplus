// fio.h
#ifndef FIO
#define FIO

// 设置测试参数
void setConfig();

// 进行随机写测试
void randwrite_4k();

// 删除测试文件
void rm_file();

// 输出结果
void runReport();

#endif // FIO