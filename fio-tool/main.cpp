#include "fio.h"
#include <conio.h> // Windows 下的 _getch()
#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>
#include <streambuf>

using namespace std;

// 自定义流缓冲区类，将输出内容同时写入控制台和日志文件
class TeeBuf : public streambuf {
public:
  TeeBuf(streambuf *consoleBuf, streambuf *fileBuf)
      : consoleBuf(consoleBuf), fileBuf(fileBuf) {}

protected:
  virtual int overflow(int c) override {
    if (c == EOF) {
      return !EOF;
    } else {
      if (consoleBuf->sputc(c) == EOF)
        return EOF;
      if (fileBuf->sputc(c) == EOF)
        return EOF;
      return c;
    }
  }

  virtual int sync() override {
    if (consoleBuf->pubsync() == -1)
      return -1;
    if (fileBuf->pubsync() == -1)
      return -1;
    return 0;
  }

private:
  streambuf *consoleBuf;
  streambuf *fileBuf;
};

int main() {
  // 获取时间并格式化文件名
  time_t log_time = time(nullptr);
  tm *localTime = localtime(&log_time);
  char timeStr[20];
  strftime(timeStr, sizeof(timeStr), "%Y-%m-%d-%H-%M-%S", localTime);
  string fileName = string(timeStr) + ".log";

  // 创建日志文件流
  ofstream logFile(fileName, ios::app);
  if (!logFile) {
    cerr << "无法创建日志文件！" << endl;
    return 1;
  }

  // 重定向 cout 输出到日志文件和控制台
  TeeBuf teeBuf(cout.rdbuf(), logFile.rdbuf());
  streambuf *originalCoutBuf = cout.rdbuf(&teeBuf);

  cout << "欢迎使用fio测试工具\n请<按键>选择你的测试内容：\n"
       << "1. 顺序写和读测试\n"
       << "2. 随机写和读测试\n"
       << "3. 4k随机50%混合读写测试\n"
       << "q. 退出程序\n"
       << "s. 你想骚一下？" << endl;

  char choice;
  while (true) {
    choice = _getch(); // 获取按键，不需要按回车

    cout << choice << endl; // 打印选项
    switch (choice) {
    case '1':
      // 设置测试参数
      setConfig();
      // 顺序写和读测试
      fio_seq();
      // 删除临时文件
      rm_file();
      // 输出存储的数据，模拟 Excel 风格
      runReport();
      break;
    case '2':
      // 设置测试参数
      setConfig();
      // 随机写和读测试
      fio_rand();
      // 删除临时文件
      rm_file();
      // 输出存储的数据，模拟 Excel 风格
      runReport();
      break;
    case '3':
      // 设置测试参数
      setConfig();
      // 随机读写测试
      fio_randrw();
      // 删除临时文件
      rm_file();
      // 输出存储的数据，模拟 Excel 风格
      runReport();
      break;
    case 'q':
      cout << "程序已退出。" << endl;
      return 0;
    case 's':
      while (true) {
        cout << "骚个鸡儿啊，正事儿不干？我不懒得写炸机代码？" << endl;
      }
      return 0;
    default:
      cout << "无效选项，请重新输入。" << endl;
    }
  }
  // 恢复 cout 的原始缓冲区
  cout.rdbuf(originalCoutBuf);

  return 0;
}
