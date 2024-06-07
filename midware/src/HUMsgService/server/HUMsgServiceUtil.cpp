#include <iomanip>
#include "HUMsgService.h"

static void Tokenize(const std::string &str, std::vector<std::string> &tokens, const std::string &delimiters)
{
    // Skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);

    // Find first "non-delimiter".
    std::string::size_type pos = str.find_first_of(delimiters, lastPos);

    while (std::string::npos != pos || std::string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));

        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);

        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

size_t CHUMsgService::getDiskFreeBytes(const std::string &mountPath)
{
    FILE *procpt;
    int abailable = -1;
    bool bFirstLine = true;
    std::string cmd = "df " + mountPath;
    std::string line;
    const std::string whiteSpace = " ";
    std::vector<std::string> split;
    char lineBuf[512];

    procpt = popen(cmd.data(), "r");
    while (fgets(lineBuf, sizeof(lineBuf), procpt))
    {
        if (bFirstLine)
        {
            bFirstLine = false;
            continue;
        }

        line = lineBuf;

        Tokenize(line, split, whiteSpace);
        if (split.size() >= 6)
        {
            abailable = atoi(split[3].data());
        }
        break;
    }

    return abailable * 1024;
}

std::string CHUMsgService::getHexString(std::string const &buffer, std::string const &delimiter)
{
    std::stringstream ss;
    std::string space = "";
    for (size_t i = 0; i < buffer.length(); i++) {
        ss << space << std::hex << std::setw(2) << std::setfill('0') << int(buffer[i]);
        space = delimiter;
    }
    return ss.str();
}

std::string CHUMsgService::getFileMD5(std::string const &filename)
{
    FILE *procpt;
    const std::string cmd = "md5sum " + filename;
    std::string line = "";
    char lineBuf[33] = {0};

    procpt = popen(cmd.data(), "r");
    if (fgets(lineBuf, sizeof(lineBuf), procpt))
    {
        line = (char *)lineBuf;
    }

    return line;
}