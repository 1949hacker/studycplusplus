#include "fio.hpp"
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
  strftime(timeStr, sizeof(timeStr), "%Y-%m-%d-%H-%M-%S", localTime);
  string fileName = "/var/log/fio_tool/" + string(timeStr) + ".log";

  // 创建日志文件流
  ofstream logFile(fileName, ios::app);
  if (!logFile) {
    cerr << "无法创建日志文件！" << endl;
    return 1;
  } else {
    cout << "成功创建日志文件：" << fileName << endl;
  }

  // 重定向 cout 输出到日志文件和控制台
  TeeBuf teeBuf(cout.rdbuf(), logFile.rdbuf());
  streambuf *originalCoutBuf = cout.rdbuf(&teeBuf);

  cout << "欢迎使用fio测试工具\n日志和输出的fio."
          "csv默认保存到/var/log/fio_tool/"
          "\n日志不会自行清除，运行前请自行清理日志文件和检查是否残留有测试文件"
          "\n输出的数据统一为KiB/s单位，iops无单位\n请<按键>"
          "选择你的测试内容：\n"
       << "1. 顺序写测试\n"
       << "2. 随机写测试\n\n\n"
       << "-------分割线-------\n"
       << "进行读测试之前需要先创建预读文件，固定为测试路径下的init_read.[0-15]"
       << ".0共16个文件\n"
       << "请自行根据测试情况判断是否需要重新生成，如需重新生成，请删除后按<r>"
       << "进行预读文件创建\n"
       << "如无需创建预读文件则直接按键开始测试即可！\n\n"
       << "测试完成后记得清理干净测试残留的文件和日志文件\n"
       << "-------分割线-------\n\n\n"
       << "3. 顺序读测试\n"
       << "4. 随机读测试\n"
       << "5. 4k随机50%混合读写测试\n"
       << "r. 创建预读文件\n"
       << "f. Fullauto全自动测试"
       << "q. 退出程序\n"
       << "s. 你想骚一下？" << endl;

  char choice;
  while (true) {
    choice = getch(); // Linux/macOS 下使用自定义 getch()

    cout << choice << endl; // 打印选项
    switch (choice) {
    case '1':
      setConfig();
      fio_seq_write();
      return 0;
    case '2':
      setConfig();
      fio_rand_write();
      return 0;
    case '3':
      setConfig();
      fio_seq_read();
      return 0;
    case '4':
      setConfig();
      fio_rand_read();
      return 0;
    case '5':
      setConfig();
      fio_randrw();
      return 0;
    case 'r':
      setConfig();
      init_read();
      return 0;
    case 'f':
      setConfig();
      fio_seq_write();
      fio_rand_write();
      init_read();
      fio_seq_read();
      fio_rand_read();
      fio_randrw();
      return 0;
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
