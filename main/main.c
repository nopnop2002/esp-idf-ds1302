#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "esp_sntp.h"

#include "ds1302.h"

// You have to set these CONFIG value using menuconfig.
#if 0
#define CONFIG_CLK_GPIO		12
#define CONFIG_IO_GPIO		13
#define CONFIG_CE_GPIO		14
#define	CONFIG_TIMEZONE		9
#endif

#if CONFIG_SET_CLOCK
    #define NTP_SERVER CONFIG_NTP_SERVER
#endif
#if CONFIG_GET_CLOCK
    #define NTP_SERVER " "
#endif
#if CONFIG_DIFF_CLOCK
    #define NTP_SERVER CONFIG_NTP_SERVER
#endif

static const char *TAG = "DS1302";

RTC_DATA_ATTR static int boot_count = 0;


void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

static void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    //sntp_setservername(0, "pool.ntp.org");
    ESP_LOGI(TAG, "Your NTP Server is %s", NTP_SERVER);
    sntp_setservername(0, NTP_SERVER);
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_init();
}

static bool obtain_time(void)
{
    ESP_ERROR_CHECK( nvs_flash_init() );
    //tcpip_adapter_init();
    ESP_ERROR_CHECK( esp_netif_init());
    ESP_ERROR_CHECK( esp_event_loop_create_default() );

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    initialize_sntp();

    // wait for time to be set
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    ESP_ERROR_CHECK( example_disconnect() );
    if (retry == retry_count) return false;
    return true;
}

void setClock(void *pvParameters)
{
    // obtain time over NTP
    ESP_LOGI(pcTaskGetName(0), "Connecting to WiFi and getting time over NTP.");
    if(!obtain_time()) {
        ESP_LOGE(pcTaskGetName(0), "Fail to getting time over NTP.");
        while (1) { vTaskDelay(1); }
    }

    // update 'now' variable with current time
    time_t now;
    struct tm timeinfo;
    char strftime_buf[64];
    time(&now);
    now = now + (CONFIG_TIMEZONE*60*60);
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(pcTaskGetName(0), "The current date/time is: %s", strftime_buf);


    // Initialize RTC
    DS1302_Dev dev;
    if (!DS1302_begin(&dev, CONFIG_CLK_GPIO, CONFIG_IO_GPIO, CONFIG_CE_GPIO)) {
        ESP_LOGE(pcTaskGetName(0), "Error: DS1302 begin");
        while (1) { vTaskDelay(1); }
    }
    ESP_LOGI(pcTaskGetName(0), "Set initial date time...");

    /*
    Member    Type Meaning(Range)
    tm_sec    int  seconds after the minute(0-60)
    tm_min    int  minutes after the hour(0-59)
    tm_hour   int  hours since midnight(0-23)
    tm_mday   int  day of the month(1-31)
    tm_mon    int  months since January(0-11)
    tm_year   int  years since 1900
    tm_wday   int  days since Sunday(0-6)
    tm_yday   int  days since January 1(0-365)
    tm_isdst  int  Daylight Saving Time flag	
    */
    ESP_LOGD(pcTaskGetName(0), "timeinfo.tm_sec=%d",timeinfo.tm_sec);
    ESP_LOGD(pcTaskGetName(0), "timeinfo.tm_min=%d",timeinfo.tm_min);
    ESP_LOGD(pcTaskGetName(0), "timeinfo.tm_hour=%d",timeinfo.tm_hour);
    ESP_LOGD(pcTaskGetName(0), "timeinfo.tm_wday=%d",timeinfo.tm_wday);
    ESP_LOGD(pcTaskGetName(0), "timeinfo.tm_mday=%d",timeinfo.tm_mday);
    ESP_LOGD(pcTaskGetName(0), "timeinfo.tm_mon=%d",timeinfo.tm_mon);
    ESP_LOGD(pcTaskGetName(0), "timeinfo.tm_year=%d",timeinfo.tm_year);

    // Set initial date and time
    DS1302_DateTime dt;
    dt.second = timeinfo.tm_sec;
    dt.minute = timeinfo.tm_min;
    dt.hour = timeinfo.tm_hour;
    dt.dayWeek = timeinfo.tm_wday; // 0= Sunday 1 = Monday
    dt.dayMonth = timeinfo.tm_mday;
    dt.month = (timeinfo.tm_mon + 1);
    dt.year = (timeinfo.tm_year + 1900);
    DS1302_setDateTime(&dev, &dt);

    // Check write protect state
    if (DS1302_isWriteProtected(&dev)) {
        ESP_LOGE(pcTaskGetName(0), "Error: DS1302 write protected");
        while (1) { vTaskDelay(1); }
    }

    // Check write protect state
    if (DS1302_isHalted(&dev)) {
        ESP_LOGE(pcTaskGetName(0), "Error: DS1302 halted");
        while (1) { vTaskDelay(1); }
    }
    ESP_LOGI(pcTaskGetName(0), "Set initial date time done");

    // goto deep sleep
    const int deep_sleep_sec = 10;
    ESP_LOGI(pcTaskGetName(0), "Entering deep sleep for %d seconds", deep_sleep_sec);
    esp_deep_sleep(1000000LL * deep_sleep_sec);
}

void getClock(void *pvParameters)
{
    DS1302_Dev dev;
    DS1302_DateTime dt;

    // Initialize RTC
    ESP_LOGI(pcTaskGetName(0), "Start");
    if (!DS1302_begin(&dev, CONFIG_CLK_GPIO, CONFIG_IO_GPIO, CONFIG_CE_GPIO)) {
        ESP_LOGE(pcTaskGetName(0), "Error: DS1302 begin");
        while (1) { vTaskDelay(1); }
    }

    // Initialise the xLastWakeTime variable with the current time.
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while(1) {
        // Get RTC date and time
        if (!DS1302_getDateTime(&dev, &dt)) {
            ESP_LOGE(pcTaskGetName(0), "Error: DS1302 read failed");
        } else {
            ESP_LOGI(pcTaskGetName(0), "%d %02d-%02d-%d %d:%02d:%02d",
                 dt.dayWeek, dt.dayMonth, dt.month, dt.year, dt.hour, dt.minute, dt.second);
        }
        vTaskDelayUntil(&xLastWakeTime, 1000);
    }
}

void diffClock(void *pvParameters)
{
    // obtain time over NTP
    ESP_LOGI(pcTaskGetName(0), "Connecting to WiFi and getting time over NTP.");
    if(!obtain_time()) {
        ESP_LOGE(pcTaskGetName(0), "Fail to getting time over NTP.");
        while (1) { vTaskDelay(1); }
    }

    // update 'now' variable with current time
    time_t now;
    struct tm timeinfo;
    char strftime_buf[64];
    time(&now);
    now = now + (CONFIG_TIMEZONE*60*60);
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%m-%d-%y %H:%M:%S", &timeinfo);
    ESP_LOGI(pcTaskGetName(0), "NTP date/time is: %s", strftime_buf);

    DS1302_Dev dev;
    DS1302_DateTime dt;

    // Initialize RTC
    if (!DS1302_begin(&dev, CONFIG_CLK_GPIO, CONFIG_IO_GPIO, CONFIG_CE_GPIO)) {
        ESP_LOGE(pcTaskGetName(0), "Error: DS1302 begin");
        while (1) { vTaskDelay(1); }
    }

    // Get RTC date and time
    if (!DS1302_getDateTime(&dev, &dt)) {
        ESP_LOGE(pcTaskGetName(0), "Error: DS1302 read failed");
        while (1) { vTaskDelay(1); }
    }

    // update 'rtcnow' variable with current time
    struct tm rtcinfo;
    rtcinfo.tm_sec = dt.second;
    rtcinfo.tm_min = dt.minute;
    rtcinfo.tm_hour = dt.hour;
    rtcinfo.tm_mday = dt.dayMonth;
    rtcinfo.tm_mon = dt.month - 1; // 0 org.
    rtcinfo.tm_year = dt.year - 1900;
    rtcinfo.tm_isdst = -1;
    time_t rtcnow = mktime(&rtcinfo);
    localtime_r(&rtcnow, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%m-%d-%y %H:%M:%S", &timeinfo);
    ESP_LOGI(pcTaskGetName(0), "RTC date/time is: %s", strftime_buf);

    // Get the time difference
    double x = difftime(rtcnow, now);
    ESP_LOGI(pcTaskGetName(0), "Time difference is: %f", x);
    
    while(1) {
        vTaskDelay(1000);
    }
}


void app_main(void)
{
    ++boot_count;
    ESP_LOGI(TAG, "CONFIG_CLK_GPIO = %d", CONFIG_CLK_GPIO);
    ESP_LOGI(TAG, "CONFIG_IO_GPIO = %d", CONFIG_IO_GPIO);
    ESP_LOGI(TAG, "CONFIG_CE_GPIO = %d", CONFIG_CE_GPIO);
    ESP_LOGI(TAG, "CONFIG_TIMEZONE= %d", CONFIG_TIMEZONE);
    ESP_LOGI(TAG, "Boot count: %d", boot_count);

#if CONFIG_SET_CLOCK
    // Set clock & Get clock
    if (boot_count == 1) {
        xTaskCreate(setClock, "setClock", 1024*4, NULL, 2, NULL);
    } else {
        xTaskCreate(getClock, "getClock", 1024*4, NULL, 2, NULL);
    }
#endif

#if CONFIG_GET_CLOCK
    // Get clock
    xTaskCreate(getClock, "getClock", 1024*4, NULL, 2, NULL);
#endif

#if CONFIG_DIFF_CLOCK
    // Diff clock
    xTaskCreate(diffClock, "diffClock", 1024*4, NULL, 2, NULL);
#endif
}

