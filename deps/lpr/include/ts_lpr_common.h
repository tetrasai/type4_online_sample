#ifndef TS_LPR_COMMON_H_
#define TS_LPR_COMMON_H_

/*!\addtogroup common
 *  @{
 */

#include <string>

#define TS_SDK_API_ __attribute__((visibility("default")))
#ifdef __cplusplus
#define TS_SDK_API extern "C" TS_SDK_API_
#else
#define TS_SDK_API TS_SDK_API_
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef int ts_errcode;
typedef ts_errcode ts_result_code;

// SDK ERROR CODE
#define TS_OK 0
#define TS_E_INVALIDARG -1
#define TS_E_HANDLE -2
#define TS_E_OUTOFMEMORY -3
#define TS_E_FAIL -4
#define TS_E_INVALID_PIXEL_FORMAT -6
#define TS_E_FILE_NOT_FOUND -7
#define TS_E_INVALID_FILE_FORMAT -8
#define TS_E_MISSING_CAMERA_PARAM -9
#define TS_E_INVALID_AUTH -13
#define TS_E_INVALID_APPID -14
#define TS_E_AUTH_EXPIRE -15
#define TS_E_UNZIP_FAILED -25
#define TS_E_FRAME_SKIP -701
#define TS_E_FRAME_SYNC_NOT_READY -702
#define TS_E_MODULE_NOT_INITIZATION -999
#define TS_E_UNSUPPORTED -1000

typedef enum {
    TS_PIX_FMT_GRAY8,    // channel is 1
    TS_PIX_FMT_BGR888,   // channel is 3
    TS_PIX_FMT_NV21,     // channel is 1
    TS_PIX_FMT_NV12,     // channel is 1
    TS_PIX_FMT_YUV420P,  // channel is 1
} TS_LPR_pixel_format_t;

typedef struct TS_LPR_image_t {
    unsigned char *image_data;
    int width;
    int height;
    int channel;
    TS_LPR_pixel_format_t format;
} TS_LPR_image_t;

typedef struct TS_LPR_input_t {
    TS_LPR_image_t image;
    unsigned int camera_id;
    uint64_t time_stamp;
} TS_LPR_input_t;

typedef struct TS_LPR_point2f_t {
    float x;
    float y;
} TS_LPR_point2f_t;

typedef struct TS_LPR_point3f_t {
    float x;
    float y;
    float z;
} TS_LPR_point3f_t;

typedef struct TS_LPR_landmarks_t {
    TS_LPR_point2f_t *points;
    float *landmark_score;
    int size;
} TS_LPR_landmarks_t;

typedef struct TS_LPR_rect_t {
    int left;
    int top;
    int right;
    int bottom;
    float rect_score;
} TS_LPR_rect_t;

typedef struct TS_LPR_text_t {
    std::string text;
    float text_quality;
} TS_LPR_info_t;

typedef struct TS_LPD_result_t {
    TS_LPR_rect_t vehicle_bbox;
    TS_LPR_rect_t plate_bbox;
    TS_LPR_landmarks_t plate_keypoints;
} TS_LPD_result_t;

typedef struct TS_LPR_result_t {
    TS_LPD_result_t lpd_result;
    TS_LPR_text_t *texts_array;
    int texts_array_size;
    TS_LPR_image_t vehicle_image;
    float quality;
} TS_LPR_result_t;

typedef enum {
    TS_LPR_LOG_DEBUG,  ///< Print logs above DEBUG (inclusive) level
    TS_LPR_LOG_INFO,   ///< Print logs above INFO  (inclusive) level
    TS_LPR_LOG_WARN,   ///< Print logs above WARNING (inclusive) level
    TS_LPR_LOG_ERR,    ///< Print logs above ERROR (inclusive) level
    TS_LPR_LOG_CLOSE   ///< Print logs close
} TS_LPR_log_level;

#ifdef __cplusplus
}
#endif

/*!@} */

#endif
