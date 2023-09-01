#ifndef TS_LPR_PERCEPTION_H_
#define TS_LPR_PERCEPTION_H_

/*!\addtogroup perception
 *  @{
 */

#include <stddef.h>

#include "ts_lpr_common.h"

/**
 * @brief  Add product license. Make sure call this interface before any other
 * interfaces
 * @note
 * @param[in]  ptr:  memory buffer of license
 * @param[in]  len: buffer's length
 * @retval if success, return TS_OK. otherwise, return error code
 */
TS_SDK_API ts_result_code TS_LPR_add_license(const void *ptr, int len);

/**
 * @brief  set sdk's log level
 * @note
 * @param[in]  level: default is TS_LPR_LOG_DEBUG, see detail in
 * TS_LPR_log_level definition
 * @retval if success, return TS_OK. otherwise, return error code
 */
TS_SDK_API ts_result_code TS_LPR_log(TS_LPR_log_level level);

/**
 * @brief  get sdk's version information
 * @note
 * @retval sdk version
 */
TS_SDK_API const char *TS_LPR_sdk_version();

/**
 * @brief   initialize LPD(Car Detection, Plate Detetcion, Plate Landmark
 * Detection) module
 * @retval if success, return TS_OK. otherwise, return error code
 */
TS_SDK_API ts_result_code TS_LPD_init();

/**
 * @brief Set parameters for LPD module
 * @note it could be called before TS_LPD_run. If not, SDK would use the
 * internal default parameters.This json string should like this:
 * {
 *  "vehicle_bbox_confidence_score_threshold": 0.3,
 *  "plate_bbox_confidence_score_threshold": 0.3,
 *  "vehicle_bbox_nms_iou_threshold": 0.5
 *  }
 *
 * vehicle_bbox_confidence_score_threshold: threshold for adjust vehicle
 * detection bbox recall/precision, range: (0.0-1.0)
 * plate_bbox_confidence_score_threshold: threshold for adjust plate detection
 * bbox recall/precision, range: (0.0-1.0)
 * vehicle_bbox_nms_iou_threshold: threshold for reducing repeated vehicle bbox,
 * affect the recall/precision, range: (0.0-1.0)
 *
 * @param[in] json_buffer: json buffer
 * @retval if success, return TS_OK. otherwise, return error code
 */
TS_SDK_API ts_result_code TS_LPD_setParam(const char *json_buffer);

/**
 * @brief  Get LPD result
 * @note
 * @param[in]  output_tensor: output tensor
 * @param[in]  output_tensor_size: the data size of output tensor
 * @param[in]  lpd_result_array: Array of LPD results whose length is equal to
 * lpd_result_array
 * @param[in]  lpd_result_len: size of lpd_result_array
 * @retval if success, return TS_OK. otherwise, return error code
 */
TS_SDK_API ts_result_code TS_LPD_run(const float *output_tensor,
                                     size_t output_tensor_size,
                                     TS_LPD_result_t **lpd_result_array,
                                     size_t *lpd_result_len);

/**
 * @brief  Release the memory allocated by function TS_LPD_run when you don't
 * need it anymore
 * @note
 * @param[in]  lpd_result_array: memory allocated by function TS_LPD_run
 * @param[in]  lpd_result_len: lpd result array size
 * @retval None
 */
TS_SDK_API void TS_LPD_release(TS_LPD_result_t *lpd_result_array,
                               size_t lpd_result_len);

/**
 * @brief  release the whole resources of LPD module
 * @note
 * @retval None
 */
TS_SDK_API void TS_LPD_deinit();

/**
 * @brief  initialize LPR(License Plate OCR recongintion) module
 * @note
 * @param[in]  modelptr: License plate OCR model's binary buffer
 * @param[in]  model_len: License plate OCR model's binary buffer length
 * @retval if success, return TS_OK. otherwise, return error code
 */
TS_SDK_API ts_result_code TS_LPR_init(const void *modelptr, int model_len);

/**
 * @brief Set parameters for LPR module
 * @note it could be called before TS_LPR_run. If not, SDK would use the
 * internal default parameters. This json string should like this:
 * {
 *   "ZoomInParam": {
 *      "input_tensor_x_magnify": 3.16875,
 *      "input_tensor_y_magnify": 3.16667,
 *      "input_tensor_x_offset": -54,
 *      "input_tensor_y_offset": -220
 *   },
 *   "plate_quality_score_threshold": 0.3,
 *   "crop_vehicle_image_mode": 0
 * }
 *
 * ZoomInParam: zoom in paramters which used in IE version, the default value
 * showed above represent no zoom (VGA to FHD).
 * plate_quality_score_threshold: threshold for plate image quality(clarity)
 * score, range: (0.0-1.0). If plate quality score is higher than threshold will
 * do License plate recognition in API 'TS_LPR_run', otherwise will not do.
 * crop_vehicle_image_mode: Is crop vehicle image enabled, '0' means crop
 * vehicle image and output, '1' means not, range:[0, 1]
 *
 * @param[in] json_buffer: json buffer
 * @retval if success, return TS_OK. otherwise, return error code
 */
TS_SDK_API ts_result_code TS_LPR_setParam(const char *json_buffer);

/**
 * @brief  Get LPR result
 * @note
 * @param[in]  input_image: high resolution image
 * @param[in]  lpd_result_array: Array of LPD results whose length is equal to
 * LPD_array_length
 * @param[in]  LPD_array_length: length for lpd_results_array
 * @param[out] LPR_result_array: Array of LPR results
 * @param[out] LPR_result_array_length:  LPR result array size
 * @retval if success, return TS_OK. otherwise, return error code
 */
TS_SDK_API ts_result_code TS_LPR_run(const TS_LPR_input_t input_image,
                                     TS_LPD_result_t *lpd_result_array,
                                     size_t lpd_result_array_len,
                                     TS_LPR_result_t **lpr_result_array,
                                     size_t *lpr_result_array_len);

/**
 * @brief  fast crop and convert image
 * @note
 * @param[in]  input_image: input image
 * @param[out] output_image: cropped image
 * @param[in]  crop_x: crop x
 * @param[in]  crop_y: crop y
 * @param[in]  crop_w: crop width
 * @param[in]  crop_h: crop height
 * @retval if success, return TS_OK. otherwise, return error code
 */
TS_SDK_API ts_result_code TS_LPR_image_crop_and_convert(
    const TS_LPR_image_t input_image, TS_LPR_image_t *output_image, int crop_x,
    int crop_y, int crop_w, int crop_h);

/**
 * @brief  Release the memory allocated by function TS_LPR_run
 * @note
 * @param[in]  LPR_results_array: memory allocated by function TS_LPR_run
 * @param[in]  LPR_results_data_length: length of array LPR_results_array
 * @retval None
 */
TS_SDK_API void TS_LPR_release(TS_LPR_result_t *lpr_results_array,
                               size_t lpr_results_array_len);

/**
 * @brief  release the whole resources of LPR module
 * @note
 * @retval None
 */
TS_SDK_API void TS_LPR_deinit();

/*!@} */
#endif