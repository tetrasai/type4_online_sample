#include <stdio.h>
#include <string>
#include <vector>

#include "ts_lpr_perception.h"

#include "ts_lpr_common.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include "utils.h"

void DrawRect(cv::Mat &mat, TS_LPR_rect_t rect);

void DrawText(cv::Mat &mat, int text_x, int text_y, std::string text);

void DrawDetectionResult(cv::Mat &mat, TS_LPD_result_t *lpd_result, int LPD_result_len);

void DrawLPRResult(cv::Mat &mat, TS_LPR_result_t *lpr_result, int lpr_result_len);

// void DrawPoint(cv::Mat &mat,
//                const flatbuffers::Vector< flatbuffers::Offset<
//                SmartCamera::PersonBodyKeypoint > >
//                    *points_vector);
// void DrawPoint(cv::Mat &mat,
//                const flatbuffers::Vector< flatbuffers::Offset<
//                SmartCamera::PersonFaceKeypoint > >
//                    *points_vector);
// void DrawDetectionResult(cv::Mat &mat, SFF_serialized_data SFF_result, size_t
// SFF_result_len);
