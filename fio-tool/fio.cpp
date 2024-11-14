#include <array>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

// 全局参数
int bwMin, bwMax, bwAvg, iopsMin, iopsMax, iopsAvg, bw[] = {0, 0, 0},
                                                    iops[] = {0, 0, 0};
string dir, fsize, ioengine, name, fio_cmd, runtime, direct, line;
stringstream fio_output;
vector<vector<string>> run_report;
vector<string> row;
vector<int> values, bw_int, iops_int;

// 参数设置
void setConfig() {
  cout << "测试路径（完整输入，带/结尾，如/mnt/iotest/）：";
  cin >> dir;
  cout << "测试文件大小，仅输入数字单位为G，size=";
  cin >> fsize;
  cout << "运行时长，仅输入数字单位为秒，runtime=";
  cin >> runtime;
  cout << "io测试引擎，ioengine=";
  cin >> ioengine;
  cout << "设置是否经过系统缓存，1不缓存，0操作系统缓存，direct=";
  cin >> direct;
}

// 删除测试文件
void rm_file() {
  string rm_command = "rm -rf " + dir + "*";
  system(rm_command.c_str()); // 删除 /iopsTest 目录下的所有文件
  cout << "临时文件已删除" << endl;
}

void run_cmd(const string &cmd) {
  // 重置变量
  fio_output.str("");
  fio_output.clear();

  FILE *fp = popen(cmd.c_str(), "r");
  if (fp == nullptr) {
    cerr << "Error opening pipe!" << endl;
    return;
  }

  // 从fio输出中获取数据
  char buffer[128];
  while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
    fio_output << buffer;
  }
  fclose(fp);

  // DEBUG:显示fio的输出内容
  //   cout << fio_output.str();
}

// 分析fio输出
void format(const int &i) {

  vector<string> bw_num, iops_num;

  while (getline(fio_output, line)) {
    if (line.find("bw ") != string::npos) {
      if (line.find("MiB") != string::npos) {
        cout << "检测到单位MiB/s，将转换为KiB/s" << endl;
        // 提取带宽数字
        regex bw_regex(R"(\d+\.\d+|\d+)");
        smatch match;
        while (regex_search(line, match, bw_regex)) {
          // 检测到单位是MiB，则转换为KiB
          float bw_value_kib = stof(match.str());
          bw_value_kib *= 1024;
          int bw_value_bytes = static_cast<int>(bw_value_kib); // 转为整数
          bw_num.push_back(to_string(bw_value_bytes));
          line = match.suffix();
        }
      } else if (line.find("KiB") != string::npos) {
        cout << "检测到单位KiB/s，直接提取" << endl;
        // 提取带宽数字
        regex bw_regex(R"(\d+\.\d+|\d+)");
        smatch match;
        while (regex_search(line, match, bw_regex)) {
          bw_num.push_back(match.str());
          line = match.suffix();
        }
      }
    }
    if (line.find("iops") != string::npos) {
      // 提取IOPS数字
      regex iops_regex(R"(\d+\.\d+|\d+)");
      smatch match;
      while (regex_search(line, match, iops_regex)) {
        iops_num.push_back(match.str());
        line = match.suffix();
      }
    }
  }
  // 转换为整数
  for (const string &s : bw_num) {
    bw_int.push_back(static_cast<int>(stof(s)));
  }
  for (const string &s : iops_num) {
    iops_int.push_back(static_cast<int>(stof(s)));
  }

  // DEBUG: 检查原始数据是否正常
  //   for (int a : bw_int) {
  //     cout << a << endl;
  //   }

  cout << name << " | 第" << i << "次带宽运行结果:"
       << "min:" << bw_int[0] << " max:" << bw_int[1] << " avg:" << bw_int[3]
       << "\n"
       << name << " | 第" << i << "次次IOPS运行结果:"
       << "min:" << iops_int[0] << " max:" << iops_int[1]
       << " avg:" << iops_int[3] << endl;
  // 整理带宽和IOPS数据
  bw[0] = bw_int[0];
  bw[1] += bw_int[1];
  bw[2] += bw_int[3];
  iops[0] += iops_int[0];
  iops[1] += iops_int[1];
  iops[2] += iops_int[2];
  // 重置数据
  bw_int.clear();
  iops_int.clear();
}

void fio_sum(const string &name) {

  // 计算最小、最大和平均值
  int bwMin = bw[0] / 3;
  int bwMax = bw[1] / 3;
  int bwAvg = bw[2] / 3;
  int iopsMin = iops[0] / 3;
  int iopsMax = iops[1] / 3;
  int iopsAvg = iops[2] / 3;
  // 将结果存储到数据表中，第一列是 randwrite_4k，后面是6个值
  values = {bwMin, bwMax, bwAvg, iopsMin, iopsMax, iopsAvg};
  row = {name};
  for (int val : values) {
    row.push_back(to_string(val)); // 将每个值转换为字符串并添加到行中
  }
  run_report.push_back(row); // 将这行添加到数据中}
  // 重置数据
  fill(begin(bw), end(bw), 0);
  fill(begin(iops), end(iops), 0);
}

void runReport() {
  // 输出存储的数据，模拟 Excel 风格
  cout << "测试类型\t带宽最小值\t带宽最大值\t带宽均值\tIOPS最大值\tIOPS最小"
          "值\t"
          "IOPS均值"
       << endl;
  for (const auto &row : run_report) {
    for (const auto &cell : row) {
      cout << cell << "\t";
    }
    cout << endl;
  }
}

// --- 顺序写和读开始 ---
void seq() {

  // 文件
  cout << "顺序写和读测试，共计200项，每项" + runtime + "秒，共计" +
              to_string(stoi(runtime) * 200) + "秒，约" +
              to_string(stoi(runtime) * 200 / 60 / 60) + "小时\n进行中..."
       << endl;

  // numjobs=1
  string DorF[] = {"filename=" + dir,
                   "directory=" + dir}; // 用数组配置单文件和文件夹
  for (string dorf : DorF) {
    for (int i = 0; i < 4; ++i) {
      if (dorf.find("file") != string::npos) {

        int bs_group[] = {512, 1024}; // 用数组配置bs块大小
        for (int bs : bs_group) {
          int iodepth_group[] = {1, 2, 8, 16, 32}; // 用数组配置iodepth循环
          for (int iodepth : iodepth_group) {
            name = "seq_read_filename_numjobs=1_iodepth=" + to_string(iodepth) +
                   "_bs=" + to_string(bs) + "k";

            // 先写后读
            string rw_group[] = {"write", "read"};
            for (string rw : rw_group) {
              // 构建fio命令
              fio_cmd = "fio -name=" + name + " -size=" + fsize +
                        "G -runtime=" + runtime +
                        "s -time_base -bs=" + to_string(bs) + "k" +
                        +" -direct=" + direct + " -rw=" + rw +
                        " -ioengine=" + ioengine + " -numjobs=1" +
                        " -group_reporting -iodepth=" + to_string(iodepth) +
                        " -" + dorf + to_string(iodepth) + "_" + to_string(bs) +
                        "k/" + to_string(i) + " -randrepeat=0";

              // 重复运行3次
              for (int i = 1; i <= 3; i++) {
                // 输出本次运行的命令以便排障
                cout << i << "次运行的命令是：" << fio_cmd << endl;
                run_cmd(fio_cmd);
                format(i);
              }
              fio_sum(name);
            }
          }
        }
      } else if (dorf.find("directory") != string::npos) {

        int numjobs[] = {8, 16}; // 用数组配置numjobs
        for (int numjob : numjobs) {
          int bs_group[] = {128, 256, 512, 1024}; // 用数组配置bs块大小
          for (int bs : bs_group) {

            int iodepth_group[] = {1, 2, 8, 16, 32}; // 用数组配置iodepth循环
            for (int iodepth : iodepth_group) {
              name = "seq_read_directory_numjobs=" + to_string(numjob) +
                     "_iodepth=" + to_string(iodepth) + "_bs=" + to_string(bs) +
                     "k";

              // 先写后读
              string rw_group[] = {"write", "read"};
              for (string rw : rw_group) {
                // 构建fio命令
                fio_cmd = "fio -name=" + name + " -size=" + fsize +
                          "G -runtime=" + runtime +
                          "s -time_base -bs=1m -direct=" + direct +
                          " -rw=" + rw + " -ioengine=" + ioengine +
                          " -numjobs=" + to_string(numjob) +
                          " -group_reporting -iodepth=" + to_string(iodepth) +
                          " -" + dorf + to_string(iodepth) + "_" +
                          to_string(bs) + "k_" + to_string(i) +
                          "/ -randrepeat=0";

                // 重复运行3次
                for (int i = 1; i <= 3; i++) {
                  // 输出本次运行的命令以便排障
                  cout << i << "本次运行的命令是：" << fio_cmd << endl;
                  run_cmd(fio_cmd);
                  format(i);
                }
                fio_sum(name);
              }
            }
          }
        }
      }
    }
  }
}
// --- 顺序写结束 ---