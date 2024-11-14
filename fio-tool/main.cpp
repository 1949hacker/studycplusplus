#include "fio.h"
#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>
#include <streambuf>
#include <termios.h>
#include <unistd.h>

// Linux/macOS 下的 getch() 实现
int getch() {
  struct termios oldt, newt;
  int ch;
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  ch = getchar();
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  return ch;
}

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

  // 日志目录初始化
  int result = system("mkdir -p /var/log/fio_tool");

  if (result == 0) {
    std::cout << "日志存储目录 /var/log/fio_tool 已成功创建。" << std::endl;
  } else {
    std::cerr
        << "创建目录 /var/log/fio_tool 时出现错误。请检查当前用户的运行权限！"
        << std::endl;
    return 0;
  }

  // 获取时间并格式化文件名
  time_t log_time = time(nullptr);
  tm *localTime = localtime(&log_time);
  char timeStr[20];
  strftime(timeStr, sizeof(timeStr), "/var/log/fio_tool/%Y-%m-%d-%H-%M-%S",
           localTime);
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

  cout << "欢迎使用fio测试工具\n日志和输出的fio."
          "csv默认保存到/var/log/fio_tool/\n请<按键>"
          "选择你的测试内容：\n"
       << "1. 顺序写和读测试\n"
       << "2. 随机写和读测试\n"
       << "3. 4k随机50%混合读写测试\n"
       << "q. 退出程序\n"
       << "s. 你想骚一下？" << endl;

  char choice;
  while (true) {
    choice = getch(); // Linux/macOS 下使用自定义 getch()

    cout << choice << endl; // 打印选项
    switch (choice) {
    case '1':
      setConfig();
      fio_seq();
      rm_file();
      runReport();
      break;
    case '2':
      setConfig();
      fio_rand();
      rm_file();
      runReport();
      break;
    case '3':
      setConfig();
      fio_randrw();
      rm_file();
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
