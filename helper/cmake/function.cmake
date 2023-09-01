# 执行传入的command
# IGNORE_ERROR 忽略错误，运行保持继续执行
# 以BASH方式运行(加上参数bash -c)
# OUTPUT 接收命令执行成功的的输出
# ERROR_CODE 接收命令执行失败的的的错误码
# ERROR_MESSAGE 接收命令执行失败的的消息
# WORKING_DIRECTORY 设置运行命令的工作目录
# OUTPUT_FILE 接收命令执行成功的的输出以文件的方式进行，存储在文件OUTPUT_FILE中
# COMMAND 需要执行的命令
FUNCTION(EXCUTE_COMMAND)
    CMAKE_PARSE_ARGUMENTS(CMD "IGNORE_ERROR;BASH" "OUTPUT;ERROR_CODE;ERROR_MESSAGE;WORKING_DIRECTORY;OUTPUT_FILE"
                          "COMMAND" ${ARGN})
    IF(NOT CMD_COMMAND)
        MESSAGE(FATAL_ERROR "no comamnd to excute")
    ENDIF()
    IF(CMD_BASH)
        SET(as_bash bash "-c")
    ENDIF()
    SET(work_dir ${CMAKE_CURRENT_SOURCE_DIR})
    IF(CMD_WORKING_DIRECTORY)
        SET(work_dir ${CMD_WORKING_DIRECTORY})
    ENDIF()
    # MESSAGE(STATUS "excute command ${as_bash} ${CMD_COMMAND} start")
    IF(CMD_OUTPUT_FILE)
        EXECUTE_PROCESS(
            COMMAND ${as_bash} "${CMD_COMMAND}"
            WORKING_DIRECTORY "${work_dir}"
            OUTPUT_FILE "${CMD_OUTPUT_FILE}"
            ERROR_VARIABLE error_message
            OUTPUT_VARIABLE success_message
            RESULT_VARIABLE error_code
            OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE)
        IF(NOT CMD_IGNORE_ERROR)
            IF(NOT error_code EQUAL 0)
                MESSAGE(
                    FATAL_ERROR
                        "excute cmd:${as_bash} ${CMD_COMMAND} faild error code:${error_code} error_message:${error_message}"
                )
            ENDIF()
        ELSE()
            IF(NOT error_code EQUAL 0)
                MESSAGE(
                    AUTHOR_WARNING
                        "excute cmd:${as_bash} ${CMD_COMMAND} faild error code:${error_code} error_message:${error_message}"
                )
            ENDIF()
        ENDIF()
        IF(CMD_OUTPUT)
            SET(${CMD_OUTPUT}
                ${success_message}
                PARENT_SCOPE)
        ENDIF()
        IF(CMD_ERROR_CODE)
            SET(${CMD_ERROR_CODE}
                ${error_code}
                PARENT_SCOPE)
        ENDIF()
        IF(CMD_ERROR_MESSAGE)
            SET(${CMD_ERROR_MESSAGE}
                ${error_message}
                PARENT_SCOPE)
        ENDIF()
    ELSE()
        EXECUTE_PROCESS(
            COMMAND ${as_bash} "${CMD_COMMAND}"
            WORKING_DIRECTORY "${work_dir}"
            OUTPUT_VARIABLE success_message
            ERROR_VARIABLE error_message
            RESULT_VARIABLE error_code
            OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE)
        IF(NOT CMD_IGNORE_ERROR)
            IF(NOT error_code EQUAL 0)
                MESSAGE(
                    FATAL_ERROR
                        "excute cmd:${as_bash} \"${CMD_COMMAND}\" faild error code:${error_code} error_message:${error_message}"
                )
            ENDIF()
        ELSE()
            IF(NOT error_code EQUAL 0)
                MESSAGE(
                    AUTHOR_WARNING
                        "excute cmd:${as_bash} ${CMD_COMMAND} faild error code:${error_code} error_message:${error_message}"
                )
            ENDIF()
        ENDIF()
        IF(CMD_OUTPUT)
            SET(${CMD_OUTPUT}
                ${success_message}
                PARENT_SCOPE)
        ENDIF()
        IF(CMD_ERROR_CODE)
            SET(${CMD_ERROR_CODE}
                ${error_code}
                PARENT_SCOPE)
        ENDIF()
        IF(CMD_ERROR_MESSAGE)
            SET(${CMD_ERROR_MESSAGE}
                ${error_message}
                PARENT_SCOPE)
        ENDIF()
    ENDIF()
ENDFUNCTION()
##########################################################################################
