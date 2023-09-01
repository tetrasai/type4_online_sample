#ifndef __UTILS_H__
#define __UTILS_H__

#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#define __FILENAME__ \
    (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

std::string FetchCurrentFormatTime();
std::string FetchCurrentDate();

#define LOGD(fmt, ...)                                                                       \
    printf("[%s] [Debug] %s(%d): " fmt "\n", FetchCurrentFormatTime().c_str(), __FILENAME__, \
           __LINE__, ##__VA_ARGS__)
#define LOGI(fmt, ...)                                                                      \
    printf("[%s] [Info] %s(%d): " fmt "\n", FetchCurrentFormatTime().c_str(), __FILENAME__, \
           __LINE__, ##__VA_ARGS__)
#define LOGW(fmt, ...)                                                                         \
    printf("[%s] [Warning] %s(%d): " fmt "\n", FetchCurrentFormatTime().c_str(), __FILENAME__, \
           __LINE__, ##__VA_ARGS__)
#define LOGE(fmt, ...)                                                                       \
    printf("[%s] [Error] %s(%d): " fmt "\n", FetchCurrentFormatTime().c_str(), __FILENAME__, \
           __LINE__, ##__VA_ARGS__)

using std::chrono::steady_clock;

class timeRecoder
{
public:
    explicit timeRecoder(const char *desc)
    {
        startTime = steady_clock::now();
        description = desc;
    }

    ~timeRecoder()
    {
        steady_clock::time_point endTime = steady_clock::now();
        auto duration =
            std::chrono::duration_cast< std::chrono::microseconds >(endTime - startTime);
        std::string format_time = FetchCurrentFormatTime();
        printf("[%s] %s time: [%f] ms\n", format_time.c_str(), description,
               duration.count() / 1000.f);
    }

private:
    steady_clock::time_point startTime;
    const char *description;
};

std::vector< std::string > split_string(const std::string &src, const std::string &splitter);
int ReadFile(const char *filepath, char **content, size_t *pFileLen);
int ReadFileByMMap(const char *filePath, char **content, size_t *pFileLen);

bool GetFileList(const char *basePath,
                 std::vector< std::string > &fileList,
                 const std::vector< std::string > &suffixs);
void RemoveFile(const std::string &file_path);

uint64_t GetTimeStampFromUTC(const std::string &utc_time);
uint64_t GetTimeStampFromSys();

#endif
