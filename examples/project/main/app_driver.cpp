/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <esp_log.h>
#include <stdlib.h>
#include <string.h>
#include <device.h>
#include <esp_matter.h>
#include <led_driver.h>
#include <app_priv.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ssd1306.h"
#include "bh1750.h"
#include <math.h>


using namespace chip::app::Clusters;
using namespace esp_matter;

static const char *TAG = "app_driver";
extern uint16_t light_endpoint_id;
extern uint16_t light_sensor_endpoint_id;
float lux = 0.0; 

/* Do any conversions/remapping for the actual value here */
static esp_err_t app_driver_light_set_power(led_driver_handle_t handle, esp_matter_attr_val_t *val)
{
    return led_driver_set_power(handle, val->val.b);
}

static esp_err_t app_driver_light_set_brightness(led_driver_handle_t handle, esp_matter_attr_val_t *val)
{
    int value = REMAP_TO_RANGE(val->val.u8, MATTER_BRIGHTNESS, STANDARD_BRIGHTNESS);
    return led_driver_set_brightness(handle, value);
}

static esp_err_t app_driver_light_set_hue(led_driver_handle_t handle, esp_matter_attr_val_t *val)
{
    int value = REMAP_TO_RANGE(val->val.u8, MATTER_HUE, STANDARD_HUE);
    return led_driver_set_hue(handle, value);
}

static esp_err_t app_driver_light_set_saturation(led_driver_handle_t handle, esp_matter_attr_val_t *val)
{
    int value = REMAP_TO_RANGE(val->val.u8, MATTER_SATURATION, STANDARD_SATURATION);
    return led_driver_set_saturation(handle, value);
}

static esp_err_t app_driver_light_set_temperature(led_driver_handle_t handle, esp_matter_attr_val_t *val)
{
    uint32_t value = REMAP_TO_RANGE_INVERSE(val->val.u16, STANDARD_TEMPERATURE_FACTOR);
    return led_driver_set_temperature(handle, value);
}

/*
static void app_driver_light_sensor_set_illuminance_cb(void *arg, void *data){
    ESP_LOGI(TAG, "Illuminance data read");
    uint16_t endpoint_id = light_endpoint_id;
    uint32_t cluster_id = IlluminanceMeasurement::Id;
    uint32_t attribute_id = IlluminanceMeasurement::Attributes::MeasuredValue::Id;

    node_t *node = node::get();
    endpoint_t *endpoint = endpoint::get(node, endpoint_id);
    cluster_t *cluster = cluster::get(endpoint, cluster_id);
    attribute_t *attribute = attribute::get(cluster, attribute_id);

    esp_matter_attr_val_t val = esp_matter_invalid(NULL);
    attribute::get_val(attribute, &val);
    val.val.u16 = !val.val.b; //replace with read new value of illuminance and also update data type = u16
    attribute::update(endpoint_id, cluster_id, attribute_id, &val);
}
*/
static void app_driver_button_toggle_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "Toggle button pressed");
    uint16_t endpoint_id = light_endpoint_id;
    uint32_t cluster_id = OnOff::Id;
    uint32_t attribute_id = OnOff::Attributes::OnOff::Id;

    node_t *node = node::get();
    endpoint_t *endpoint = endpoint::get(node, endpoint_id);
    cluster_t *cluster = cluster::get(endpoint, cluster_id);
    attribute_t *attribute = attribute::get(cluster, attribute_id);

    esp_matter_attr_val_t val = esp_matter_invalid(NULL);
    attribute::get_val(attribute, &val);
    val.val.b = !val.val.b;
    attribute::update(endpoint_id, cluster_id, attribute_id, &val);
}

esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val)
{
    esp_err_t err = ESP_OK;
    if (endpoint_id == light_endpoint_id) {
        led_driver_handle_t handle = (led_driver_handle_t)driver_handle;
        if (cluster_id == OnOff::Id) {
            if (attribute_id == OnOff::Attributes::OnOff::Id) {
                err = app_driver_light_set_power(handle, val);
            }
        } else if (cluster_id == LevelControl::Id) {
            if (attribute_id == LevelControl::Attributes::CurrentLevel::Id) {
                err = app_driver_light_set_brightness(handle, val);
            }
        } else if (cluster_id == ColorControl::Id) {
            if (attribute_id == ColorControl::Attributes::CurrentHue::Id) {
                err = app_driver_light_set_hue(handle, val);
            } else if (attribute_id == ColorControl::Attributes::CurrentSaturation::Id) {
                err = app_driver_light_set_saturation(handle, val);
            } else if (attribute_id == ColorControl::Attributes::ColorTemperatureMireds::Id) {
                err = app_driver_light_set_temperature(handle, val);
            }
        }
    }
    return err;
}
/*esp_err_t app_driver_attribute_test_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val)
{
    esp_err_t err = ESP_OK;
    if (endpoint_id == light_sensor_endpoint_id) {
        bh1750_handle_t handle = (bh1750_handle_t)driver_handle;
        if (cluster_id == IlluminanceMeasurement::Id) {
            if (attribute_id == IlluminanceMeasurement::Attributes::MeasuredValue::Id) {
                bh1750_power_on(handle);
                bh1750_set_measure_time(handle, 69); // Adjust this measure time as needed
                bh1750_set_measure_mode(handle, BH1750_CONTINUE_1LX_RES);

                //esp_matter_attr_val_t val = esp_matter_invalid(NULL);
                val->val.u16 = (uint16_t)lux;
                printf("test update cb success\n");
                attribute::update(endpoint_id, cluster_id, attribute_id, &val);

    

            }
        }
    }


}*/



void sensor_display_task(void *pvParameters)
{
    //esp_err_t err = ESP_OK;
    // Initialize SSD1306 OLED
    
    ssd1306_handle_t display = ssd1306_create(I2C_PORT, SSD1306_ADDR);
    ssd1306_init(display);

    // Initialize BH1750 sensor
    bh1750_handle_t light_sensor_handle = bh1750_create(I2C_PORT, BH1750_ADDR);
    bh1750_power_on(light_sensor_handle);
    bh1750_set_measure_time(light_sensor_handle, 60);
    bh1750_set_measure_mode(light_sensor_handle, BH1750_CONTINUE_1LX_RES);
    

    //float lux = 0.0;

    while (1) {
        // Read light intensity from BH1750
        esp_err_t err = bh1750_get_data(light_sensor_handle, &lux);
        

        node_t *node = node::get();
        endpoint_t *endpoint = endpoint::get(node, light_sensor_endpoint_id);
        cluster_t *cluster = NULL;
        attribute_t *attribute = NULL;

        uint16_t endpoint_id = light_sensor_endpoint_id;
        uint32_t cluster_id = IlluminanceMeasurement::Id;
        uint32_t attribute_id = IlluminanceMeasurement::Attributes::MeasuredValue::Id;
        
        
        cluster = cluster::get(endpoint, IlluminanceMeasurement::Id);
        attribute = attribute::get(cluster, IlluminanceMeasurement::Attributes::MeasuredValue::Id);
        esp_matter_attr_val_t val = esp_matter_invalid(NULL);

        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Illuminance data read successful");
            printf("Light intensity: %.2f lx\n", lux);

            //esp_matter_attr_val_t val; // = esp_matter_invalid(NULL);
            printf("test point 1\n");
            attribute::get_val(attribute, &val);
            printf("test point 2\n");
            uint16_t m_val = (10000*log10(lux))+1;
            val.val.u16 = m_val; // consider converting using Remap to range
            printf("test point 3\n");
            attribute::update(endpoint_id, cluster_id, attribute_id, &val);
            printf("test point 4\n");

            // Display data on SSD1306 OLED
            ssd1306_clear_screen(display, 0x00); // Clear OLED screen
            char str[16];
            snprintf(str, sizeof(str), "Light: %.2f lx", lux);
            ssd1306_draw_string(display, 0, 0, (uint8_t *)str, 16, 1);
            ssd1306_refresh_gram(display); // Refresh OLED display
        } else {
            printf("Error reading BH1750 sensor: %d\n", err);
        }

        vTaskDelay(pdMS_TO_TICKS(2000)); // Wait for 2 seconds before reading again
        //ssd1306_delete(display);
    }

    // Cleanup
    ssd1306_delete(display);
    bh1750_delete(light_sensor_handle);
    vTaskDelete(NULL);
    //return err;
}


esp_err_t app_driver_light_set_defaults(uint16_t endpoint_id)
{
    esp_err_t err = ESP_OK;
    void *priv_data = endpoint::get_priv_data(endpoint_id);
    led_driver_handle_t handle = (led_driver_handle_t)priv_data;
    node_t *node = node::get();
    endpoint_t *endpoint = endpoint::get(node, endpoint_id);
    cluster_t *cluster = NULL;
    attribute_t *attribute = NULL;
    esp_matter_attr_val_t val = esp_matter_invalid(NULL);

    /* Setting brightness */
    cluster = cluster::get(endpoint, LevelControl::Id);
    attribute = attribute::get(cluster, LevelControl::Attributes::CurrentLevel::Id);
    attribute::get_val(attribute, &val);
    err |= app_driver_light_set_brightness(handle, &val);

    /* Setting color */
    cluster = cluster::get(endpoint, ColorControl::Id);
    attribute = attribute::get(cluster, ColorControl::Attributes::ColorMode::Id);
    attribute::get_val(attribute, &val);
    if (val.val.u8 == EMBER_ZCL_COLOR_MODE_CURRENT_HUE_AND_CURRENT_SATURATION) {
        /* Setting hue */
        attribute = attribute::get(cluster, ColorControl::Attributes::CurrentHue::Id);
        attribute::get_val(attribute, &val);
        err |= app_driver_light_set_hue(handle, &val);
        /* Setting saturation */
        attribute = attribute::get(cluster, ColorControl::Attributes::CurrentSaturation::Id);
        attribute::get_val(attribute, &val);
        err |= app_driver_light_set_saturation(handle, &val);
    } else if (val.val.u8 == EMBER_ZCL_COLOR_MODE_COLOR_TEMPERATURE) {
        /* Setting temperature */
        attribute = attribute::get(cluster, ColorControl::Attributes::ColorTemperatureMireds::Id);
        attribute::get_val(attribute, &val);
        err |= app_driver_light_set_temperature(handle, &val);
    } else {
        ESP_LOGE(TAG, "Color mode not supported");
    }

    /* Setting power */
    cluster = cluster::get(endpoint, OnOff::Id);
    attribute = attribute::get(cluster, OnOff::Attributes::OnOff::Id);
    attribute::get_val(attribute, &val);
    err |= app_driver_light_set_power(handle, &val);

    return err;

}

app_driver_handle_t app_driver_light_init()
{
    /* Initialize led */
    led_driver_config_t config = led_driver_get_config();
    led_driver_handle_t handle = led_driver_init(&config);
    return (app_driver_handle_t)handle;
}

/*
app_driver_handle_t app_driver_light_sensor_init(){
    light_sensor_driver_config_t config = light_sensor_driver_get_config();
    light_sensor_driver_handle_t handle = light_sensor_driver_init(&config);
    return (app_driver_handle_t)handle;

}
*/

app_driver_handle_t app_driver_button_init()
{
    /* Initialize button */
    button_config_t config = button_driver_get_config();
    button_handle_t handle = iot_button_create(&config);
    iot_button_register_cb(handle, BUTTON_PRESS_DOWN, app_driver_button_toggle_cb, NULL);
    return (app_driver_handle_t)handle;
}
