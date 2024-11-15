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
int bwMin, bwMax, bwAvg, iopsMin, iopsMax, iopsAvg, bw[] = {0, 0, 0, 0, 0, 0},
                                                    iops[] = {0, 0, 0, 0, 0, 0};
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
    if (line.find("samples") != string::npos) {
      cout << "筛选成功，原始数据：" << line << endl;
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
  }
  // 转换为整数
  for (const string &s : bw_num) {
    bw_int.push_back(static_cast<int>(stof(s)));
  }
  for (const string &s : iops_num) {
    iops_int.push_back(static_cast<int>(stof(s)));
  }

  // DEBUG: 检查原始数据是否正常
  // for (int a : bw_int) {
  //   cout << a << endl;
  // }

  if (bw_int[6] != 0) {
    // 混合读写
    cout << name << " | 第" << i << "次带宽运行<读>结果:"
         << "min:" << bw_int[0] << " max:" << bw_int[1] << " avg:" << bw_int[3]
         << "\n"
         << name << " | 第" << i << "次次IOPS运行<读>结果:"
         << "min:" << iops_int[0] << " max:" << iops_int[1]
         << " avg:" << iops_int[3] << endl;
    cout << name << " | 第" << i << "次带宽运行<写>结果:"
         << "min:" << bw_int[6] << " max:" << bw_int[7] << " avg:" << bw_int[9]
         << "\n"
         << name << " | 第" << i << "次次IOPS运行<写>结果:"
         << "min:" << iops_int[5] << " max:" << iops_int[6]
         << " avg:" << iops_int[7] << endl;
    // 整理带宽和IOPS数据
    bw[0] += bw_int[0];
    bw[1] += bw_int[1];
    bw[2] += bw_int[3];
    bw[3] = bw_int[6];
    bw[4] += bw_int[7];
    bw[5] += bw_int[9];
    iops[0] += iops_int[0];
    iops[1] += iops_int[1];
    iops[2] += iops_int[2];
    iops[3] += iops_int[5];
    iops[4] += iops_int[6];
    iops[5] += iops_int[7];
  } else {
    cout << name << " | 第" << i << "次带宽运行结果:"
         << "min:" << bw_int[0] << " max:" << bw_int[1] << " avg:" << bw_int[3]
         << "\n"
         << name << " | 第" << i << "次次IOPS运行结果:"
         << "min:" << iops_int[0] << " max:" << iops_int[1]
         << " avg:" << iops_int[3] << endl;
    // 整理带宽和IOPS数据
    bw[0] += bw_int[0];
    bw[1] += bw_int[1];
    bw[2] += bw_int[3];
    iops[0] += iops_int[0];
    iops[1] += iops_int[1];
    iops[2] += iops_int[2];
  }
}

void fio_sum(const string &name) {

  if (bw[3] != 0) {
    // 针对混合读写处理
    //  计算最小、最大和平均值
    int RbwMin = bw[0] / 3;
    int RbwMax = bw[1] / 3;
    int RbwAvg = bw[2] / 3;
    int RiopsMin = iops[0] / 3;
    int RiopsMax = iops[1] / 3;
    int RiopsAvg = iops[2] / 3;
    int WbwMin = bw[3] / 3;
    int WbwMax = bw[4] / 3;
    int WbwAvg = bw[5] / 3;
    int WiopsMin = iops[3] / 3;
    int WiopsMax = iops[4] / 3;
    int WiopsAvg = iops[5] / 3;
    // 将结果存储到数据表中，第一列是 名称，后面是6个值
    values = {RbwMin, RbwMax, RbwAvg, RiopsMin, RiopsMax, RiopsAvg,
              WbwMin, WbwMax, WbwAvg, WiopsMin, WiopsMax, WiopsAvg};
    row = {name};
    for (int val : values) {
      row.push_back(to_string(val)); // 将每个值转换为字符串并添加到行中
    }
    run_report.push_back(row); // 将这行添加到数据中
  } else {
    // 计算最小、最大和平均值
    int bwMin = bw[0] / 3;
    int bwMax = bw[1] / 3;
    int bwAvg = bw[2] / 3;
    int iopsMin = iops[0] / 3;
    int iopsMax = iops[1] / 3;
    int iopsAvg = iops[2] / 3;
    // 将结果存储到数据表中，第一列是 名称，后面是6个值
    values = {bwMin, bwMax, bwAvg, iopsMin, iopsMax, iopsAvg};
    row = {name};
    for (int val : values) {
      row.push_back(to_string(val)); // 将每个值转换为字符串并添加到行中
    }
    run_report.push_back(row); // 将这行添加到数据中
  }

  // 重置数据
  fill(begin(bw), end(bw), 0);
  fill(begin(iops), end(iops), 0);
}

void runReport() {
  // 打开文件用于追加写入
  ofstream outputFile("/var/log/fio_tool/fio.csv", ios::app);

  if (outputFile.is_open()) {
    // 如果文件为空，先写入表头（假设表头只在文件为空时写入一次）
    if (outputFile.tellp() == 0) {
      outputFile << "测试类型,带宽最小值,带宽最大值,带宽均值,IOPS最大值,"
                    "IOPS最小值,IOPS均值,写带宽最小值,写带宽最大值,写带宽均值,"
                    "写IOPS最大值,"
                    "写IOPS最小值,写IOPS均值,"
                 << endl;
    }

    for (const auto &row : run_report) {
      for (const auto &cell : row) {
        outputFile << cell << ",";
      }
      outputFile << endl;
    }

    // 关闭文件
    outputFile.close();

    cout << "数据已成功追加到fio.csv文件。" << endl;
  } else {
    cerr << "无法打开fio.csv文件进行追加写入。" << endl;
  }
}

// --- 顺序写和读开始 ---
void fio_seq() {

  // 文件
  cout << "顺序写和读测试，共计200项，每项3次，每次" + runtime + "秒，共计" +
              to_string(stoi(runtime) * 200 * 3) + "秒，约" +
              to_string(stoi(runtime) * 200 * 3 / 60 / 60) + "小时\n进行中..."
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
            // 先写后读
            string rw_group[] = {"write", "read"};
            for (string rw : rw_group) {
              name = "seq_" + rw +
                     "_filename_numjobs=1_iodepth=" + to_string(iodepth) +
                     "_bs=" + to_string(bs) + "k";
              // 构建单文件fio命令
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
                // 重置数据
                bw_int.clear();
                iops_int.clear();
              }
              fio_sum(name);
              rm_file();
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
              // 先写后读
              string rw_group[] = {"write", "read"};
              for (string rw : rw_group) {
                name = "seq_" + rw + "_directory_numjobs=" + to_string(numjob) +
                       "_iodepth=" + to_string(iodepth) +
                       "_bs=" + to_string(bs) + "k";
                // 构建文件夹fio命令
                fio_cmd = "mkdir -p " + dir + to_string(iodepth) + "_" +
                          to_string(bs) + "k_" + to_string(i) +
                          "/&&fio -name=" + name + " -size=" + fsize +
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
                  // 重置数据
                  bw_int.clear();
                  iops_int.clear();
                }
                fio_sum(name);
                rm_file();
              }
            }
          }
        }
      }
    }
  }
}
// --- 顺序写和读结束 ---

// --- 随机写和读开始 ---
void fio_rand() {

  // 文件
  cout << "随机写和读测试，共计30项，每项3次，每次" + runtime + "秒，共计" +
              to_string(stoi(runtime) * 30 * 3) + "秒，约" +
              to_string(stoi(runtime) * 30 * 3 / 60 / 60) + "小时\n进行中..."
       << endl;

  // numjobs=1
  string DorF[] = {"filename=" + dir,
                   "directory=" + dir}; // 用数组配置单文件和文件夹
  for (string dorf : DorF) {
    for (int i = 0; i < 4; ++i) {
      if (dorf.find("file") != string::npos) {

        int bs_group[] = {4}; // 用数组配置bs块大小
        for (int bs : bs_group) {
          int iodepth_group[] = {1, 2, 8, 16, 32}; // 用数组配置iodepth循环
          for (int iodepth : iodepth_group) {
            // 先写后读
            string rw_group[] = {"randwrite", "randread"};
            for (string rw : rw_group) {
              name = "seq_" + rw +
                     "_filename_numjobs=1_iodepth=" + to_string(iodepth) +
                     "_bs=" + to_string(bs) + "k";
              // 构建单文件fio命令
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
                // 重置数据
                bw_int.clear();
                iops_int.clear();
              }
              fio_sum(name);
              rm_file();
            }
          }
        }
      } else if (dorf.find("directory") != string::npos) {

        int numjobs[] = {8, 16}; // 用数组配置numjobs
        for (int numjob : numjobs) {
          int bs_group[] = {4}; // 用数组配置bs块大小
          for (int bs : bs_group) {
            int iodepth_group[] = {1, 2, 8, 16, 32}; // 用数组配置iodepth循环
            for (int iodepth : iodepth_group) {
              // 先写后读
              string rw_group[] = {"randwrite", "randread"};
              for (string rw : rw_group) {
                name = "seq_" + rw + "_directory_numjobs=" + to_string(numjob) +
                       "_iodepth=" + to_string(iodepth) +
                       "_bs=" + to_string(bs) + "k";
                // 构建文件夹fio命令
                fio_cmd = "mkdir -p " + dir + to_string(iodepth) + "_" +
                          to_string(bs) + "k_" + to_string(i) +
                          "/&&fio -name=" + name + " -size=" + fsize +
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
                  // 重置数据
                  bw_int.clear();
                  iops_int.clear();
                }
                fio_sum(name);
                rm_file();
              }
            }
          }
        }
      }
    }
  }
}
// --- 随机写和读结束 ---

// --- 随机读写开始 ---
void fio_randrw() {

  // 文件
  cout << "50%随机读写测试，共计15项，每项3次，每次" + runtime + "秒，共计" +
              to_string(stoi(runtime) * 30 * 3) + "秒，约" +
              to_string(stoi(runtime) * 30 * 3 / 60 / 60) + "小时\n进行中..."
       << endl;

  // numjobs=1
  string DorF[] = {"filename=" + dir,
                   "directory=" + dir}; // 用数组配置单文件和文件夹
  for (string dorf : DorF) {
    for (int i = 0; i < 4; ++i) {
      if (dorf.find("file") != string::npos) {

        int bs_group[] = {4}; // 用数组配置bs块大小
        for (int bs : bs_group) {
          int iodepth_group[] = {1, 2, 8, 16, 32}; // 用数组配置iodepth循环
          for (int iodepth : iodepth_group) {
            // 先写后读
            string rw_group[] = {"randrw"};
            for (string rw : rw_group) {
              name = "seq_" + rw +
                     "_filename_numjobs=1_iodepth=" + to_string(iodepth) +
                     "_bs=" + to_string(bs) + "k";
              // 构建单文件fio命令
              fio_cmd = "fio -name=" + name + " -size=" + fsize +
                        "G -runtime=" + runtime +
                        "s -time_base -bs=" + to_string(bs) + "k" +
                        +" -direct=" + direct + " -rw=" + rw +
                        " -ioengine=" + ioengine + " -numjobs=1" +
                        " -group_reporting -iodepth=" + to_string(iodepth) +
                        " -" + dorf + to_string(iodepth) + "_" + to_string(bs) +
                        "k/" + to_string(i) + " -randrepeat=0" +
                        " -rwmixwrite=50";

              // 重复运行3次
              for (int i = 1; i <= 3; i++) {
                // 输出本次运行的命令以便排障
                cout << i << "次运行的命令是：" << fio_cmd << endl;
                run_cmd(fio_cmd);

                string read_or_write[] = {"read", "write"};
                format(i);
                // 重置数据
                bw_int.clear();
                iops_int.clear();
              }
              fio_sum(name);
              rm_file();
            }
          }
        }
      } else if (dorf.find("directory") != string::npos) {

        int numjobs[] = {8, 16}; // 用数组配置numjobs
        for (int numjob : numjobs) {
          int bs_group[] = {4}; // 用数组配置bs块大小
          for (int bs : bs_group) {
            int iodepth_group[] = {1, 2, 8, 16, 32}; // 用数组配置iodepth循环
            for (int iodepth : iodepth_group) {
              // 先写后读
              string rw_group[] = {"randrw"};
              for (string rw : rw_group) {
                name = "seq_" + rw + "_directory_numjobs=" + to_string(numjob) +
                       "_iodepth=" + to_string(iodepth) +
                       "_bs=" + to_string(bs) + "k";
                // 构建文件夹fio命令
                fio_cmd = "mkdir -p " + dir + to_string(iodepth) + "_" +
                          to_string(bs) + "k_" + to_string(i) +
                          "/&&fio -name=" + name + " -size=" + fsize +
                          "G -runtime=" + runtime +
                          "s -time_base -bs=1m -direct=" + direct +
                          " -rw=" + rw + " -ioengine=" + ioengine +
                          " -numjobs=" + to_string(numjob) +
                          " -group_reporting -iodepth=" + to_string(iodepth) +
                          " -" + dorf + to_string(iodepth) + "_" +
                          to_string(bs) + "k_" + to_string(i) +
                          "/ -randrepeat=0" + " -rwmixwrite=50";

                // 重复运行3次
                for (int i = 1; i <= 3; i++) {
                  // 输出本次运行的命令以便排障
                  cout << i << "本次运行的命令是：" << fio_cmd << endl;
                  run_cmd(fio_cmd);
                  format(i);
                  // 重置数据
                  bw_int.clear();
                  iops_int.clear();
                }
                fio_sum(name);
                rm_file();
              }
            }
          }
        }
      }
    }
  }
}
// --- 随机写和读结束 ---