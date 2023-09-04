/**
 * @file type4_main.cpp
 * @brief Tetras sample code which include DataRecv, DataAlign, SDK run, DataTransmision
 * @version 0.1
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <memory>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include "BlockingQueue.h"
#include "data_trans.hpp"
#include "draw_helper.h"
#include "json11.hpp"
#include "ts_lpr_common.h"
#include "ts_lpr_perception.h"
#include "utils.h"

#include <chrono>
#include <fstream>
#include <stdio.h>

#include <sys/mman.h>
#include <sys/stat.h>

static const int WIDTH = 2016;
static const int HEIGHT = 1520;

static const int TENSOR_SIZE = 1670400;
static const int HD_IMAGE_SIZE = WIDTH * HEIGHT * 2;  // YUYV422
static const int MAX_QUEUE_SIZE = 10;

static float x_magnify;
static float y_magnify;
static float x_offset;
static float y_offset;

static bool delete_image_tensor = true;

static double fps;

static int frame_count = 0;

static float process_time_count = 0;

struct TensorAndImage
{
    TS_LPD_result_t *lpd_result;
    size_t lpd_result_len;
    // cv::Mat image_mat;
    uint64_t timestamp = 0;
    TS_LPR_image_t image;
    uint64_t image_prepare_time_cost = 0;
};

static BlockingQueue< TensorAndImage > g_data_queue;

static int InitLicense(const std::string &license_path);
static bool CheckParams(const json11::Json &root);

static int ReceiveDataFromFile();  // For official CameraApp

static char *tensor_file_dir = "";
static char *image_file_dir = "";

static void releaseTensorAndImage(TensorAndImage *tensor_and_image);

static void convertLPDResult(TS_LPD_result_t *lpd_result, size_t lpd_result_len);

// #define CROP_VEHICLE

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: %s <path/to/config.json>\n", argv[0]);
        return -1;
    }

    char *json_buffer{nullptr};
    size_t json_buffer_len{0};
    int ret = ReadFile(argv[1], &json_buffer, &json_buffer_len);
    if (ret != 0) {
        LOGE("read file %s failed", argv[1]);
        return -1;
    }

    std::string err;
    json11::Json root = json11::Json::parse(json_buffer, err);
    delete[] json_buffer;
    json_buffer = nullptr;
    if (!err.empty()) {
        LOGE("json file parse failed: %s", err.c_str());
        return -1;
    }

    if (!CheckParams(root)) {
        return -1;
    }

    int camera_id = root["camera_id"].int_value();
    std::string model_path = root["model_path"].string_value();
    std::string lpd_param_path = root["lpd_param_path"].string_value();
    std::string lpr_param_path = root["lpr_param_path"].string_value();
    std::string license_path = root["license_path"].string_value();
    bool draw_result = root["draw_result"].bool_value();
    int draw_result_rate = root["draw_result_rate"].int_value();

    // delete_image_tensor = root["delete_tensor_and_image"].bool_value();
    tensor_file_dir = const_cast< char * >(root["inference_path"].string_value().c_str());
    image_file_dir = const_cast< char * >(root["raw_path"].string_value().c_str());

    auto mqtt_param_obj = root["mqtt_param"];
    std::string mqtt_broker_ip = mqtt_param_obj["mqtt_broker_ip"].string_value();
    std::string mqtt_broker_port = mqtt_param_obj["mqtt_broker_port"].string_value();
    std::string mqtt_topic = mqtt_param_obj["camera_id"].string_value();
    int mqtt_sendbuf_size = mqtt_param_obj["mqtt_sendbuf_size"].int_value();
    int mqtt_recvbuf_size = mqtt_param_obj["mqtt_recvbuf_size"].int_value();

    const json11::Json ZoomInParamObj = root["InputTensorToRawZoomInParam"];
    if (!ZoomInParamObj.is_null()) {
        if (ZoomInParamObj["input_tensor_x_magnify"].is_null()
            || ZoomInParamObj["input_tensor_y_magnify"].is_null()
            || ZoomInParamObj["input_tensor_x_offset"].is_null()
            || ZoomInParamObj["input_tensor_y_offset"].is_null()) {
            LOGE("ZoomInParam is null");
            return TS_E_INVALIDARG;
        }

        if (!ZoomInParamObj["input_tensor_x_magnify"].is_number()
            || !ZoomInParamObj["input_tensor_y_magnify"].is_number()
            || !ZoomInParamObj["input_tensor_x_offset"].is_number()
            || !ZoomInParamObj["input_tensor_y_offset"].is_number()) {
            LOGE("magnify or offset is not number");
            return TS_E_INVALIDARG;
        }

        x_magnify = ZoomInParamObj["input_tensor_x_magnify"].number_value();
        y_magnify = ZoomInParamObj["input_tensor_y_magnify"].number_value();
        x_offset = ZoomInParamObj["input_tensor_x_offset"].number_value();
        y_offset = ZoomInParamObj["input_tensor_y_offset"].number_value();
        if (x_magnify < 0 || y_magnify < 0) {
            LOGE("x_magnify or y_magnify must > 0");
            return TS_E_INVALIDARG;
        }

        LOGI("zoomin x_magnify: %f, x_offset: %f, y_magnify: %f, y_offset: %f", x_magnify,
             x_offset, y_magnify, y_offset);
    }

    TS_LPR_log(TS_LPR_LOG_INFO);

    ret = InitLicense(license_path);
    if (ret != 0) {
        LOGE("init license failed, ret: %d", ret);
        return -1;
    }
    LOGI("init license success");

    /* ----------------------------------------------------------- */
    /* Init LPD Detector */
    ret = TS_LPD_init();
    if (ret != TS_OK) {
        LOGE("TS_LPD_init failed, ret: %d", ret);
        return -1;
    }
    LOGI("TS_LPD_init success");

    char *lpd_param_buf{nullptr};
    size_t lpd_param_buf_len{0};

    /* ----------------------------------------------------------- */

    /* ----------------------------------------------------------- */
    /* Init SFI model */
    char *model_buf{nullptr};
    size_t model_buf_len{0};
    ret = ReadFile(model_path.c_str(), &model_buf, &model_buf_len);
    if (ret != 0 || model_buf == nullptr) {
        LOGE("read %s failed", model_path.c_str());
        TS_LPD_deinit();
        return -1;
    }

    ret = TS_LPR_init(model_buf, model_buf_len);
    delete[] model_buf;
    model_buf = nullptr;

    if (ret != TS_OK) {
        LOGE("TS_GAZE_init failed, ret: %d", ret);
        TS_LPR_deinit();
        return -1;
    }

    LOGI("TS_GAZE_init success");

    ret = ReadFile(lpd_param_path.c_str(), &lpd_param_buf, &lpd_param_buf_len);
    if (ret != 0 || lpd_param_buf == nullptr) {
        LOGE("read %s failed", lpd_param_path.c_str());
        TS_LPD_deinit();
        TS_LPR_deinit();
        return -1;
    }

    ret = TS_LPD_setParam(lpd_param_buf);
    delete[] lpd_param_buf;
    lpd_param_buf = nullptr;

    if (ret != TS_OK) {
        LOGE("TS_GAZE_setParam failed, ret: %d", ret);
        TS_LPD_deinit();
        TS_LPR_deinit();
        return -1;
    }
    LOGI("TS_GAZE_setParam success");

    if (!lpr_param_path.empty()) {
        char *lpr_param_data(nullptr);
        size_t lpr_param_data_len = 0;
        // affine file is generated by gaze calibration tool
        ret = ReadFile(lpr_param_path.c_str(), &lpr_param_data, &lpr_param_data_len);
        if (ret == 0 && lpr_param_data != nullptr) {
            ret = TS_LPR_setParam(lpr_param_data);
            delete[] lpr_param_data;
            lpr_param_data = nullptr;
            if (ret == TS_OK) {
                LOGI("TS_LPR_setParam success");
            } else {
                LOGE("TS_LPR_setParam failed, ret: %d", ret);
            }
        } else {
            LOGE("read lpr param %s failed", lpr_param_path.c_str());
        }
    }
    /* ----------------------------------------------------------- */
    // DataTrans data_trans;
    // data_trans.init(mqtt_broker_ip.c_str(), mqtt_broker_port.c_str(), mqtt_sendbuf_size,
    //                 mqtt_recvbuf_size);

    std::thread recv_data_thread(ReceiveDataFromFile);

    static uint32_t img_count = 0;

    // main thread run SFF & SFI & data transmission
    while (true) {
        {
            // timeRecoder tr("process LPR");
            TS_LPR_input_t lpr_input;
            TS_LPR_result_t *lpr_result;
            size_t lpr_result_size;

            TS_LPD_result_t *lpd_result;
            size_t lpd_result_len;
            TensorAndImage data;

            {
                timeRecoder tr("===> pop data");
                LOGI("g_data_queue size: %zu", g_data_queue.size());
                data = g_data_queue.pop(nullptr);
            }
            uint64_t timestamp = data.timestamp;
            uint64_t image_prepare_time_cost = data.image_prepare_time_cost;

            auto start = std::chrono::high_resolution_clock::now();

            LOGI("data image ptr: %p", data.image.image_data);
            TS_LPR_image_t image = data.image;

            lpr_input.image = image;
            lpr_input.camera_id = 0;
            lpr_input.time_stamp = timestamp;

            lpd_result = data.lpd_result;
            lpd_result_len = data.lpd_result_len;
            {
                timeRecoder tr("TS_LPR_run");
                ret = TS_LPR_run(lpr_input, lpd_result, lpd_result_len, &lpr_result,
                                 &lpr_result_size);
                LOGI("TS_LPR_run success");
            }
            if (ret != TS_OK) {
                LOGE("TS_LPR_run failed, ret: %d", ret);
            }

            /* This is just for visualization debug */
            // img_count++;
            // if (draw_result && draw_result_rate > 0) {
            //     if (img_count % draw_result_rate == 0) {
            //         DrawLPRResult(mat, lpr_result, lpr_result_size);
            //         char img_save_path[128] = {0};
            //         sprintf(img_save_path, "%lu_%06d.jpg", timestamp, img_count);
            //         cv::imwrite(img_save_path, mat);
            //     }
            // }

            // for send data
            // {
            //     timeRecoder tr("send_data");
            //     // data transmission with SFI results
            //     data_trans.send_data(mqtt_topic.c_str(), gaze_result, gaze_result_len,
            //     gaze_input,
            //                          data.image_data, data.image_data_size);
            // }
            releaseTensorAndImage(&data);

            TS_LPR_release(lpr_result, lpr_result_size);
            TS_LPD_release(lpd_result, lpd_result_len);

            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast< std::chrono::milliseconds >(stop - start);
            uint64_t lpr_time_cost = duration.count();

            uint64_t max_time_cost = 0;

            if (lpr_time_cost > image_prepare_time_cost) {
                max_time_cost = lpr_time_cost;
            } else {
                max_time_cost = image_prepare_time_cost;
            }

            frame_count++;
            process_time_count = process_time_count + max_time_cost;
            // fps = 1000.0 / (avg of max_time_cost);
            fps = 1000.0 / (( float )process_time_count / frame_count);
            LOGI("prepare data time cost: %ld ms, LPR time cost: %ld ms", image_prepare_time_cost,
                 lpr_time_cost);
            LOGI("frame_count: %lu", frame_count);
            LOGI("LPR running FPS: %f", fps);

            // reset frame_count and process_time_count
            // count 30 frames time cost to calculate FPS
            if (frame_count == 30) {
                frame_count = 0;
                process_time_count = 0;
            }
        }
    }

    recv_data_thread.join();

    TS_LPD_deinit();
    TS_LPR_deinit();

    return 0;
}

static void releaseTensorAndImage(TensorAndImage *tensor_and_image)
{
    // image data will auto release
    if (tensor_and_image) {
        if (tensor_and_image->image.image_data) {
            delete[] tensor_and_image->image.image_data;
            tensor_and_image->image.image_data = nullptr;
        }
    }

    tensor_and_image = nullptr;
}

static int ReceiveDataFromFile()
{
    LOGI("ReceiveDataFromFile");
    std::string cur_date = FetchCurrentDate();
    printf("Current Date: %s\n", cur_date.c_str());
    int ret = 0;

    std::string pre_tensor_dir;
    std::string pre_image_dir;

    while (true) {
        std::vector< std::string > image_list;
        while (true) {
            GetFileList(image_file_dir, image_list, std::vector< std::string >{".nv12"});
            if (!image_list.empty()) {
                break;
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        LOGI("image num: %zu, queue_size: %zu", image_list.size(), g_data_queue.size());

        std::sort(image_list.begin(), image_list.end());

        for (const auto &image_path : image_list) {

            auto start = std::chrono::high_resolution_clock::now();
            /* constuct image path */
            std::string tensor_path = image_path;
            tensor_path.replace(tensor_path.find("Raw"), 3, "InferenceData");

            // for nv12
            tensor_path.replace(tensor_path.find(".nv12"), 4, ".bin");
            tensor_path = tensor_path.substr(0, tensor_path.find_last_of("_")) + ".bin";

            std::string image_name = split_string(image_path, "/").back();
            std::string image_dir = image_path.substr(0, image_path.find_last_of("/"));
            std::string tensor_dir = tensor_path.substr(0, tensor_path.find_last_of("/"));

            // 20220428234546039_4032x3040.nv12
            std::string image_name_without_ext = split_string(image_name, ".").front();
            std::string width_str = split_string(image_name_without_ext, "x").front();
            width_str = split_string(width_str, "_").back();
            std::string height_str = split_string(image_name_without_ext, "x").back();
            int width = std::stoi(width_str);
            int height = std::stoi(height_str);
            LOGI("width: %d, height: %d", width, height);

            TensorAndImage tensor_image;
            TS_LPD_result_t *lpd_result;
            size_t lpd_result_len;
            // 1. read tensor data
            {
                timeRecoder tr("===> read output tensor");
                char *tensor_buf = nullptr;
                size_t tensor_size = 0;
                ret = ReadFileByMMap(tensor_path.c_str(), &tensor_buf, &tensor_size);
                if (ret != 0) {
                    LOGE("read %s failed", tensor_path.c_str());
                    RemoveFile(tensor_path);
                    RemoveFile(image_path);
                    continue;
                }

                float *tensor_float = reinterpret_cast< float * >(tensor_buf);
                size_t tensor_float_size = tensor_size / sizeof(float);

                int ret =
                    TS_LPD_run(tensor_float, tensor_float_size, &lpd_result, &lpd_result_len);
                if (ret != TS_OK) {
                    LOGE("TS_LPD_run failed, ret: %d", ret);
                    RemoveFile(tensor_path);
                    RemoveFile(image_path);
                    continue;
                }

                if (lpd_result_len == 0) {
                    LOGE("lpd_result_len is 0");
                    RemoveFile(tensor_path);
                    RemoveFile(image_path);
                    continue;
                }

                // mmap release
                munmap(tensor_buf, tensor_size);
            }

            char *nv_buf = {0};
            size_t nv_size = 0;
            {
                timeRecoder tr("===> ReadNV12 File");
                LOGI("image_path: %s", image_path.c_str());
                // mmap read nv12 data is faster than read file directly
                ret = ReadFileByMMap(image_path.c_str(), &nv_buf, &nv_size);
                if (ret != 0) {
                    LOGE("read %s failed", tensor_path.c_str());
                    RemoveFile(image_path);
                    continue;
                }
            }

            {
                if (delete_image_tensor) {
                    timeRecoder tr("===> RemoveFile");
                    RemoveFile(tensor_path);
                    RemoveFile(image_path);

                    /* Remove unused directory, like
                     * "/misc/dnn_out/Raw/20220817/0827" */
                    // TODO: Remove /misc/dnn_out/Raw/20220817/
                    if (pre_image_dir.empty()) {
                        pre_image_dir = image_dir;
                        pre_tensor_dir = tensor_dir;
                    } else if (pre_image_dir < image_dir) {
                        if (pre_image_dir.find(image_file_dir) != std::string::npos) {
                            RemoveFile(pre_image_dir);
                        }

                        if (pre_tensor_dir.find(tensor_file_dir) != std::string::npos) {
                            RemoveFile(pre_tensor_dir);
                        }

                        pre_image_dir = image_dir;
                        pre_tensor_dir = tensor_dir;
                    }
                }
            }

            //2. crop nv12 to vehicle size and convert to bgr888
            int crop_x = lpd_result[0].vehicle_bbox.left * x_magnify + x_offset;
            int crop_y = lpd_result[0].vehicle_bbox.top * y_magnify + y_offset;
            int crop_width =
                (lpd_result[0].vehicle_bbox.right - lpd_result[0].vehicle_bbox.left) * x_magnify;
            int crop_height =
                (lpd_result[0].vehicle_bbox.bottom - lpd_result[0].vehicle_bbox.top) * y_magnify;
            // nv12 width and height adjust
            crop_width = (crop_width / 2) * 2;
            crop_height = (crop_height / 2) * 2;

            TS_LPR_image_t input_image = {( unsigned char * )nv_buf, width, height, 1,
                                          TS_PIX_FMT_NV12};
            TS_LPR_image_t output_image;
            output_image.format = TS_PIX_FMT_BGR888;
            {
                timeRecoder tr("===> crop and convert nv12 to bgr888");
                LOGI("crop_x: %d, crop_y: %d, crop_width: %d, crop_height: %d", crop_x, crop_y,
                     crop_width, crop_height);
                TS_LPR_image_crop_and_convert(input_image, &output_image, crop_x, crop_y,
                                              crop_width, crop_height);

                // release nv12 data
                munmap(nv_buf, nv_size);
            }
            tensor_image.image = output_image;

            convertLPDResult(lpd_result, lpd_result_len);
            tensor_image.lpd_result = lpd_result;
            tensor_image.lpd_result_len = lpd_result_len;
            
	    // 3. get timestamp
            std::string UTC_time = split_string(image_name, ".").front();
            UTC_time = split_string(UTC_time, "_").front();
            LOGI("UTC_time: %s", UTC_time.c_str());
            uint64_t timestamp = GetTimeStampFromUTC(UTC_time);
            tensor_image.timestamp = timestamp;
            LOGI("timestamp: %llu\n", timestamp);

            if (g_data_queue.size() >= MAX_QUEUE_SIZE) {
                LOGW("Warning ! drop data from queue\n");
                auto temp_data = g_data_queue.pop(nullptr);
                releaseTensorAndImage(&temp_data);
            }

            LOGI("push data to queue, queue size: %zu", g_data_queue.size());
            printf("\n");
            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast< std::chrono::milliseconds >(stop - start);
            tensor_image.image_prepare_time_cost = duration.count();

            g_data_queue.push(tensor_image);
        }
    }

    return 0;
}

static void convertLPDResult(TS_LPD_result_t *lpd_result, size_t lpd_result_len)
{
    if (lpd_result == nullptr || lpd_result_len == 0) {
        return;
    }

    TS_LPD_result_t *lpd_result_ptr = lpd_result;

    for (size_t i = 0; i < lpd_result_len; i++) {
        TS_LPD_result_t &result = lpd_result_ptr[i];

        result.plate_bbox.left =
            (result.plate_bbox.left - result.vehicle_bbox.left) * x_magnify + x_offset;
        result.plate_bbox.top =
            (result.plate_bbox.top - result.vehicle_bbox.top) * y_magnify + y_offset;
        result.plate_bbox.right =
            (result.plate_bbox.right - result.vehicle_bbox.left) * x_magnify;
        result.plate_bbox.bottom =
            (result.plate_bbox.bottom - result.vehicle_bbox.top) * y_magnify;

        LOGI("after convert plate_bbox: %d, %d, %d, %d", result.plate_bbox.left,
             result.plate_bbox.top, result.plate_bbox.right, result.plate_bbox.bottom);

        result.vehicle_bbox.right =
            (result.vehicle_bbox.right - result.vehicle_bbox.left) * x_magnify + x_offset - 1;
        result.vehicle_bbox.bottom =
            (result.vehicle_bbox.bottom - result.vehicle_bbox.top) * y_magnify + y_offset - 1;
        result.vehicle_bbox.left = 0;
        result.vehicle_bbox.top = 0;

        LOGI("after convert vehicle_bbox: %d, %d, %d, %d", result.vehicle_bbox.left,
             result.vehicle_bbox.top, result.vehicle_bbox.right, result.vehicle_bbox.bottom);
    }
}

static int InitLicense(const std::string &license_path)
{
    int ret = 0;
    char *license_content{nullptr};
    size_t len = 0;
    ret = ReadFile(license_path.c_str(), &license_content, &len);
    if (ret != 0) {
        LOGE("read license %s failed\n", license_path.c_str());
        return -1;
    }
    ret = TS_LPR_add_license(license_content, len);
    delete[] license_content;
    return ret;
}

static bool CheckParams(const json11::Json &root)
{
    if (root.is_null()) {
        return false;
    }

    if (!root["lpd_param_path"].is_string()) {
        LOGE("param_path is invalid!!!");
        return false;
    }

    if (!root["model_path"].is_string()) {
        LOGE("model_path is invalid!!!");
        return false;
    }

    if (!root["lpd_param_path"].is_string()) {
        LOGE("param_path is invalid!!!");
        return false;
    }

    if (root["mqtt_param"].is_null()) {
        LOGE("mqtt_param is null!!!");
        return false;
    }

    return true;
}
