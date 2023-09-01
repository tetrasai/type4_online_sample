#ifndef __DATA_TRANS_H__
#define __DATA_TRANS_H__

#include "b64.h"
#include "json11.hpp"
#include "mqtt.h"
#include "mqtt_pal.h"
#include <string>

#include "ts_lpr_common.h"

class DataTrans
{
public:
    DataTrans();
    ~DataTrans();

    int init(const char *addr, const char *port, uint32_t sendbuf_size, uint32_t recvbuf_size);
    int send_message(const char *topic, void *message, int message_size);
    // int send_data(const char *topic,
    //               void *message,
    //               int message_size,
    //               TS_GAZE_input_t gaze_input,
    //               void *image_data,
    //               int image_data_size);
    int deinit();

    void set_device_id(const std::string &device_id);
    void set_model_id(const std::string &model_id);

private:
    int release();
    static void publish_callback(void **unused, struct mqtt_response_publish *published);
    static void *client_refresher(void *client);
    static std::string fetch_current_time();

    // std::string data_to_json_str(const char *data,
    //                              int data_size,
    //                              TS_GAZE_input_t gaze_input,
    //                              void *image_data,
    //                              int image_data_size);

private:
    int sockfd_{-1};
    pthread_t client_daemon_;
    struct mqtt_client client_;

    uint8_t *sendbuf_{nullptr};
    uint8_t *recvbuf_{nullptr};

    std::string device_id_{"00000000-0000-2000-8002-e45f0117e6f7"};
    std::string model_id_{"0203020000040000"};
};

#endif