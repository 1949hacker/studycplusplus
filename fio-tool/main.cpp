#include <iostream>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    // 使用popen打开一个管道来执行ls命令，"r"表示以读取模式打开管道
    FILE *pipe = popen("ls", "r");
    if (!pipe)
    {
        std::cerr << "无法打开管道执行ls命令" << std::endl;
        return EXIT_FAILURE;
    }

    char buffer[128];
    // 从管道中读取输出内容并打印到控制台
    while (fgets(buffer, sizeof(buffer), pipe) != NULL)
    {
        std::cout << buffer;
    }

    // 关闭管道
    int returnCode = pclose(pipe);
    if (returnCode == -1)
    {
        std::cerr << "无法关闭管道" << std::endl;
        return EXIT_FAILURE;
    }

    // 根据返回码判断命令执行是否成功
    if (returnCode != 0)
    {
        std::cerr << "ls命令执行失败，返回码: " << returnCode << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}