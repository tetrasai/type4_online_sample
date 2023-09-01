/**
 * @file data_trans.cpp
 * @brief DataTransmission implemented by MQTT-C [https://github.com/LiamBindle/MQTT-C]
 * @version 0.1
 * @date 2022-08-02
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <algorithm>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include "data_trans.hpp"
#include "json11.hpp"
#include "posix_sockets.h"
#include "utils.h"

// #include "person_analysis_top_generated.h"

DataTrans::DataTrans() : sockfd_(-1), sendbuf_(nullptr), recvbuf_(nullptr) {}

DataTrans::~DataTrans()
{
    deinit();
}

std::string DataTrans::fetch_current_time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    time_t sec = static_cast< time_t >(tv.tv_sec);
    int ms = static_cast< int >(tv.tv_usec) / 1000;

    struct tm *local = localtime(&sec);
    char time_buf[64] = {0};
    strftime(time_buf, 64, "%Y%m%d%H%M%S", local);

    char ms_buf[4] = {0};
    snprintf(ms_buf, sizeof(ms_buf), "%03d", ms);

    return std::string(time_buf) + std::string(ms_buf);
}

void DataTrans::set_device_id(const std::string &device_id)
{
    device_id_ = device_id;
}

void DataTrans::set_model_id(const std::string &model_id)
{
    model_id_ = model_id;
}

int DataTrans::release()
{
    if (sockfd_ != -1) {
        close(sockfd_);
        sockfd_ = -1;
    }

    if (sendbuf_ != nullptr) {
        delete[] sendbuf_;
        sendbuf_ = nullptr;
    }

    if (recvbuf_ != nullptr) {
        delete[] recvbuf_;
        recvbuf_ = nullptr;
    }

    return 0;
}

int DataTrans::init(const char *addr,
                    const char *port,
                    uint32_t sendbuf_size,
                    uint32_t recvbuf_size)
{
    if (sendbuf_ != nullptr || recvbuf_ != nullptr) {
        LOGE("sendbuf or recvbuf is already init");
        return -1;
    }

    if (sendbuf_size == 0 || recvbuf_size == 0) {
        LOGE("invalid args, buf size is 0");
        return -1;
    }

    sockfd_ = open_nb_socket(addr, port);
    if (sockfd_ == -1) {
        LOGE("open socket failed: %s:%s", addr, port);
        return -1;
    }

    sendbuf_ = new (std::nothrow) uint8_t[sendbuf_size];
    if (sendbuf_ == nullptr) {
        LOGE("OOM");
        release();
        return -1;
    }

    recvbuf_ = new (std::nothrow) uint8_t[recvbuf_size];
    if (recvbuf_ == nullptr) {
        LOGE("OOM");
        release();
        return -1;
    }

    MQTTErrors ret = mqtt_init(&client_, sockfd_, sendbuf_, sendbuf_size, recvbuf_, recvbuf_size,
                               &publish_callback);
    if (ret != MQTT_OK) {
        LOGE("mqtt_init failed, ret: %d", ret);
        release();
        return -1;
    }

    /* Create an anonymous session */
    char *client_id = NULL;
    /* Ensure we have a clean session */
    uint8_t connect_flags = MQTT_CONNECT_CLEAN_SESSION;
    /* Send connection request to the broker. */
    mqtt_connect(&client_, client_id, NULL, NULL, 0, NULL, NULL, connect_flags, 400);
    if (client_.error != MQTT_OK) {
        LOGE("mqtt_connect failed: %s", mqtt_error_str(client_.error));
        release();
        return -1;
    }

    if (pthread_create(&client_daemon_, NULL, client_refresher, &client_)) {
        LOGE("Failed to start client daemon.");
        release();
        return -1;
    }

    LOGI("DataTrans initialize complete!!!");
    return 0;
}

int DataTrans::deinit()
{
    release();

    if (client_daemon_ > 0) {
        pthread_cancel(client_daemon_);
    }

    return 0;
}

void *DataTrans::client_refresher(void *client)
{
    while (1) {
        mqtt_sync(( struct mqtt_client * )client);
        usleep(20000U);
    }

    return NULL;
}

void DataTrans::publish_callback(void **unused, struct mqtt_response_publish *published) {}

int DataTrans::send_message(const char *topic, void *message, int message_size)
{
    if (topic == nullptr || message == nullptr || message_size <= 0) {
        LOGE("invalid args, topic or message is NULL");
        return -1;
    }

    LOGI("send message topic: %s", topic);
    mqtt_publish(&client_, topic, message, message_size, MQTT_PUBLISH_QOS_0);

    if (client_.error != MQTT_OK) {
        LOGE("mqtt_publish failed: %s\n", mqtt_error_str(client_.error));
        return -1;
    }

    return 0;
}

// int DataTrans::send_data(const char *topic,
//                          void *message,
//                          int message_size,
//                          TS_GAZE_input_t gaze_input,
//                          void *image_data,
//                          int image_data_size)
// {
//     if (topic == nullptr) {
//         LOGE("invalid args, topic is NULL");
//         return -1;
//     }

//     std::string json_str = data_to_json_str(( const char * )message, message_size, gaze_input,
//                                             image_data, image_data_size);
//     if (json_str.empty()) {
//         LOGE("SFI_data_to_json failed");
//         return -1;
//     }

//     LOGI("message topic: %s", topic);

//     mqtt_publish(&client_, topic, json_str.c_str(), json_str.size(), MQTT_PUBLISH_QOS_0);
//     if (client_.error != MQTT_OK) {
//         LOGE("mqtt_publish failed: %s\n", mqtt_error_str(client_.error));
//         return -1;
//     }

//     return 0;
// }

// std::string DataTrans::data_to_json_str(const char *data,
//                                         int data_size,
//                                         TS_GAZE_input_t gaze_input,
//                                         void *image_data,
//                                         int image_data_size)
// {
//     if (data == nullptr || data_size <= 0) {
//         LOGE("invalid args, data is NULL or data_size <= 0");
//         return "";
//     }

//     auto person_analysis_top = SmartCamera::GetPersonAnalysisTop(data);
//     if (person_analysis_top == nullptr) {
//         return "";
//     }

//     // perception result
//     auto perception_result = person_analysis_top->perception();
//     if (perception_result == nullptr) {
//         return "";
//     }

//     // person list
//     auto person_list = perception_result->person_list();
//     if (person_list == nullptr) {
//         return "";
//     }
//     int person_list_size = person_list->size();

//     LOGI("person_list_size:%d", person_list_size);

//     json11::Json::object json_obj;
//     json_obj["DeviceID"] = device_id_;
//     json_obj["ModelID"] = model_id_;
//     // json_obj["Image"] = false;

//     json11::Json::array infer_array;
//     json11::Json::object infer_obj;

//     // std::string timestamp = fetch_current_time();
//     json_obj["timestamp"] = std::to_string(gaze_input.time_stamp);
//     LOGD("timestamp:%s", json_obj["timestamp"].string_value().c_str());
//     json_obj["image_width"] = gaze_input.image.width;
//     json_obj["image_height"] = gaze_input.image.height;

//     if (image_data != nullptr) {
//         LOGI("image_data_size:%d", image_data_size);
//         char *encoded_data = b64_encode(( const unsigned char * )image_data, image_data_size);
//         json_obj["image_data"] = encoded_data;

//         if (encoded_data != nullptr) {
//             free(encoded_data);
//             encoded_data = nullptr;
//         }
//     }

//     for (int person_idx = 0; person_idx < person_list_size; ++person_idx) {
//         auto person = person_list->Get(person_idx);
//         if (person == nullptr) {
//             continue;
//         }

//         // person face
//         auto person_face = person->face();
//         if (person_face == nullptr) {
//             continue;
//         }

//         // person face rect
//         auto person_face_rect = person_face->bounding_box();
//         auto face_box = static_cast< const SmartCamera::BoundingBox2d * >(person_face_rect);
//         float face_box_score = person_face->bounding_box_score();
//         json11::Json::object face_obj;
//         if (person_face_rect != nullptr) {
//             LOGI("face[%d] face_box:[%d,%d] ~ [%d,%d], score:%f", person_idx, face_box->left(),
//                  face_box->top(), face_box->right(), face_box->bottom(), face_box_score);
//             face_obj["score"] = face_box_score;
//             face_obj["left"] = face_box->left();
//             face_obj["top"] = face_box->top();
//             face_obj["right"] = face_box->right();
//             face_obj["bottom"] = face_box->bottom();
//         }
//         infer_obj["face_rect"] = face_obj;

//         json11::Json::object attribute_obj;
//         auto face_attribute = person_face->attribute();
//         if (face_attribute != nullptr) {
//             // age
//             auto age = static_cast< const SmartCamera::PersonFaceAttributeAgeValue * >(
//                 face_attribute->age());
//             uint8_t age_val = 0;
//             if (age != nullptr) {
//                 age_val = age->value();
//             }
//             attribute_obj["age"] = age_val;

//             // gender
//             auto gender = static_cast< const SmartCamera::PersonFaceAttributeGenderClass * >(
//                 face_attribute->gender());
//             std::string gender_str = "unknown";
//             float gender_score = 0.0f;
//             if (gender != nullptr) {
//                 auto gender_type = gender->type();
//                 if (gender_type
//                     == SmartCamera::PersonFaceAttributeGenderClassType::
//                         PersonFaceAttributeGenderClassType_Male) {
//                     gender_str = "male";
//                 } else if (gender_type
//                            == SmartCamera::PersonFaceAttributeGenderClassType::
//                                PersonFaceAttributeGenderClassType_Female) {
//                     gender_str = "female";
//                 } else {
//                     gender_str = "InvalidGender";
//                 }
//                 gender_score = gender->score();
//             }
//             attribute_obj["gender"] = gender_str;
//             attribute_obj["gender_score"] = gender_score;
//             LOGI("face[%d] age:%d, gender:%s, gender_score:%f", person_idx, age_val,
//                  gender_str.c_str(), gender_score);
//         }

//         infer_obj["attribute"] = attribute_obj;

//         auto person_behavior = person->behavior();
//         json11::Json::object gaze_obj;
//         std::string gaze_str = "no gaze";
//         bool is_gaze = false;
//         float gaze_point_x = 0;
//         float gaze_point_y = 0;
//         if (person_behavior != nullptr) {
//             auto gaze = person_behavior->gaze();
//             if (gaze != nullptr) {
//                 is_gaze = gaze->is_gaze();
//                 auto gaze_point = gaze->gaze_point();
//                 auto gaze_point_2d = static_cast< const SmartCamera::Point2d * >(gaze_point);
//                 if (is_gaze && gaze_point != nullptr) {
//                     gaze_str = "is gaze";
//                     gaze_point_x = gaze_point_2d->x();
//                     gaze_point_y = gaze_point_2d->y();

//                 } else {
//                     gaze_str = "no gaze";
//                 }
//             }
//         }
//         gaze_obj["is_gaze"] = is_gaze;
//         gaze_obj["gaze_point_x"] = gaze_point_x;
//         gaze_obj["gaze_point_y"] = gaze_point_y;
//         infer_obj["gaze"] = gaze_obj;

//         LOGI("face[%d] is_gaze:%s, gaze_point:[%f,%f]", person_idx, gaze_str.c_str(),
//              gaze_point_x, gaze_point_y);
//         infer_array.push_back(infer_obj);
//     }

//     json_obj["Inferences"] = infer_array;

//     return json11::Json(json_obj).dump();
// }