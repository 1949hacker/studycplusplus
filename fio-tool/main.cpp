#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <regex>
#include <array>
#include <stdexcept>
#include <fstream>
#include <cstdlib>

using namespace std;

// rm_file 函数实现：假设删除临时文件的操作
void rm_file()
{
    // 这里可以根据需求编写删除文件的逻辑
    system("rm -rf /iopsTest/*"); // 删除 /iopsTest 目录下的所有文件
    cout << "临时文件已删除" << endl;
}

// randwrite 函数实现
void randwrite()
{
    vector<int> bw = {0, 0, 0};
    vector<int> iops = {0, 0, 0};

    cout << "随机写进行中..." << endl;

    for (int i = 0; i < 4; ++i)
    {
        // 构建fio命令
        string cmd = "fio -name=YEOS -size=5G -runtime=15s -time_base -bs=4k -direct=1 -rw=randwrite -ioengine=libaio -numjobs=12 -group_reporting -iodepth=64 -filename=~/iopsTest/" + to_string(i) + " -randrepeat=0";

        // 使用popen执行命令并获取输出
        FILE *fp = popen(cmd.c_str(), "r");
        if (fp == nullptr)
        {
            cerr << "Error opening pipe!" << endl;
            return;
        }

        // 从fio输出中获取数据
        stringstream fio_output;
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), fp) != nullptr)
        {
            fio_output << buffer;
        }
        fclose(fp);

        // 分析fio输出
        vector<string> bw_num;
        vector<string> iops_num;
        string line;
        while (getline(fio_output, line))
        {
            if (line.find("bw") != string::npos)
            {
                // 提取带宽数字
                regex bw_regex(R"(\d+\.\d+|\d+)");
                smatch match;
                while (regex_search(line, match, bw_regex))
                {
                    bw_num.push_back(match.str());
                    line = match.suffix();
                }
            }
            else if (line.find("iops") != string::npos)
            {
                // 提取IOPS数字
                regex iops_regex(R"(\d+\.\d+|\d+)");
                smatch match;
                while (regex_search(line, match, iops_regex))
                {
                    iops_num.push_back(match.str());
                    line = match.suffix();
                }
            }
        }

        // 转换为整数
        vector<int> bw_int;
        vector<int> iops_int;
        for (const string &s : bw_num)
        {
            bw_int.push_back(static_cast<int>(stof(s)));
        }
        for (const string &s : iops_num)
        {
            iops_int.push_back(static_cast<int>(stof(s)));
        }

        cout << "单次带宽运行结果:" << endl;
        for (int num : bw_int)
        {
            cout << num << " ";
        }
        cout << endl;

        cout << "单次IOPS运行结果:" << endl;
        for (int num : iops_int)
        {
            cout << num << " ";
        }
        cout << endl;

        // 跳过第一次运行结果
        if (i == 0)
            continue;

        // 处理带宽和IOPS数据
        for (size_t j = 0; j < bw_int.size(); ++j)
        {
            if (bw_num[j].find("MiB") != string::npos)
            {
                cout << "输出结果为MiB单位,将进行转换" << endl;
                bw[0] += bw_int[0] * 1024;
                bw[1] += bw_int[1] * 1024;
                bw[2] += bw_int[3] * 1024;
                iops[0] += iops_int[0];
                iops[1] += iops_int[1];
                iops[2] += iops_int[2];
            }
            else if (bw_num[j].find("KiB") != string::npos)
            {
                cout << "输出结果为KiB单位,不转换" << endl;
                bw[0] += bw_int[0];
                bw[1] += bw_int[1];
                bw[2] += bw_int[3];
                iops[0] += iops_int[0];
                iops[1] += iops_int[1];
                iops[2] += iops_int[2];
            }
        }
    }

    // 计算最小、最大和平均值
    int bwMin = bw[0] / 3;
    int bwMax = bw[1] / 3;
    int bwAvg = bw[2] / 3;
    int iopsMin = iops[0] / 3;
    int iopsMax = iops[1] / 3;
    int iopsAvg = iops[2] / 3;

    // 输出最终结果
    cout << "\n\n\n随机写均值如下:" << endl;
    cout << "带宽最小值:" << bwMin << ", 最大值" << bwMax << ", 均值" << bwAvg << endl;
    cout << "IOPS最小值:" << iopsMin << ", 最大值" << iopsMax << ", 均值" << iopsAvg << endl;
}

int main()
{
    cout << "欢迎使用fio测试工具\n本工具测试内容:\n路径挂载模式下IOPS性能测试" << endl;

    // 删除临时文件
    rm_file();

    // 进行随机写测试
    randwrite();

    // 删除临时文件
    rm_file();

    return 0;
}
