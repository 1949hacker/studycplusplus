#include "fio.h"
#include <iostream>

using namespace std;

int main() {
  cout << "欢迎使用fio测试工具\n本工具测试内容:\n路径挂载模式下IOPS性能测试"
       << endl;

  // 设置测试参数
  setConfig();

  // 删除临时文件
  rm_file();

  // 进行随机写测试
  seq_write();

  // 删除临时文件
  // rm_file();

  // 输出存储的数据，模拟 Excel 风格
  runReport();

  return 0;
}
