#include "GalvoParser.h"

// 协议解析状态机状态
// 数据格式 FF FF <点数量N>2byte <点数据>4byte * N
// 例如： FF FF 00 02 AA AA BB BB 12 34 98 76表示发送2个点，第1个点的坐标X轴为AA AA ，Y轴为BB BB；第2个点的坐标X轴为12 34，Y轴为98 76
typedef enum
{
    STATE_HEADER_1,
    STATE_HEADER_2,
    STATE_POINT_COUNT_1,
    STATE_POINT_COUNT_2,
    STATE_DATA
} parser_state_t;

// 双缓冲区静态分配（ESP32内部SRAM，速度最快）
static point_t frame_buffer_A[MAX_POINTS_PER_FRAME];
static point_t frame_buffer_B[MAX_POINTS_PER_FRAME];

// 核心业务指针（使用 volatile 确保多核/中断可见性）
static point_t *volatile dac_render_ptr = frame_buffer_A;
static uint32_t volatile dac_render_count = 0;

static point_t *parser_write_ptr = frame_buffer_B;

// 双缓冲切换的自旋锁（静态分配，跨核可见）
static portMUX_TYPE buffer_switch_mutex = portMUX_INITIALIZER_UNLOCKED;

/**
 * 切换双缓冲（由解析任务调用，使用 task 级临界区）
 */
static void switch_ping_pong_buffer(uint32_t point_count)
{
    portENTER_CRITICAL(&buffer_switch_mutex);

    if (parser_write_ptr == frame_buffer_B)
    {
        dac_render_ptr = frame_buffer_B;
        parser_write_ptr = frame_buffer_A;
    }
    else
    {
        dac_render_ptr = frame_buffer_A;
        parser_write_ptr = frame_buffer_B;
    }
    dac_render_count = point_count;

    portEXIT_CRITICAL(&buffer_switch_mutex);
}

/**
 * 流式字节状态机解析
 */
static void parse_serial_stream(uint8_t *data, size_t len)
{
    static parser_state_t state = STATE_HEADER_1;
    static uint16_t expect_points = 0;
    static uint32_t current_point_idx = 0;
    static uint8_t byte_count = 0;
    static uint8_t temp_buf[4] = {0};

    for (size_t i = 0; i < len; i++)
    {
        uint8_t b = data[i];

        switch (state)
        {
        case STATE_HEADER_1:
            if (b == 0xFF)
                state = STATE_HEADER_2;
            break;

        case STATE_HEADER_2:
            if (b == 0xFF)
                state = STATE_POINT_COUNT_1;
            else
                state = STATE_HEADER_1;
            break;

        case STATE_POINT_COUNT_1:
            temp_buf[0] = b;
            state = STATE_POINT_COUNT_2;
            break;

        case STATE_POINT_COUNT_2:
            temp_buf[1] = b;
            // 默认发送端为大端序
            expect_points = (temp_buf[0] << 8) | temp_buf[1];

            current_point_idx = 0;
            byte_count = 0;

            if (expect_points > 0 && expect_points <= MAX_POINTS_PER_FRAME)
            {
                state = STATE_DATA;
            }
            else
            {
                state = STATE_HEADER_1; // 点数异常，丢弃本帧
            }
            break;

        case STATE_DATA:
            temp_buf[byte_count++] = b;

            if (byte_count == 4)
            {
                // 大端序解包成 uint16
                uint16_t x = (temp_buf[0] << 8) | temp_buf[1];
                uint16_t y = (temp_buf[2] << 8) | temp_buf[3];

                parser_write_ptr[current_point_idx].x = x;
                parser_write_ptr[current_point_idx].y = y;

                current_point_idx++;
                byte_count = 0;

                if (current_point_idx >= expect_points)
                {
                    switch_ping_pong_buffer(expect_points);
                    state = STATE_HEADER_1;
                }
            }
            break;
        }
    }
}

/**
 * 独立的后台串口接收任务 (避免阻塞Arduino的loop)
 */
static void uart_parser_task(void *pvParameters)
{
    // 这里的接收缓冲区大小要能装得下高频大块数据
    uint8_t rx_buf[512];

    while (1)
    {
        // 检查硬件串口是否有数据
        int available_bytes = Serial.available();
        if (available_bytes > 0)
        {
            int bytes_to_read = min(available_bytes, (int)sizeof(rx_buf));
            int len = Serial.readBytes(rx_buf, bytes_to_read);
            if (len > 0)
            {
                parse_serial_stream(rx_buf, len);
            }
        }
        // 交出CPU控制权 2ms，让给其他任务或系统后台
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}

void galvoParser_init()
{
    // 创建一个常驻后台的FreeRTOS任务专门跑串口解析，优先级设为 5 (高于loop的1)
    xTaskCreatePinnedToCore(
        uart_parser_task, // 任务函数
        "UartParserTask", // 任务名
        4096,             // 栈大小
        NULL,             // 参数
        5,                // 优先级
        NULL,             // 任务句柄
        0                 // 绑定到核心 0（让出核心1专门跑Arduino loop/中断）
    );
}

void galvoParser_getRenderFrame(point_t **out_points, uint32_t *out_count)
{
    // 使用 ISR 版本临界区，与解析任务的写操作跨核互斥
    portENTER_CRITICAL_ISR(&buffer_switch_mutex);
    *out_points = dac_render_ptr;
    *out_count = dac_render_count;
    portEXIT_CRITICAL_ISR(&buffer_switch_mutex);
}