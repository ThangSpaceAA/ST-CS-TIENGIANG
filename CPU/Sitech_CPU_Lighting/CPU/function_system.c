#include "function_system.h"

bool get_State_Cmd_Terminal(char *cmd, char *sample)
{
    // Lay output gan bien
    FILE *fd;

    int line_num = 1;
    int find_result = 0;
    char temp[512];

    fd = popen(cmd, "r");
    if (!fd)
        return 1;
    //char buffer[1024];

    while (fgets(temp, 512, fd) != NULL)
    {
        if ((strstr(temp, sample)) != NULL)
        {
            // printf("A match found on line: %d\n", line_num);
            // printf("\n%s\n", temp);
            find_result++;
        }
        line_num++;
    }

    //Close the file if still open.
    if (fd)
    {
        fclose(fd);
    }
    if (find_result > 0)
    {
        return true;
    }
    return false;
}