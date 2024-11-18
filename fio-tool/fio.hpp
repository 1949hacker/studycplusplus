// fio.hpp
#ifndef FIO_HPP
#define FIO_HPP

// 设置测试参数
void setConfig();

// 创建预读文件
void init_read();

// 顺序写
void fio_seq_write();

// 顺序读
void fio_seq_read();

// 随机写
void fio_rand_write();

// 随机读
void fio_rand_read();

// 随机读写测试
void fio_randrw();

// 输出结果
void runReport();

#endif // FIO_HPP