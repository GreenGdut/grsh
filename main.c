//
// Created by green_gdut on 2026/10/15.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>
#include <sys/types.h>
#include <fcntl.h>

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

        //pipe
        char** left_args = NULL;
        char** right_args = NULL;
        size_t pipe_index = 0;
        size_t pipe_count = 0;

        for (size_t i = 0;i < argc;i++)
        {
            if ( strcmp(args[i],"|") == 0)
            {
                pipe_count++;
                pipe_index = i;
            }
        }

        if (pipe_count > 1)
        {
            fprintf(stderr,"pipe count: %lu\n",pipe_count);
            continue;
        }

        if (pipe_count == 1)
        {
            if (pipe_index == 0 || pipe_index == argc - 1)
            {
                fprintf(stderr,"syntax error: expected 'command | command'\n");
                continue;
            }
            args[pipe_index] = NULL;
            right_args = &args[pipe_index + 1];
            left_args = &args[0];

            int pipe_fds[2];
            if (pipe(pipe_fds) == -1)
            {
                perror("pipe");
                continue;
            }

            pid_t left_pid = fork();
            if (left_pid < 0)
            {
                perror("fork");
                close(pipe_fds[0]);
                close(pipe_fds[1]);
                continue;
            }

            if (left_pid == 0)
            {
                if (dup2(pipe_fds[1],STDOUT_FILENO) == -1)
                {
                    perror("dup2");
                    _exit(1);
                }

                close(pipe_fds[0]);
                close(pipe_fds[1]);

                if (execvp(left_args[0],left_args) == -1)
                {
                    perror("execvp");
                    _exit(127);
                }
            }

            pid_t right_pid = fork();
            if (right_pid < 0)
            {
                perror("fork");
                close(pipe_fds[0]);
                close(pipe_fds[1]);
                if (waitpid(left_pid,NULL,0))
                {
                    perror("waitpid");
                    _exit(1);
                }
                continue;
            }

            if (right_pid == 0)
            {
                if (dup2(pipe_fds[0],STDIN_FILENO) == -1)
                {
                    perror("dup2");
                    _exit(1);
                }

                close(pipe_fds[0]);
                close(pipe_fds[1]);

                if (execvp(right_args[0],right_args) == -1)
                {
                    perror("execvp");
                    _exit(127);
                }
            }

            close(pipe_fds[0]);
            close(pipe_fds[1]);
            if (waitpid(left_pid,NULL,0))
            {
                perror("waitpid");
                _exit(1);
            }
            if (waitpid(right_pid,NULL,0))
            {
                perror("waitpid");
                _exit(1);
            }

            continue;

        }//pipe

        //redirection_out
        char* output_file = NULL;
        int redirection_error = 0;

        for (size_t i = 0;i < argc;i++)
        {
            if (strcmp(args[i],">") == 0)
            {
                if (i == 0 || i + 1 >= argc || i + 2 != argc || strcmp(args[i + 1], ">") == 0)
                /*难点
                 *首先，要知道argv[]有n个参数，但有效参数仅有n-1个，末尾是NULL
                 *上面四个条件分别筛选以下 错误情况
                 * > 前无命令
                 * > 后无filename，直接接NULL
                 * > filename 后仍有有效参数，而不是NULL
                 * 把第二个>当成filename
                 */

                {
                    fprintf(stderr, "syntax error: expected 'command > filename'\n");
                    redirection_error = 1;
                    break;
                }

                output_file = args[i+1];
                args[i] = NULL; //防止把> filename传给execvp(),
                argc = i;
                break;
            }
        }//redirection_out

        //redirection_in
        char* input_file = NULL;

        for (size_t i = 0;i < argc;i++)
        {
            if (strcmp(args[i],"<") == 0)
            {
                if (i == 0 || i + 1 >= argc || i + 2 != argc || strcmp(args[i + 1], "<") == 0)
                {
                    fprintf(stderr,"syntax error: expected 'command < filename'\n");
                    redirection_error = 1;
                    break;
                }

                input_file = args[i+1];
                args[i] = NULL;
                argc = i;
                break;
            }
        }//redirection_in

        //出错跳过循环
        if (redirection_error)
        {
            continue;
        }

        //fork
        //execvp()执行成功后不会执行后续函数，因此需要放到子进程
        pid_t pid = fork();

        if (pid < 0)
        {
            perror("fork");
            continue;
        }

        if (pid == 0)
        {
            //direction_out
            if (output_file != NULL)
            {
                const int output_fd = open(output_file,O_WRONLY|O_CREAT|O_TRUNC,0644);
                if (output_fd < 0)
                {
                    perror("open");
                    _exit(1);
                }

                if (dup2(output_fd,STDOUT_FILENO) == -1)
                {
                    perror("dup2");
                    _exit(1);
                }

                if (close(output_fd) == -1)
                {
                    perror("close");
                    _exit(1);
                }
            }//direction_out

            //direction_in
            if (input_file != NULL)
            {
                const int input_fd = open(input_file,O_RDONLY);

                if (input_fd < 0)
                {
                    perror("open");
                    _exit(1);
                }

                if (dup2(input_fd,STDIN_FILENO) == -1)
                {
                    perror("dup2");
                    _exit(1);
                }

                if (close(input_fd) == -1)
                {
                    perror("close");
                    _exit(1);
                }
            }//direction_in


            execvp(args[0],args);
            //(要执行的程序名，以NULL结尾的参数指针数组）
            //寻找对应程序替换进程
            //v代表传入argv数组,会把该数组作为新程序的传入参数
            //p代表通过PATH查找程序位置
            //执行成功不返回值且后续函数被忽略
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
