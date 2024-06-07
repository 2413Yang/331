
#ifndef FILE_L__H__
#define FILE_L__H__

#include <string>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

class CFileL
{
public:
    CFileL(std::string file)
        : mDirp(NULL), mFp(NULL)
    {
        if (file[file.length() - 1] == '/')
        {
            isFile = false;
            mDirp = opendir(file.data());
        }
        else
        {
            isFile = true;
            mFp = fopen(file.data(), "r");
        }
    }

    ~CFileL()
    {
        if (isFile)
            mFp ? fclose(mFp) : 0;
        else
            mDirp ? closedir(mDirp) : 0;
    }

    std::string operator()()
    {
        std::string line;
        if (isFile)
        {
            char buffer[512] = {0};
            mFp ? fgets(buffer, sizeof(buffer), mFp) : 0;
            line = buffer;
        }
        else
        {
            struct dirent *direntp = NULL;
            mDirp ? direntp = readdir(mDirp) : 0;
            if (direntp)
                line = direntp->d_name;
        }
        return line;
    }

    std::vector<std::string> encodeCfg(std::string line)
    {
        std::vector<std::string> value;
        unsigned int subLength = 0, subBegin = 0;
        for (auto it : line)
        {
            if (it == '#')
                break;
            else if (it == ' ' || it == '\r' || it == '\n')
            {
                if (subLength != 0)
                {
                    value.push_back(line.substr(subBegin, subLength));
                    subBegin += subLength;
                }
                subLength = 0;
                subBegin++;
            }
            else
            {
                subLength++;
                if ((subBegin + subLength) == line.length() && subLength > 0)
                    value.push_back(line.substr(subBegin, subLength));
            }
        }

        return value;
    }

private:
    bool isFile;
    DIR *mDirp;
    FILE *mFp;
};

#endif /*FILE_L__H__*/