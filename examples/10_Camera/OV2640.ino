/**********************************************************************
  OV2640.ino

  OV2640 camera support routines
**********************************************************************/

//****************************************
// Constants
//****************************************

// URI handler structure for GET /jpeg
const httpd_uri_t ov2640JpegPage =
{
    .uri      = "/jpeg",
    .method   = HTTP_GET,
    .handler  = ov2640JpegHandler,
    .is_websocket = true,
    .handle_ws_control_frames = false,
    .supported_subprotocol = nullptr,
};

//****************************************
// Locals
//****************************************

bool ov2640JpegDisplayTime;  // Set to true to display the JPEG conversion time

//*********************************************************************
// Process the web server's frame buffer
// Inputs:
//   frameBuffer: Buffer containing the raw image data
// Outputs:
//   Returns true if the processing was successful and false upon error
bool ov2640ProcessWebServerFrameBuffer(camera_fb_t * frameBuffer)
{
    return true;
}

//*********************************************************************
// Encode the JPEG image
size_t ov2640SendJpegChunk(void * arg,
                           size_t index,
                           const void* data,
                           size_t len)
{
    R4A_JPEG_CHUNKING_T * chunk = (R4A_JPEG_CHUNKING_T *)arg;
    if(!index){
        chunk->length = 0;
    }
    if(httpd_resp_send_chunk(chunk->req, (const char *)data, len) != ESP_OK){
        return 0;
    }
    chunk->length += len;
    return len;
}

//*********************************************************************
// JPEG image web page handler
esp_err_t ov2640JpegHandler(httpd_req_t *request)
{
    int64_t endTime;
    camera_fb_t * frameBuffer;
    int64_t startTime;
    esp_err_t status;

    do
    {
        startTime = esp_timer_get_time();
        frameBuffer = nullptr;
        status = ESP_FAIL;

        // Allocate the frame buffer
        frameBuffer = esp_camera_fb_get();
        if (!frameBuffer)
        {
            Serial.println("ERROR: Failed to capture the image");
            httpd_resp_send_500(request);
            break;
        }

        // Process the frame buffer
        ov2640ProcessWebServerFrameBuffer(frameBuffer);

        // Build the response header
        httpd_resp_set_type(request, "image/jpeg");
        httpd_resp_set_hdr(request, "Content-Disposition", "inline; filename=capture.jpg");
        httpd_resp_set_hdr(request, "Access-Control-Allow-Origin", "*");

        // Add the timestamp to the header
        char timestamp[32];
        snprintf(timestamp, sizeof(timestamp), "%lld.%06ld",
                 frameBuffer->timestamp.tv_sec, frameBuffer->timestamp.tv_usec);
        httpd_resp_set_hdr(request, "X-Timestamp", (const char *)timestamp);

        // Send the captured image
        if (frameBuffer->format == PIXFORMAT_JPEG)
        {
            status = httpd_resp_send(request, (const char *)frameBuffer->buf, frameBuffer->len);
            if (status != ESP_OK)
                break;
        }
        else
        {
            // Break the image into multiple chunks
            R4A_JPEG_CHUNKING_T jchunk = {request, 0};
            status = frame2jpg_cb(frameBuffer,
                                  80,
                                  ov2640SendJpegChunk,
                                  &jchunk) ? ESP_OK : ESP_FAIL;
            if (status != ESP_OK)
                break;
            status = httpd_resp_send_chunk(request, NULL, 0);
            if (status != ESP_OK)
                break;
        }
        endTime = esp_timer_get_time();
        if (ov2640JpegDisplayTime)
            Serial.printf("JPG: %lu bytes %lu mSec", (uint32_t)(frameBuffer->len),
                          (uint32_t)((endTime - startTime) / 1000));
        status = ESP_OK;
    } while (0);

    // Return the frame buffer
    if (frameBuffer)
        esp_camera_fb_return(frameBuffer);
    return status;
}

//*********************************************************************
// Initialize the camera
void ov2640Setup()
{
    camera_config_t config;

    // Specify the camera control pins
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;

    // Route a 20 MHz clock signal from the ESP32 to the camera
    config.xclk_freq_hz = 20 * 1000 * 1000;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_xclk = XCLK_GPIO_NUM;

    // Connect the camera to an initialized I2C bus
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.sccb_i2c_port = R4A_4WD_CAR_SCCB_BUS_NUMBER;

    // Route the camera data to the ESP32 I2S controller
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;

    // Select the pixel format sent over the I2S bus (camera --> ESP32)
    config.pixel_format = PIXFORMAT_JPEG;
    config.jpeg_quality = 10;

    // Select the frame size
    //  config.frame_size = FRAMESIZE_VGA;     //640*480
    config.frame_size = FRAMESIZE_CIF;     //400*296
    //  config.frame_size = FRAMESIZE_QVGA;    //320*240
    //  config.frame_size = FRAMESIZE_HQVGA;   //240*176
    //  config.frame_size = FRAMESIZE_QCIF;    //176*144
    //  config.frame_size = FRAMESIZE_QQVGA2;  //128*160
    //  config.frame_size = FRAMESIZE_QQVGA;   //160*120

    // Select the number of frame buffers
    config.fb_count = 1;

    // Initialize the camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }
    Serial.println("Camera configuration complete!");
}
