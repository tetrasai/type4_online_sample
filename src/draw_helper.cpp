/**
 * @file draw_helper.cpp
 * @brief Help to draw SDK result for visualization
 * @version 0.1
 * @date 2022-07-15
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "draw_helper.h"

void DrawRect(cv::Mat &mat, TS_LPR_rect_t rect)
{
    cv::rectangle(mat, cv::Point(rect.left, rect.top), cv::Point(rect.right, rect.bottom),
                  cv::Scalar(0, 255, 0), 2, 8, 0);
}

// void DrawPoint(cv::Mat &mat,
//                const flatbuffers::Vector< flatbuffers::Offset< SmartCamera::PersonBodyKeypoint
//                > >
//                    *points_vector)
// {
//     auto size = points_vector->size();
//     for (auto i = 0; i < size; ++i) {
//         auto p_raw_point = points_vector->Get(i)->point();
//         auto ppoint = static_cast< const SmartCamera::Point2d * >(p_raw_point);
//         cv::circle(mat, cv::Point2f(ppoint->x(), ppoint->y()), 2, cv::Scalar(0, 0, 255), 2);
//     }
// }

// void DrawPoint(cv::Mat &mat,
//                const flatbuffers::Vector< flatbuffers::Offset< SmartCamera::PersonFaceKeypoint
//                > >
//                    *points_vector)
// {
//     auto size = points_vector->size();
//     for (auto i = 0; i < size; ++i) {
//         auto p_raw_point = points_vector->Get(i)->point();
//         auto ppoint = static_cast< const SmartCamera::Point2d * >(p_raw_point);
//         cv::circle(mat, cv::Point2f(ppoint->x(), ppoint->y()), 2, cv::Scalar(0, 0, 255), 2);
//     }
// }

static std::string toGender(int n)
{
    switch (n) {
    case 0:
        return "unknow";
    case 1:
        return "Male";
    case 2:
        return "Female";
    default:
        return "invalid";
    }
}

void DrawText(cv::Mat &mat, int text_x, int text_y, std::string text)
{
    int font_face = cv::FONT_HERSHEY_COMPLEX;
    double font_scale = 2;
    int thickness = 1;
    int baseline;
    // 获取文本框的长宽
    cv::Size text_size = cv::getTextSize(text, font_face, font_scale, thickness, &baseline);

    // 将文本框居中绘制
    cv::Point origin;
    origin.x = text_x;
    origin.y = text_y;
    cv::putText(mat, text, origin, font_face, font_scale, cv::Scalar(0, 255, 255), thickness, 8,
                0);
}

void DrawDetectionResult(cv::Mat &mat, TS_LPD_result_t *lpd_result, int LPD_result_len)
{
    if (lpd_result == nullptr || LPD_result_len == 0 || mat.data == nullptr) {
        return;
    }

    for (int i = 0; i < LPD_result_len; i++) {
        TS_LPD_result_t item = lpd_result[i];
        DrawRect(mat, item.vehicle_bbox);
        DrawRect(mat, item.plate_bbox);
    }
}

void DrawLPRResult(cv::Mat &mat, TS_LPR_result_t *lpr_result, int lpr_result_len)
{
    if (lpr_result == nullptr || lpr_result_len == 0 || mat.data == nullptr) {
        return;
    }

    LOGI("lpr_result_len:%d", lpr_result_len);

    for (int i = 0; i < lpr_result_len; i++) {
        DrawDetectionResult(mat, &(lpr_result[i].lpd_result), 1);
        LOGI("texts_array_size:%d", lpr_result[i].texts_array_size);
        std::string p_str;
        for (int j = 0; j < lpr_result[i].texts_array_size; j++) {
            printf("lpr text[%d]:%S", i, lpr_result[i].texts_array[j].text.c_str());
            std::string text_str = lpr_result[i].texts_array[j].text;
            LOGI("text_str:%s", text_str.c_str());
            if (j == 0) {
                p_str = text_str;
            } else {
                p_str = p_str + "&&&" + text_str;
            }
        }

        DrawText(mat, lpr_result[i].lpd_result.vehicle_bbox.right,
                 lpr_result[i].lpd_result.vehicle_bbox.bottom, p_str);
    }
}

// void DrawGazeResult(cv::Mat &mat,
//                     GAZE_serialized_data gaze_serial_result,
//                     size_t gaze_serial_result_len)
// {
//     if (gaze_serial_result == nullptr || mat.data == nullptr) {
//         return;
//     }

//     auto person_analysis_top = SmartCamera::GetPersonAnalysisTop(gaze_serial_result);
//     if (person_analysis_top == nullptr) {
//         return;
//     }

//     // perception result
//     auto perception_result = person_analysis_top->perception();
//     if (perception_result == nullptr) {
//         return;
//     }

//     // person list
//     auto person_list = perception_result->person_list();
//     if (person_list == nullptr) {
//         return;
//     }
//     int person_list_size = person_list->size();

//     LOGI("person_list_size:%d", person_list_size);
//     for (int person_idx = 0; person_idx < person_list_size; ++person_idx) {
//         auto person = person_list->Get(person_idx);
//         if (person == nullptr) {
//             continue;
//         }

//         uint32_t tracking_id = 0;
//         auto tracking = person->tracking();
//         tracking_id = tracking->id();

//         // person face
//         auto person_face = person->face();
//         if (person_face == nullptr) {
//             continue;
//         }

//         // person face rect
//         auto person_face_rect = person_face->bounding_box();
//         auto face_box = static_cast< const SmartCamera::BoundingBox2d * >(person_face_rect);
//         float face_box_score = person_face->bounding_box_score();
//         if (person_face_rect != nullptr) {
//             LOGI("face[%d] face_box:[%d,%d] ~ [%d,%d], score:%f", person_idx, face_box->left(),
//                  face_box->top(), face_box->right(), face_box->bottom(), face_box_score);
//         }

//         // face attribute
//         auto face_attribute = person_face->attribute();
//         if (face_attribute != nullptr) {
//             // age
//             auto age = static_cast< const SmartCamera::PersonFaceAttributeAgeValue * >(
//                 face_attribute->age());
//             float age_score = 0.0f;
//             uint8_t age_val = 0;
//             if (age != nullptr) {
//                 age_val = age->value();
//                 age_score = age->score();
//             }
//             LOGI("face[%d] attribute age:%d, age_score: %f", person_idx, age_val, age_score);

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
//             LOGI("face[%d] attribute gender:%s, gender_score: %f", person_idx,
//             gender_str.c_str(),
//                  gender_score);

//             // mask
//             auto mask = static_cast< const SmartCamera::PersonFaceAttributeMask * >(
//                 face_attribute->mask());
//             float mask_score = 0.0f;
//             std::string mask_str = "unknown";
//             if (mask != nullptr) {
//                 mask_score = mask->score();
//                 if (mask_score > 0) {
//                     mask_str = "mask";
//                 } else {
//                     mask_str = "nomask";
//                 }
//             }
//             LOGI("face[%d] attribute mask:%s, mask_score: %f", person_idx, mask_str.c_str(),
//                  mask_score);

//             // hat
//             auto hat =
//                 static_cast< const SmartCamera::PersonFaceAttributeHat *
//                 >(face_attribute->hat());
//             float hat_score = 0.0f;
//             std::string hat_str = "unknown";
//             if (hat != nullptr) {
//                 hat_score = hat->score();
//                 if (hat_score > 0) {
//                     hat_str = "hat";
//                 } else {
//                     hat_str = "nohat";
//                 }
//             }
//             LOGI("face[%d] attribute hat:%s, hat_score: %f", person_idx, hat_str.c_str(),
//                  hat_score);

//             // glasses
//             auto glasses = static_cast< const SmartCamera::PersonFaceAttributeGlasses * >(
//                 face_attribute->glasses());
//             std::string glasses_str = "unknown";
//             float glasses_score = 0.0f;
//             if (glasses != nullptr) {
//                 float sun_glass_score = glasses->sun_glass_score();
//                 float normal_glass_score = glasses->normal_glass_score();
//                 const float delta = 1e-10;
//                 if (sun_glass_score > delta) {
//                     glasses_str = "sunglasses";
//                     glasses_score = sun_glass_score;
//                 } else if (normal_glass_score > delta) {
//                     glasses_str = "normalglasses";
//                     glasses_score = normal_glass_score;
//                 } else {
//                     glasses_str = "noglasses";
//                     glasses_score = 0.0f;
//                 }
//             }
//             LOGI("face[%d] attribute glasses:%s, glasses_score: %f", person_idx,
//                  glasses_str.c_str(), glasses_score);
//         }

//         auto person_behavior = person->behavior();
//         std::string gaze_str = "no gaze";
//         float gaze_point_x = 0;
//         float gaze_point_y = 0;
//         if (person_behavior != nullptr) {
//             auto gaze = person_behavior->gaze();
//             if (gaze != nullptr) {
//                 bool is_gaze = gaze->is_gaze();
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

//         LOGI("face[%d] gaze is_focus:%s, focus_point:[%f,%f], tracking_id:%d", person_idx,
//              gaze_str.c_str(), gaze_point_x, gaze_point_y, tracking_id);

//         if (person_face_rect != nullptr) {
//             DrawRect(mat, face_box);
//             DrawText(mat, face_box->right(), face_box->top(), gaze_str);
//             std::string id_str = "id:" + std::to_string(tracking_id);
//             DrawText(mat, face_box->right(), face_box->bottom(), id_str);
//         }
//     }
// }
