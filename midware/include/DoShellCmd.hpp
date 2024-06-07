
#ifndef DOSHELLCMD__H__
#define DOSHELLCMD__H__
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <string>
#include "mlog.h"

class CDoShellCmd
{
public:
    CDoShellCmd()
    {
        ptr_Output = NULL;
    }
    ~CDoShellCmd()
    {
    }

    std::string operator()(std::string cmd)
    {
        ptr_Output = popen(cmd.c_str(), "r");
        if (NULL == ptr_Output)
            LOGERR("popen error: %s, cmd fail:%s", strerror(errno), cmd.data());
        char buff[1024] = {0};
        size_t length = fread(buff, 1, sizeof(buff), ptr_Output);
        if (length > 0)
            str_Result.assign(buff, length);
        else
            LOGINF("cmd [%s] resul is error or null : %s ", cmd.data(), strerror(errno));
        pclose(ptr_Output);
        ptr_Output = NULL;

        return str_Result;
    }

private:
    FILE *ptr_Output;
    std::string str_Result;
};

#endif /*DOSHELLCMD__H__*/