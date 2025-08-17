/**********************************************************************
  OV2640.cpp

  Robots-For-All (R4A)
  OV2640 camera support
**********************************************************************/

#include "R4A_ESP32.h"

//****************************************
// Constants
//****************************************

const R4A_FRAME_SIZE_MASK_t r4aOv2640SupportedFrameSizes = 0
//    | SUPPORTED(FRAMESIZE_96X96)    //   96 x   96
    | SUPPORTED(FRAMESIZE_QQVGA)    //  160 x  120
    | SUPPORTED(FRAMESIZE_128X128)  //  128 x  128
    | SUPPORTED(FRAMESIZE_QCIF)     //  176 x  144
    | SUPPORTED(FRAMESIZE_HQVGA)    //  240 x  176
    | SUPPORTED(FRAMESIZE_240X240)  //  240 x  240
    | SUPPORTED(FRAMESIZE_QVGA)     //  320 x  240
//    | SUPPORTED(FRAMESIZE_320X320)  //  320 x  320
    | SUPPORTED(FRAMESIZE_CIF)      //  400 x  296
    | SUPPORTED(FRAMESIZE_HVGA)     //  480 x  320
    | SUPPORTED(FRAMESIZE_VGA)      //  640 x  480
    | SUPPORTED(FRAMESIZE_SVGA)     //  800 x  600
    | SUPPORTED(FRAMESIZE_XGA)      // 1024 x  768
    | SUPPORTED(FRAMESIZE_HD)       // 1200 x  720
    | SUPPORTED(FRAMESIZE_SXGA)     // 1280 x 1024
    | SUPPORTED(FRAMESIZE_UXGA)     // 1600 x 1200
;

const R4A_PIXEL_FORMAT_MASK_t r4aOv2640SupportedPixelFormats = 0
    | SUPPORTED(R4A_PIXEL_FORMAT_GRAYSCALE)
    | SUPPORTED(R4A_PIXEL_FORMAT_JPEG)
    | SUPPORTED(R4A_PIXEL_FORMAT_RAW)
    | SUPPORTED(R4A_PIXEL_FORMAT_RGB444)
    | SUPPORTED(R4A_PIXEL_FORMAT_RGB555)
    | SUPPORTED(R4A_PIXEL_FORMAT_RGB565)
    | SUPPORTED(R4A_PIXEL_FORMAT_RGB888)
    | SUPPORTED(R4A_PIXEL_FORMAT_YUV420)
    | SUPPORTED(R4A_PIXEL_FORMAT_YUV422)
;

//****************************************
// Locals
//****************************************

uint8_t r4aOV2640PixelFormat = PIXFORMAT_RGB565;

bool r4aOV2640JpegDisplayTime;  // Set to true to display the JPEG conversion time

//*********************************************************************
// Display a group of registers
void r4aOv2640DisplayRegisters(uint8_t firstRegister,
                               size_t bytesToRead,
                               Print * display)
{
    uint8_t data[256];
    uint32_t offset;
    uint8_t regAddress;
    int value;

    do
    {
        // Validate the bytesToRead value
        if (bytesToRead > sizeof(data))
        {
            display->printf("ERROR: bytesToRead > %d!\r\n", sizeof(bytesToRead));
            break;
        }

        // Read the register values
        regAddress = firstRegister;
        for (offset = 0; offset < bytesToRead; offset++)
        {
            value = r4aCameraGetRegister(regAddress, nullptr);
            if (value < 0)
            {
                if (display)
                    display->printf("ERROR: Failed to read register 0x%02x\r\n", regAddress);
                break;
            }
            data[offset] = (uint8_t)value;
        }
        if (value < 0)
            break;

        // Display the bytes
        r4aDumpBuffer(firstRegister, data, bytesToRead, display);
    } while (0);
}

//*********************************************************************
// Dump all of the OV2640 registers in hexadecimal
void r4aOv2640DumpRegisters(Print * display)
{
    size_t bytesToRead;
    uint32_t offset;

    do
    {
        // Display the header
        display->println("OV2640 Registers");
        display->println("----------------");
        display->println();
        display->println("             0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f  0123456789abcdef");
        display->println("            -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --  ----------------");

        // Display register 5
        offset = 5;
        bytesToRead = 1;
        r4aOv2640DisplayRegisters(offset, bytesToRead, display);

        // Display register 0x44
        offset = 0x44;
        bytesToRead = 1;
        r4aOv2640DisplayRegisters(offset, bytesToRead, display);

        // Display register 0x50 - 0x57
        offset = 0x50;
        bytesToRead = 8;
        r4aOv2640DisplayRegisters(offset, bytesToRead, display);

        // Display register 0x5a - 0x57c
        offset = 0x5a;
        bytesToRead = 3;
        r4aOv2640DisplayRegisters(offset, bytesToRead, display);

        // Display register 0x7c - 0x7d
        offset = 0x7c;
        bytesToRead = 2;
        r4aOv2640DisplayRegisters(offset, bytesToRead, display);

        // Display register 0x86 - 0x87
        offset = 0x86;
        bytesToRead = 2;
        r4aOv2640DisplayRegisters(offset, bytesToRead, display);

        // Display register 0x8c
        offset = 0x8c;
        bytesToRead = 1;
        r4aOv2640DisplayRegisters(offset, bytesToRead, display);

        // Display register 0xc0 - 0xc3
        offset = 0xc0;
        bytesToRead = 4;
        r4aOv2640DisplayRegisters(offset, bytesToRead, display);

        // Display register 0xd3
        offset = 0xd3;
        bytesToRead = 1;
        r4aOv2640DisplayRegisters(offset, bytesToRead, display);

        // Display register 0xda
        offset = 0xda;
        bytesToRead = 1;
        r4aOv2640DisplayRegisters(offset, bytesToRead, display);

        // Display register 0xe0
        offset = 0xe0;
        bytesToRead = 1;
        r4aOv2640DisplayRegisters(offset, bytesToRead, display);

        // Display register 0xf0
        offset = 0xf0;
        bytesToRead = 1;
        r4aOv2640DisplayRegisters(offset, bytesToRead, display);

        // Display register 0xf7
        offset = 0xf7;
        bytesToRead = 1;
        r4aOv2640DisplayRegisters(offset, bytesToRead, display);

        // Display the offset header
        display->println("            -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --  ----------------");
        display->println("             0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f  0123456789abcdef");
    } while (0);
}

//*********************************************************************
// JPEG image web page handler
esp_err_t r4aOv2640JpegHandler(httpd_req_t *request)
{
    int64_t endTime;
    camera_fb_t * frameBuffer;
    int64_t startTime;
    esp_err_t status;
    R4A_OV2640_SETUP * ov2640Parameters;

    // Get the OV2640 data structure address
    ov2640Parameters = (R4A_OV2640_SETUP *)request->user_ctx;

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

        // Build the response header
        httpd_resp_set_type(request, "image/jpeg");
        httpd_resp_set_hdr(request, "Content-Disposition", "inline; filename=capture.jpg");
        httpd_resp_set_hdr(request, "Access-Control-Allow-Origin", "*");

        // Add the timestamp to the header
        char timestamp[32];
        snprintf(timestamp, sizeof(timestamp), "%lld.%06ld",
                 frameBuffer->timestamp.tv_sec, frameBuffer->timestamp.tv_usec);
        httpd_resp_set_hdr(request, "X-Timestamp", (const char *)timestamp);

        // Process the frame buffer
        if (ov2640Parameters && (ov2640Parameters->_processWebServerFrameBuffer))
            ov2640Parameters->_processWebServerFrameBuffer(frameBuffer, &Serial);

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
                                  r4aOv2640SendJpegChunk,
                                  &jchunk) ? ESP_OK : ESP_FAIL;
            if (status != ESP_OK)
                break;
            status = httpd_resp_send_chunk(request, NULL, 0);
            if (status != ESP_OK)
                break;
        }
        endTime = esp_timer_get_time();
        if (r4aOV2640JpegDisplayTime)
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
// Encode the JPEG image
size_t r4aOv2640SendJpegChunk(void * arg,
                              size_t index,
                              const void* data,
                              size_t len)
{
    R4A_JPEG_CHUNKING_T * chunk = (R4A_JPEG_CHUNKING_T *)arg;
    if (!index)
        chunk->length = 0;
    if (httpd_resp_send_chunk(chunk->req, (const char *)data, len) != ESP_OK)
        return 0;
    chunk->length += len;
    return len;
}

//*********************************************************************
// Initialize the camera
bool r4aOv2640Setup(const R4A_OV2640_SETUP * ov2640Parameters,
                    Print * display)
{
    bool cameraInitialized;
    camera_config_t config;
    sensor_t * ov2640Camera;
    const R4A_CAMERA_PINS * pins;
    esp_err_t status;

    do
    {
        cameraInitialized = false;

        // Get the pins data structure
        pins = ov2640Parameters->_pins;
        if (pins == nullptr)
        {
            if (display)
                display->printf("ERROR: Camera pins need to be specified!\r\n");
            break;
        }

        // Connect the control pins
        config.pin_reset = pins->_pinReset;
        config.pin_pwdn = pins->_pinPowerDown;

        // Route a clock signal from the ESP32 to the camera
        config.xclk_freq_hz = ov2640Parameters->_clockHz;
        config.ledc_timer = ov2640Parameters->_ledcTimer;
        config.ledc_channel = ov2640Parameters->_ledcChannel;
        config.pin_xclk = pins->_pinXCLK;

        // Connect and I2C controller to the camera's SCCB pins
        config.pin_sccb_sda = pins->_pinSccbData;
        config.pin_sccb_scl = pins->_pinSccbClk;

        // Frame synchronization
        config.pin_vsync = pins->_pinVSYNC;
        config.pin_href = pins->_pinHREF;
        config.pin_pclk = pins->_pinPCLK;

        // Route the camera data to the ESP32 I2S controller
        config.pin_d0 = pins->_pinY2;
        config.pin_d1 = pins->_pinY3;
        config.pin_d2 = pins->_pinY4;
        config.pin_d3 = pins->_pinY5;
        config.pin_d4 = pins->_pinY6;
        config.pin_d5 = pins->_pinY7;
        config.pin_d6 = pins->_pinY8;
        config.pin_d7 = pins->_pinY9;

        // Select the pixel format sent over the I2S bus (camera --> ESP32)
        config.pixel_format = ov2640Parameters->_pixelFormat;
        config.jpeg_quality = ov2640Parameters->_jpegQuality;

        // Select the frame size
        config.frame_size = ov2640Parameters->_frameSize;

        // Select the number of frame buffers
        config.fb_count = ov2640Parameters->_frameBufferCount;

        // When to take the picture
        config.grab_mode = CAMERA_GRAB_LATEST;
        config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

        // Initialize the camera
        status = esp_camera_init(&config);
        if (status != ESP_OK)
        {
            // Display the camera initialization error
            if (display)
                display->printf("ERROR: Camera setup failed, returned error 0x%x\r\n", status);
            break;
        }

        // Get the camera structure
        ov2640Camera = esp_camera_sensor_get();
        if (!ov2640Camera)
        {
            // Display camera structure allocation failure
            display->println("ERROR: Camera structure allocation failure");
            break;
        }

        // Adjust the image to match what is seen
        ov2640Camera->set_hmirror(ov2640Camera, 1);
        ov2640Camera->set_vflip(ov2640Camera, 1);

        // Adjust the brightness and color saturation
        ov2640Camera->set_brightness(ov2640Camera, 1);
        ov2640Camera->set_saturation(ov2640Camera, -2);
        ov2640Camera->set_agc_gain(ov2640Camera, 30);
        ov2640Camera->set_awb_gain(ov2640Camera, 1);
        ov2640Camera->set_gain_ctrl(ov2640Camera, 1);

        // Successful initialization
        if (display)
            display->println("Camera configuration complete!");
        cameraInitialized = true;
    } while (0);
    return cameraInitialized;
}

//*********************************************************************
// Build the OV2640 details web page
esp_err_t r4aOv2640WebPage(httpd_req_t *request)
{
    sensor_t * sensor;
    String webPage("");

    // Build the web page
    webPage += "<!DOCTYPE html>";
    webPage += "<html lang=\"en\">";
    webPage += "<head>";
    webPage += "<meta charset=\"utf-8\">";
    webPage += "<title>OV2640 Details</title>";
    webPage += "</head>";
    webPage += "<body>";
    webPage += "<h1>OV2640 Details</h1>";
    webPage += "<p>Supported frame sizes:";
    if (r4aOv2640SupportedFrameSizes == 0)
        webPage += " None</p>";
    else
    {
        webPage += "</p>";
        webPage += "<ul>";
        for (int index = 0; index < (sizeof(r4aOv2640SupportedFrameSizes) << 2); index++)
            if (r4aOv2640SupportedFrameSizes & (1 << index))
            {
                const R4A_CAMERA_FRAME * frameDetails;
                char sizeName[32];

                // Get the frame format details
                frameDetails = r4aCameraFindFrameDetails((framesize_t)index);
                if (frameDetails)
                {
                    // Build the size name width x height
                    sprintf(sizeName, "%dx%d", frameDetails->_xPixels, frameDetails->_yPixels);

                    // Add the FRAMESIZE_t value
                    webPage += "<li>";
                    webPage += String(index);
                    webPage += ": ";

                    // Add the name
                    webPage += frameDetails->_name;
                    if (strcmp(sizeName, frameDetails->_name) != 0)
                    {
                        // Add the width and height
                        webPage += " (";
                        webPage += String(frameDetails->_xPixels);
                        webPage += " x ";
                        webPage += String(frameDetails->_yPixels);
                        webPage += " )";
                    }
                    webPage += "</li>";
                }
            }
        webPage += "</ul>";
    }

    webPage += "<p>Supported pixel formats:";
    if (r4aOv2640SupportedPixelFormats == 0)
        webPage += " None</p>";
    else
    {
        webPage += "</p>";
        webPage += "<ul>";
        for (int index = 0; index < (sizeof(r4aOv2640SupportedPixelFormats) << 2); index++)
            if (r4aOv2640SupportedPixelFormats & (1 << index))
            {
                const R4A_CAMERA_PIXEL * pixelDetails;

                // Get the frame format details
                pixelDetails = r4aCameraFindPixelDetails((pixformat_t)index);
                if (pixelDetails)
                {
                    // Add the PIXFORMAT_t value
                    webPage += "<li>";
                    webPage += String(index);
                    webPage += ": ";

                    // Add the name
                    webPage += pixelDetails->_name;
                    webPage += ", bits per pixel: ";
                    webPage += String(pixelDetails->_bitsPerPixel);
                    webPage += "</li>";
                }
            }
        webPage += "</ul>";
    }

    // Display the current image
    sensor = r4aCameraGetSensor();
    if (sensor)
    {
        framesize_t frameSize;
        int khz;
        int mhz;
        char mhzString[16];
        R4A_FRAME_SIZE_t r4aFrameFormat;
        int xPixels;
        int xRatio;
        int yPixels;
        int yRatio;

        frameSize = sensor->status.framesize;
        r4aFrameFormat = r4aCameraFrameFormat[frameSize]._r4aFrameFormat;

        xRatio = r4aCameraFrameFormats[r4aFrameFormat]._xRatio;
        yRatio = r4aCameraFrameFormats[r4aFrameFormat]._yRatio;

        xPixels = r4aCameraFrameFormats[r4aFrameFormat]._xPixels;
        yPixels = r4aCameraFrameFormats[r4aFrameFormat]._yPixels;

        khz = sensor->xclk_freq_hz / 1000;
        mhz = khz / 1000;
        khz -= mhz * 1000;
        sprintf(mhzString, "%d.%03d MHz", mhz, khz);

        webPage += "<h2>Image</h2>";
        webPage += "<ul>";
        webPage += "<li>XCLK: ";
        webPage += String(mhzString);
        webPage += "</li>";
        webPage += "<li>Frame Size: " + String(frameSize) + "</li>";
        webPage += "<li>Pixel Format: " + String(sensor->pixformat) + "</li>";
        webPage += "<li>Quality: " + String(sensor->status.quality) + "</li>";
        webPage += "<li>Aspect Ratio: "
                + String(xRatio) + " x "
                + String(yRatio) + ", ("
                + String(xPixels) + " x "
                + String(yPixels) + " pixels)</li>";
        webPage += "</ul>";
        webPage += "<img src=\"../jpeg\">";
    }

    webPage += "</body>";
    webPage += "</html>";

    // Respond to the request with the web page
    httpd_resp_send(request, webPage.c_str(), HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

//*********************************************************************
// Update the camera processing state
void r4aOv2640Update(R4A_OV2640_SETUP * ov2640Parameters,
                     Print * display)
{
    camera_fb_t * frameBuffer;

    if (ov2640Parameters->_processFrameBuffer)
    {
        // Get a frame buffer
        frameBuffer = esp_camera_fb_get();
        if (!frameBuffer)
            return;

        // Process the frame buffer
        ov2640Parameters->_processFrameBuffer(frameBuffer, display);

        // Return the frame buffer
        esp_camera_fb_return(frameBuffer);
    }
}
