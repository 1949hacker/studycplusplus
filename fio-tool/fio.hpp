// fio.hpp
#ifndef FIO_HPP
#define FIO_HPP

// 设置测试参数
void setConfig();

// 顺序写和读
void fio_seq();

// 随机写和读
void fio_rand();

// 随机读写测试
void fio_randrw();

// 删除测试文件
void rm_file();

// 输出结果
void runReport();

#endif // FIO_HPP