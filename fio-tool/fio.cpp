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
string dir, fsize, runtime, direct, ioengine, numjobs, iodepth, name, cmd;
vector<vector<string>> run_report;

// 参数设置
void setConfig() {
  cout << "测试路径（完整输入，带/结尾，如/mnt/iotest/）：";
  cin >> dir;
  cout << "测试文件大小，单位为G，size=";
  cin >> fsize;
  cout << "运行时长（秒s 分m 时h），例如30s，runtime=";
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

// --- 顺序写开始 ---
void seq_write() {

  // 文件
  cout << "顺序写测试进行中..." << endl;

  // numjobs=1
  string DorF[] = {"filename=" + dir,
                   "directory=" + dir}; // 用数组配置单文件和文件夹
  for (string dorf : DorF) {

    int bs_group[] = {512, 1024}; // 用数组配置bs块大小
    for (int bs : bs_group) {

      int iodepth_group[] = {1, 2, 8, 16}; // 用数组配置iodepth循环
      for (int iodepth : iodepth_group) {

        vector<int> bw = {0, 0, 0};
        vector<int> iops = {0, 0, 0};
        for (int i = 0; i < 4; ++i) {

          if (dorf.find("file") != string::npos) {
            name = "seq_read_filename_numjobs=1_iodepth=" + to_string(iodepth) +
                   "_bs=" + to_string(bs) + "k";
            // 构建fio命令
            cmd = "fio -name=" + name + " -size=" + fsize +
                  " -runtime=" + runtime + " -time_base -bs=" + to_string(bs) +
                  "k" + +" -direct=" + direct +
                  " -rw=write "
                  "-ioengine=" +
                  ioengine + " -numjobs=1" +
                  " -group_reporting -iodepth=" + to_string(iodepth) + " -" +
                  dorf + to_string(iodepth) + to_string(bs) + "k" + +"/" +
                  to_string(i) + " -randrepeat=0";
            // iodepth debug
            cout << "io深度：" + to_string(iodepth) << endl;
            cout << cmd << endl;
          } else if (dorf.find("directory") != string::npos) {
            name =
                "seq_read_directory_numjobs=1_iodepth=" + to_string(iodepth) +
                "_bs=" + to_string(bs) + "k";
            // 构建fio命令
            cmd = "fio -name=" + name + " -size=" + fsize +
                  " -runtime=" + runtime +
                  " -time_base -bs=1m -direct=" + direct +
                  " -rw=write "
                  "-ioengine=" +
                  ioengine + " -numjobs=1" +
                  " -group_reporting -iodepth=" + to_string(iodepth) + " -" +
                  dorf + to_string(iodepth) + "/" + to_string(bs) + "k" +
                  to_string(i) + "/ -randrepeat=0";
            cout << cmd << endl;
          }
          break;

          // 使用popen执行命令并获取输出
          FILE *fp = popen(cmd.c_str(), "r");
          if (fp == nullptr) {
            cerr << "Error opening pipe!" << endl;
            return;
          }

          // 从fio输出中获取数据
          stringstream fio_output;
          char buffer[128];
          while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
            fio_output << buffer;
          }
          fclose(fp);

          // 分析fio输出
          vector<string> bw_num;
          vector<string> iops_num;
          string line;
          while (getline(fio_output, line)) {
            if (line.find("bw") != string::npos) {
              if (line.find("MiB/s") != string::npos) {
                // 提取带宽数字
                regex bw_regex(R"(\d+\.\d+|\d+)");
                smatch match;
                while (regex_search(line, match, bw_regex)) {
                  // 检测到单位是MiB，则转换为MiB
                  float bw_value_kib = stof(match.str()) * 1024;
                  int bw_value_bytes =
                      static_cast<int>(bw_value_kib); // 转为整数
                  bw_num.push_back(match.str());
                  line = match.suffix();
                }
              } else {
                // 提取带宽数字
                regex bw_regex(R"(\d+\.\d+|\d+)");
                smatch match;
                while (regex_search(line, match, bw_regex)) {
                  bw_num.push_back(match.str());
                  line = match.suffix();
                }
              }
            } else if (line.find("iops") != string::npos) {
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
          vector<int> bw_int;
          vector<int> iops_int;
          for (const string &s : bw_num) {
            bw_int.push_back(static_cast<int>(stof(s)));
          }
          for (const string &s : iops_num) {
            iops_int.push_back(static_cast<int>(stof(s)));
          }

          cout << name << "单次带宽运行结果:"
               << "min:" << bw_int[0] << " max:" << bw_int[1]
               << " avg:" << bw_int[3] << "\n"
               << name << "单次IOPS运行结果:"
               << "min:" << iops_int[0] << " max:" << iops_int[1]
               << " avg:" << iops_int[3] << endl;

          // 跳过第一次运行结果
          if (i == 0)
            continue;

          // 整理带宽和IOPS数据
          bw[0] += bw_int[0];
          bw[1] += bw_int[1];
          bw[2] += bw_int[3];
          iops[0] += iops_int[0];
          iops[1] += iops_int[1];
          iops[2] += iops_int[2];
        }

        // 计算最小、最大和平均值
        int bwMin = bw[0] / 3;
        int bwMax = bw[1] / 3;
        int bwAvg = bw[2] / 3;
        int iopsMin = iops[0] / 3;
        int iopsMax = iops[1] / 3;
        int iopsAvg = iops[2] / 3;

        vector<int> values = {bwMin, bwMax, bwAvg, iopsMin, iopsMax, iopsAvg};

        // 将结果存储到数据表中，第一列是 randwrite_4k，后面是6个值
        vector<string> row = {name}; // 第一列是测试类型
        for (int val : values) {
          row.push_back(to_string(val)); // 将每个值转换为字符串并添加到行中
        }
        run_report.push_back(row); // 将这行添加到数据中
      }
    }
  }
}
// --- 顺序写结束 ---

void runReport() {
  // 输出存储的数据，模拟 Excel 风格
  cout << "测试类型\t带宽最小值\t带宽最大值\t带宽均值\tIOPS最大值\tIOPS最小值\t"
          "IOPS均值"
       << endl;
  for (const auto &row : run_report) {
    for (const auto &cell : row) {
      cout << cell << "\t";
    }
    cout << endl;
  }
}