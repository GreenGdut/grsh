#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cstdio>
#include <limits.h>

#include<sys/wait.h>
#include<unistd.h>


int main()
{
    std::string line;

    while (true)
    {
        std::cout << "my_shell>" << std::flush;

        if (!std::getline(std::cin, line))
        {
            std::cout << '\n';
            break;
        }

        std::istringstream input(line);
        std::vector<std::string> args;
        std::string argument;

        while (input >> argument)
        {
            args.push_back(argument);
        }

        if (args.empty())
        {
            continue;
        }

        if (args[0] == "exit")
        {
            break;
        }

        //cd
        if (args[0] == "cd")
        {
            if (args.size() < 2)
            {
                std::cout << '\n';
                continue;
            }

            if (args.size() > 2)
            {
                std::cout << "Too many arguments!" << '\n';
                continue;
            }

            if (chdir(args[1].c_str()) == -1)//chdir切目录，成功返回0
            {
                std::perror("cd");
            }

            continue;//cd 处理后直接开始新的主循环，跳过后续fork()等
        }

        //pwd
        if (args[0] == "pwd")
        {
            char current_dir[PATH_MAX];
            if (getcwd(current_dir,sizeof(current_dir)) == nullptr)
            {
                std::perror("pwd");
            }
            else
            {
                std::cout << current_dir << '\n';
            }

            continue;//跳过当前循环，忽略后续操作
        }

        std::vector<char*> argv;

        for (auto & arg : args)
        {
            argv.push_back(arg.data());
        }

        argv.push_back(nullptr);

        pid_t pid = fork();
        //创建子进程并返回状态
        //<0创建失败
        //=0处于子进程
        //>0创建成功并返回pid

        if (pid < 0)
        {
            std::perror("fork");
            continue;
        }
        if (pid == 0)
        {
            execvp(argv[0], argv.data());
            std::perror("execvp");
            _exit(127);
        }

        int status;

        if (waitpid(pid, &status, 0) == -1)
        {
            std::perror("waitpid");
        }
    }
    return 0;
}




