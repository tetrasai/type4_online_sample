#include <dirent.h>
#include <fstream>
#include <stdarg.h>
#include <string.h>
#include <string>
#include <sys/time.h>
#include <unistd.h>

#include "utils.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int ReadFile(const char *filePath, char **content, size_t *pFileLen)
{
    if (!content) {
        return -1;
    }
    FILE *pF = nullptr;
    pF = fopen(filePath, "r");
    if (pF == nullptr) {
        return -1;
    }
    fseek(pF, 0, SEEK_END);
    int nFileLen = ftell(pF);
    rewind(pF);
    char *szBuffer = new (std::nothrow) char[nFileLen + 1];
    if (!szBuffer) {
        fclose(pF);
        return -1;
    }
    nFileLen = fread(szBuffer, sizeof(char), nFileLen, pF);
    szBuffer[nFileLen] = '\0';
    fclose(pF);
    *content = szBuffer;
    if (pFileLen) {
        *pFileLen = nFileLen;
    }
    return 0;
}

int ReadFileByMMap(const char *filePath, char **content, size_t *pFileLen)
{
    if (!content) {
        return -1;
    }
    int fd = open(filePath, O_RDONLY);
    if (fd == -1) {
        return -1;
    }
    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size == -1) {
        return -1;
    }
    lseek(fd, 0, SEEK_SET);

    char *szBuffer = ( char * )mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);

    if (close(fd) == -1) {
        return -1;
    }
    *content = szBuffer;
    if (pFileLen) {
        *pFileLen = file_size;
    }
    return 0;
}

bool GetFileList(const char *basePath,
                 std::vector< std::string > &fileList,
                 const std::vector< std::string > &suffixs)
{
    static const int MAXBUFSIZE = 1024;

    char buf[1024] = {0};
    getcwd(buf, MAXBUFSIZE);

    DIR *dir;
    struct dirent *ptr;
    char base[1000] = {0};
    if ((dir = opendir(basePath)) == nullptr) {
        return false;
    }
    while ((ptr = readdir(dir)) != nullptr) {
        if (strcmp(ptr->d_name, ".") == 0
            || strcmp(ptr->d_name, "..") == 0)  /// current dir OR parrent dir
            continue;
        else if (ptr->d_type == 8) {  /// file
            // std::string strImagePath = buf + std::string("/") + basePath + "/" + ptr->d_name;
            std::string strImagePath = std::string(basePath) + "/" + ptr->d_name;
            auto pos = strImagePath.rfind(".");
            std::string suffix(strImagePath.c_str() + pos, strImagePath.length() - pos);
            bool bExist(false);  // 直接读取文件
            for (const auto &item : suffixs) {
                if (suffix == item) {
                    bExist = true;
                    break;
                }
            }
            if (bExist) {
                fileList.push_back(std::move(strImagePath));
            }
        } else if (ptr->d_type == 10) {  /// link file
        } else if (ptr->d_type == 4) {   /// dir
            memset(base, '\0', sizeof(base));
            strcpy(base, basePath);
            strcat(base, "/");
            strcat(base, ptr->d_name);
            GetFileList(base, fileList, suffixs);
        }
    }
    closedir(dir);
    return true;
}

std::vector< std::string > split_string(const std::string &src, const std::string &splitter)
{
    std::vector< std::string > sub_strs;

    size_t head = 0;
    while (head < src.length()) {
        size_t tail = src.find(splitter, head);
        if (tail != std::string::npos) {
            sub_strs.push_back(src.substr(head, tail - head));
            head = tail + splitter.length();
        } else {
            sub_strs.push_back(src.substr(head));
            break;
        }
    }

    return sub_strs;
}

uint64_t GetTimeStampFromUTC(const std::string &utc_time)
{
    if (utc_time.size() != 17) {
        printf("invalid UTC time: %s\n", utc_time.c_str());
        return 0;
    }

    std::string str_year = utc_time.substr(0, 4);
    std::string str_month = utc_time.substr(4, 2);
    std::string str_day = utc_time.substr(6, 2);
    std::string str_hour = utc_time.substr(8, 2);
    std::string str_min = utc_time.substr(10, 2);
    std::string str_sec = utc_time.substr(12, 2);
    std::string str_millisec = utc_time.substr(14, 3);

    struct tm time;
    time.tm_year = std::stoi(str_year) - 1900;
    time.tm_mon = std::stoi(str_month) - 1;
    time.tm_mday = std::stoi(str_day);
    time.tm_hour = std::stoi(str_hour);
    time.tm_min = std::stoi(str_min);
    time.tm_sec = std::stoi(str_sec);

    uint64_t timestamp = 0;
    timestamp = mktime(&time) * 1000 + std::stoi(str_millisec);
    printf("timestamp: %lu\n", timestamp);

    return timestamp;
}

void RemoveFile(const std::string &file_path)
{
    if (std::remove(file_path.c_str()) == 0) {
        printf("remove file %s success\n", file_path.c_str());
    } else {
        printf("remove file %s failed\n", file_path.c_str());
    }
}

uint64_t GetTimeStampFromSys()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return static_cast< uint64_t >(tv.tv_sec) * 1000 + static_cast< uint64_t >(tv.tv_usec) / 1000;
}

std::string FetchCurrentDate()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    time_t sec = static_cast< time_t >(tv.tv_sec);

    struct tm *local = localtime(&sec);
    char time_buf[20] = {0};
    strftime(time_buf, 20, "%Y%m%d%H%M", local);

    return std::string(time_buf);
}

std::string FetchCurrentFormatTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    time_t sec = static_cast< time_t >(tv.tv_sec);

    struct tm *local = localtime(&sec);
    char time_buf[64] = {0};
    strftime(time_buf, 64, "%Y-%m-%d %H:%M:%S:", local);

    int msec = static_cast< int >(tv.tv_usec / 1000);
    char msec_buf[4] = {0};
    snprintf(msec_buf, sizeof(msec_buf), "%03d", msec);

    return std::string(time_buf) + std::string(msec_buf);
}