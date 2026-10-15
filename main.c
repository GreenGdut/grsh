//
// Created by green_gdut on 2026/10/15.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>

int main()
{
    char line[4096];

    while(1)
    {
        printf("my_shell>");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin))
        {
            printf("\n");
            break;
        }

        char* args[128];
        size_t argc = 0;

        char* argument = strtok(line," \t\n");

        while (argument != NULL && argc < 127)
        {
            args[argc] = argument;
            argc++;

            argument = strtok(NULL," \t\n");
        }

        args[argc] = NULL;

        if (argc == 0)
        {
            continue;
        }

        if (strcmp(args[0], "exit") == 0)
        {
            break;
        }

        //cd
        if (strcmp(args[0], "cd") == 0)
        {
            if (argc < 2)
            {
                continue;
            }

            if (argc > 2)
            {
                printf("cd: too many arguments\n");
                continue;
            }

            if (chdir(args[1]) == -1)//chdir()切换进程目录，若成功则返回0，失败返回-1
            {
                perror("cd");
            }

            continue;
        }//cd

        //pwd
        if (strcmp(args[0],"pwd") == 0)
        {
            char current_dir[PATH_MAX];

            if (getcwd(current_dir,sizeof(current_dir)) == NULL)
            {
                perror("pwd");
            }

            else
            {
                puts(current_dir);
            }

            continue;
        }

        //redirection


        //redirection

        //fork
        pid_t pid = fork();

        if (pid < 0)
        {
            perror("fork");
            continue;
        }

        if (pid == 0)
        {
            execvp(args[0],args);//(要执行的程序名，以NULL结尾的参数指针数组）
            perror("execvp");//execvp成功则不会返回
            _exit(127);
        }

        int status;

        if (waitpid(pid,&status,0) == -1)
        {
            perror("waitpid");
        }//fork



    }

    return 0;
}