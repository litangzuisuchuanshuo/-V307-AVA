#include "debug.h"
#include "string.h"
#include "lcd.h"
#include "stcSTC11L08XE.h"
/* Wi-Fi 名称 和 密码 */
#define SSID ("up")
#define PASSWORD ("12345678")

/* Global define */
#define RXBUF_SIZE 1024 // DMA buffer size
#define size(a) (sizeof(a) / sizeof(*(a)))
/* Global Variable */
u8 TxBuffer[] = " ";
u8 RxBuffer[RXBUF_SIZE] = {0};
#define apiEndpoint "https://api.openai.com/v1/chat/completions";
#define apiKey ="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";
//ChatGPT prompt
#define prompt "what can I help you with today?";

/*******************************************************************************
 * Function Name  : USARTx_CFG
 * Description    : Initializes the USART6 peripheral.
 * 描述   ：   串口初始化
 * Input          : None
 * Return         : None
 *******************************************************************************/
void USARTx_CFG(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    // 开启时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART6, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    /* USART6 TX-->C0  RX-->C1 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // RX，输入上拉
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200;                                    // 波特率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     // 数据位 8
    USART_InitStructure.USART_StopBits = USART_StopBits_1;                          // 停止位 1
    USART_InitStructure.USART_Parity = USART_Parity_No;                             // 无校验
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件流控
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;                 // 使能 RX 和 TX

    USART_Init(UART6, &USART_InitStructure);
    DMA_Cmd(DMA2_Channel7, ENABLE); // 开启接收 DMA
    USART_Cmd(UART6, ENABLE);       // 开启UART
}

/*******************************************************************************
 * Function Name  : DMA_INIT
 * Description    : Configures the DMA.
 * 描述   ：   DMA 初始化
 * Input          : None
 * Return         : None
 *******************************************************************************/
void DMA_INIT(void)
{
    //...

    // RX DMA 初始化，环形缓冲区自动接收
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)RxBuffer; // 接收缓冲区
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;    // 方向 : 外设 作为 源，即 内存 <- 外设
    DMA_InitStructure.DMA_BufferSize = RXBUF_SIZE;        // 缓冲区长度为 RXBUF_SIZE
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;       // 循环模式，构成环形缓冲区
    DMA_Init(DMA2_Channel7, &DMA_InitStructure);
}

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t real_size = size * nmemb;
    char *response_buffer = (char *)userp;

    memcpy(response_buffer, contents, real_size);

    return real_size;
}

/**
 * Function for performing HTTP POST request
 */
static void perform_post_request(const char *url, const char *data, char *response_buffer, size_t response_buffer_size)
{
    CURL *curl = curl_easy_init();
    CURLcode res;

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, response_buffer);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
    }

    strncpy(response_buffer + response_buffer_size - 1, "\0", 1);
}

/**
 * Function for converting speech to text using OpenAI ChatGPT API
 */
static char* convert_speech_to_text(const char *speech)
{
    // Prepare the request data
    json_object *request_data = json_object_new_object();
    json_object *requests_array = json_object_new_array();
    json_object *request_obj = json_object_new_object();
    json_object *model_obj = json_object_new_string("gpt-3.5-turbo");

    json_object_object_add(request_obj, "model", model_obj);
    json_object_object_add(request_obj, "prompt", json_object_new_string(speech));
    json_object_object_add(requests_array, NULL, request_obj);
    json_object_object_add(request_data, "requests", requests_array);

    const char *request_json = json_object_to_json_string(request_data);

    // Perform the HTTP POST request
    char response_buffer[4096];
    perform_post_request(apiEndpoint, request_json, response_buffer, sizeof(response_buffer));

    // Parse the response
    json_object *response = json_tokener_parse(response_buffer);
    json_object *choices_array;
    json_object_object_get_ex(response, "choices", &choices_array);
    json_object *choices = json_object_array_get_idx(choices_array, 0);
    const char *chat_text = json_object_get_string(json_object_object_get(choices, "text"));

    // Clean up and return the result
    json_object_put(response);

    char *result = (char *)malloc(strlen(chat_text) + 1);
    strncpy(result, chat_text, strlen(chat_text) + 1);

    return result;
}

/**
 * Main program
 */
int main(void)
{
    USARTx_CFG();
    DMA_INIT();

    while (1)
    {
        // Read data from the receive buffer
        uint16_t num = uartAvailableWiFi();
        if (num > 0)
        {
            uartReadWiFi(buffer, num);
            buffer[num] = '\0';
            printf("\r\nReceived %d bytes :\r\n%s\r\n", num, buffer);

            // Convert speech to text
            char *text = convert_speech_to_text(buffer);

            // Display the converted text on the LCD
            lcd_show_string(0, 90, 24, text);

            // Clean up
            free(text);
        }
    }
}
