/**********************************************************************
  OV2640.cpp

  Robots-For-All (R4A)
  OV2640 camera support
**********************************************************************/

#include "R4A_ESP32.h"

//****************************************
// Types
//****************************************

typedef struct _R4A_WEB_PAGE_ENABLES
{
    int _valueId;
    const char * _tagText;
    const char * _titleText;
} R4A_WEB_PAGE_ENABLES;

typedef struct _R4A_WEB_PAGE_VALUES
{
    int _valueId;
    const char * _tagText;
    const char * _titleText;
    int _minValue;
    int _maxValue;
} R4A_WEB_PAGE_VALUES;

//****************************************
// Constants
//****************************************

enum R4A_ENABLE_ID
{
    EID_AECE = 0,
    EID_AWB,
    EID_BPC,
    EID_COLORBAR,
    EID_DCW,
    EID_ECE,
    EID_GCE,
    EID_GMA,
    EID_HMIRROR,
    EID_LENS,
    EID_VFLIP,
    EID_WPC,
    // Add new values here
    EID_MAX_EID
};

enum R4A_VALUE_ID
{
    VID_AE_LEVEL = 0,
    VID_AEC_VALUE,
    VID_AGC_GAIN,
    VID_BRIGHTNESS,
    VID_CONTRAST,
    VID_DENOISE,
    VID_GAIN_CEILING,
    VID_QUALITY,
    VID_SATURATION,
    VID_SHARPNESS,
    VID_EFFECTS,
    VID_WB_MODE,
    // Add new values here
    VID_MAX_VID
};

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
    R4A_OV2640_PROCESS_WEB_SERVER_FRAME_BUFFER processFrame;
    int64_t startTime;
    esp_err_t status;

    // Get the OV2640 data structure address
    processFrame = (R4A_OV2640_PROCESS_WEB_SERVER_FRAME_BUFFER)request->user_ctx;

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
        if (processFrame)
            processFrame(frameBuffer, &Serial);

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
// Add the supported pixel formats
void r4aOv2640WebPageAddPixelFormats(String &webPage)
{
    // Display the supported pixel formats
    webPage += "<H1>Supported Pixel Formats</h1>";
    if (r4aOv2640SupportedPixelFormats == 0)
        webPage += "<p>None</p>";
    else
    {
        webPage += "<ul>";

        // Walk the list of supported pixel formats
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
}

//*********************************************************************
// Add the supported frame sizes
void r4aOv2640WebPageAddFrameSizes(String &webPage)
{
    // Display the supported frame sizes
    webPage += "<h1>Supported Frame Sizes</h1>";
    if (r4aOv2640SupportedFrameSizes == 0)
        webPage += "<p>None</p>";
    else
    {
        webPage += "<ul>";

        // Walk the list of supported frame sizes
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
}

//*********************************************************************
// Add the images sizes
void r4aOv2640WebPageAddImageAttributes(String &webPage)
{
    const R4A_CAMERA_FRAME * cameraFrame;
    const R4A_CAMERA_PIXEL * cameraPixelFormat;
    framesize_t frameSize;
    int khz;
    int mhz;
    char mhzString[16];
    pixformat_t pixelFormat;
    int xPixels;
    int xRatio;
    int yPixels;
    int yRatio;

    // Translate the frame size
    frameSize = (framesize_t)r4aCameraGetFrameSize();
    cameraFrame = r4aCameraFindFrameDetails(frameSize);

    xRatio = cameraFrame->_xRatio;
    yRatio = cameraFrame->_yRatio;

    xPixels = cameraFrame->_xPixels;
    yPixels = cameraFrame->_yPixels;

    // Translate the pixel format
    pixelFormat = (pixformat_t)r4aCameraGetPixelFormat();
    cameraPixelFormat = r4aCameraFindPixelDetails(pixelFormat);

    // Format the clock frequency
    khz = r4aCameraGetClockHz() / 1000;
    mhz = khz / 1000;
    khz -= mhz * 1000;
    sprintf(mhzString, "%d.%03d MHz", mhz, khz);

    // Display the image attributes
    webPage += "<ul>";
    webPage += "<li>Frame Size: "
            + String((int)frameSize)
            + ", "
            + cameraFrame->_name
            + ", "
            + String(xRatio) + " x "
            + String(yRatio) + ", ("
            + String(xPixels) + " x "
            + String(yPixels) + " pixels)</li>";
    webPage += "<li>Pixel Format: "
            + String((int)r4aCameraGetPixelFormat())
            + ", "
            + cameraPixelFormat->_name
            + "</li>";
    webPage += "<li>Quality: " + String(r4aCameraGetQuality()) + "</li>";
    webPage += "<li>Automatic Exposure Correction: " + String(r4aCameraGetAutomaticExposureCorrection()) + "</li>";
    webPage += "<li>Automatic Exposure Enable: " + String(r4aCameraGetAutomaticExposureEnable()) + "</li>";
    webPage += "<li>Automatic Exposure Level: " + String(r4aCameraGetAutomaticExposureLevel()) + "</li>";
    webPage += "<li>Automatic Gain Control: " + String(r4aCameraGetAutomaticGainControl()) + "</li>";
    webPage += "<li>Automatic White Balance Enable: " + String(r4aCameraGetAutomaticWhiteBalanceEnable()) + "</li>";
    webPage += "<li>BPC: " + String(r4aCameraGetBpc()) + "</li>";
    webPage += "<li>Brightness: " + String(r4aCameraGetBrightness()) + "</li>";
    webPage += "<li>Color Bar Enable: " + String(r4aCameraGetColorBarEnable()) + "</li>";
    webPage += "<li>Contrast: " + String(r4aCameraGetContrast()) + "</li>";
    webPage += "<li>DCW: " + String(r4aCameraGetDcw()) + "</li>";
//    webPage += "<li>Denoise: " + String(r4aCameraGetDenoise()) + "</li>";
    webPage += "<li>Exposure Control Enable: " + String(r4aCameraGetExposureControlEnable()) + "</li>";
    webPage += "<li>Gain Ceiling: " + String(r4aCameraGetGainCeiling()) + "</li>";
    webPage += "<li>Gain Control Enable: " + String(r4aCameraGetGainControlEnable()) + "</li>";
    webPage += "<li>GMA Raw: " + String(r4aCameraGetRawGmaEnable()) + "</li>";
    webPage += "<li>Horizontal Mirror: " + String(r4aCameraGetHorizontalMirror()) + "</li>";
    webPage += "<li>Lens Control Enable: " + String(r4aCameraGetLensControlEnable()) + "</li>";
    webPage += "<li>Saturation: " + String(r4aCameraGetSaturation()) + "</li>";
    webPage += "<li>Special Effect: " + String(r4aCameraGetSpecialEffect()) + "</li>";
//    webPage += "<li>Sharpness: " + String(r4aCameraGetSharpness()) + "</li>";
    webPage += "<li>Vertical Flip: " + String(r4aCameraGetVerticalFlip()) + "</li>";
    webPage += "<li>White Balance Mode: " + String(r4aCameraGetWhiteBalanceMode()) + "</li>";
    webPage += "<li>WPC: " + String(r4aCameraGetWpc()) + "</li>";
    webPage += "<li>XCLK: " + String(mhzString) + "</li>";
    webPage += "</ul>";
}

//*********************************************************************
// Add an enable line
void r4aOv2640WebPageAddEnable(String &webPage,
                               const char * currentWebPage,
                               const char * titleString,
                               const char * tagString,
                               const char * buttonString,
                               int currentValue)
{
    webPage += "        <tr>\n";
    webPage += "          <form action=\"";
    webPage +=  currentWebPage;
    webPage += "\" method=\"get\">\n";
    webPage += "            <td>";
    webPage += titleString;
    webPage += "</td>\n";

    webPage += "            <td>\n";
    webPage += "              <select name=\"";
    webPage += tagString;
    webPage += "\">\n";
    webPage += "                <option";
    webPage += currentValue ? " selected" : "";
    webPage += " value=\"0\">0</option>\n";
    webPage += "                <option";
    webPage += currentValue ? "" : " selected";
    webPage += " value=\"1\">1</option>\n";
    webPage += "              </select>\n";
    webPage += "            </td>\n";

    webPage += "            <td><input type=\"submit\" value=\"";
    webPage += buttonString;
    webPage += "\"></td>\n";
    webPage += "          </form>\n";
    webPage += "        </tr>\n";
}

//*********************************************************************
// Add a value line
void r4aOv2640WebPageAddValue(String &webPage,
                              const char * currentWebPage,
                              const char * titleString,
                              const char * tagString,
                              const char * buttonString,
                              int minValue,
                              int maxValue,
                              int currentValue)
{
    webPage += "        <tr>\n";
    webPage += "          <form action=\"";
    webPage +=  currentWebPage;
    webPage += "\" method=\"get\">\n";
    webPage += "            <td>";
    webPage += titleString;
    webPage += "</td>\n";

    webPage += "            <td>\n";
    webPage += "              <select name=\"";
    webPage += tagString;
    webPage += "\">\n";
    for (int value = minValue; value <= maxValue; value++)
    {
        webPage += "                <option";
        if (value == currentValue)
            webPage += " selected";
        webPage += " value=\"";
        webPage += String(value);
        webPage += "\">";
        webPage += String(value);
        webPage += "</option>\n";
    }
    webPage += "              </select>\n";
    webPage += "            </td>\n";

    webPage += "            <td><input type=\"submit\" value=\"";
    webPage += buttonString;
    webPage += "\"></td>\n";
    webPage += "          </form>\n";
    webPage += "        </tr>\n";
}

//*********************************************************************
// Add a button
void r4aOv2640WebPageAddButton(String &webPage,
                               const char * currentWebPage,
                               const char * buttonText)
{
    webPage += "      <form action=\"";
    webPage +=  currentWebPage;
    webPage += "\" method=\"get\">\n";
    webPage += "        <input type=\"submit\" value=\"";
    webPage += buttonText;
    webPage += "\">\n";
    webPage += "      </form>\n";
    webPage += "      <br>\r\n\n";
}

//*********************************************************************
// Build the OV2640 details web page
esp_err_t r4aOv2640WebPage(httpd_req_t *request)
{
    const R4A_WEB_PAGE_ENABLES enables[] =
    { //   ID              tag          title
        {EID_AECE,      "AEC",    "Automatic Exposure Control"},
        {EID_AWB,       "AWB",      "Automatic White Balance"},
        {EID_BPC,       "BPC",      "BPC"},
        {EID_COLORBAR,  "CBE",      "Color Bar"},
        {EID_DCW,       "DCW",      "DCW"},
        {EID_ECE,       "ECE",      "Exposure Control"},
        {EID_GCE,       "GCE",      "Gain Control"},
        {EID_GMA,       "GMA",      "GMA Raw"},
        {EID_HMIRROR,   "HMIRROR",  "Horizontal Mirror"},
        {EID_LENS,      "LENS",     "Lens Control"},
        {EID_VFLIP,     "VFLIP",    "Vertical Flip"},
        {EID_WPC,       "WPC",      "WPC"},
    };
    const int enableCount = sizeof(enables) / sizeof(enables[0]);
    const R4A_WEB_PAGE_VALUES valueList[] =
    { //    ID                 tag             title            min  max
        {VID_AE_LEVEL,      "AE_LEVEL",     "Auto Exposure",    -2,  2},
//        {VID_AEC_VALUE,     "AEC_VALUE",    "AEC Value",         0,  1200},
        {VID_AGC_GAIN,      "AGC_GAIN",     "AGC Gain",          0, 30},
        {VID_BRIGHTNESS,    "BRIGHTNESS",   "Brightness",       -2,  2},
        {VID_CONTRAST,      "CONTRAST",     "Contrast",         -2,  2},
//        {VID_DENOISE,       "DENOISE",      "Denoise",           0,  6},
        {VID_GAIN_CEILING,  "GAIN_CEILING", "Gain Ceiling",      0,  6},
        {VID_QUALITY,       "QUALITY",      "Quality",           3, 63}, // 0 - 2 cause crashes
        {VID_SATURATION,    "SATURATION",   "Saturation",       -2,  2},
//        {VID_SHARPNESS,     "SHARPNESS",    "Sharpness",        -2,  2},
        {VID_EFFECTS,       "EFFECTS",      "Special Effects",   0,  6},
        {VID_WB_MODE,       "WB_MODE",      "White Balance Mode",0,  4},
    };
    const int valueListCount = sizeof(valueList) / sizeof(valueList[0]);
    const char * currentWebPage = "OV2640";
    int index;
    sensor_t * sensor;
    String webPage("");

    // Build the web page
    webPage += "<!DOCTYPE html>\n";
    webPage += "<html lang=\"en\">\n";
    webPage += "  <head>\n";
    webPage += "    <meta charset=\"utf-8\">\n";
    webPage += "    <title>OV2640 Details</title>\n";
    webPage += "  </head>\n";
    webPage += "  <body>\n";
    webPage += "<h1>OV2640 Image</h1>\n";

    // Time stamp the image
    if (r4aNtpIsTimeValid())
    {
        uint32_t seconds = r4aNtpGetEpochTime();
        webPage += "<p>" + r4aNtpGetDate(seconds)
                + " " + r4aNtpGetTime24(seconds)
                + "</p>\n";
    }

    // Display the current image
    sensor = r4aCameraGetSensor();
    if (sensor)
    {
        const char * endOfValue;
        char * fieldName;
        size_t length;
        size_t nameLength;
        const char * options;
        char * valueText;
        size_t valueLength;
        int value;

        // Get the values
        int enableValues[EID_MAX_EID];
        enableValues[EID_AECE] = r4aCameraGetAutomaticExposureEnable();
        enableValues[EID_AWB] = r4aCameraGetAutomaticWhiteBalanceEnable();
        enableValues[EID_BPC] = r4aCameraGetBpc();
        enableValues[EID_COLORBAR] = r4aCameraGetColorBarEnable();
        enableValues[EID_DCW] = r4aCameraGetDcw();
        enableValues[EID_ECE] = r4aCameraGetExposureControlEnable();
        enableValues[EID_GCE] = r4aCameraGetGainControlEnable();
        enableValues[EID_GMA] = r4aCameraGetRawGmaEnable();
        enableValues[EID_HMIRROR] = r4aCameraGetHorizontalMirror();
        enableValues[EID_LENS] = r4aCameraGetLensControlEnable();
        enableValues[EID_VFLIP] = r4aCameraGetVerticalFlip();
        enableValues[EID_WPC] = r4aCameraGetWpc();

        int values[VID_MAX_VID];
        values[VID_AE_LEVEL] = r4aCameraGetAutomaticExposureLevel();
        values[VID_AEC_VALUE] = r4aCameraGetAutomaticExposureCorrection();
        values[VID_AGC_GAIN] = r4aCameraGetAutomaticGainControl();
        values[VID_BRIGHTNESS] = r4aCameraGetBrightness();
        values[VID_CONTRAST] = r4aCameraGetContrast();
        values[VID_DENOISE] = r4aCameraGetDenoise();
        values[VID_GAIN_CEILING] = r4aCameraGetGainCeiling();
        values[VID_QUALITY] = r4aCameraGetQuality();
        values[VID_SATURATION] = r4aCameraGetSaturation();
        values[VID_SHARPNESS] = r4aCameraGetSharpness();
        values[VID_EFFECTS] = r4aCameraGetSpecialEffect();
        values[VID_WB_MODE] = r4aCameraGetWhiteBalanceMode();

        // Parse the URI to locate the field
        options = request->uri;
        while (*options && (*options != '?'))
            options += 1;
        if (*options)
            options += 1;

        // Allocate space for the options string
        length = strlen(options);
        if (length)
        {
            fieldName = (char *)r4aMalloc(length + 1, "HTTP options");
            if (fieldName)
            {
                // Copy the options
                strcpy(fieldName, options);

                // Get the field value
                valueText = fieldName;
                while (*valueText && (*valueText != '='))
                    valueText += 1;
                nameLength = valueText - fieldName;
                if (*valueText)
                    valueText += 1;

                // Zero terminate the field name
                fieldName[nameLength] = 0;

                // Determine the end if the value
                endOfValue = valueText;
                while (*endOfValue && (*endOfValue != '&'))
                    endOfValue += 1;
                valueLength = endOfValue - valueText;
                valueText[valueLength] = 0;

                // Get the value
                if (sscanf(valueText, "%d", &value) != 1)
                {
                    webPage += "<p>ERROR: Invalid value for option ";
                    webPage += fieldName;
                    webPage += "\n</p>";
                }
                else
                {
                    // Walk the list of supported enable options
                    for (index = 0; index < enableCount; index++)
                    {
                        const char * field;

                        // Determine if this option was specified
                        field = enables[index]._tagText;
                        if (strcmp(field, fieldName) == 0)
                        {
                            // Process the option
                            switch (enables[index]._valueId)
                            {
                            case EID_AECE:
                                r4aCameraSetAutomaticExposureControlEnable(value);
                                enableValues[EID_AECE] = r4aCameraGetAutomaticExposureEnable();
                                break;

                            case EID_AWB:
                                r4aCameraSetAutomaticWhiteBalance(value);
                                enableValues[EID_AWB] = r4aCameraGetAutomaticWhiteBalanceEnable();
                                break;

                            case EID_BPC:
                                r4aCameraSetBpc(value);
                                enableValues[EID_BPC] = r4aCameraGetBpc();
                                break;

                            case EID_COLORBAR:
                                r4aCameraSetColorBar(value);
                                enableValues[EID_COLORBAR] = r4aCameraGetColorBarEnable();
                                break;

                            case EID_DCW:
                                r4aCameraSetDcw(value);
                                enableValues[EID_DCW] = r4aCameraGetDcw();
                                break;

                            case EID_ECE:
                                r4aCameraSetExposureControl(value);
                                enableValues[EID_ECE] = r4aCameraGetExposureControlEnable();
                                break;

                            case EID_GCE:
                                r4aCameraSetGainControlEnable(value);
                                enableValues[EID_GCE] = r4aCameraGetGainControlEnable();
                                break;

                            case EID_GMA:
                                r4aCameraSetRawGmaEnable(value);
                                enableValues[EID_GMA] = r4aCameraGetRawGmaEnable();
                                break;

                            case EID_LENS:
                                r4aCameraSetLensControlEnable(value);
                                enableValues[EID_LENS] = r4aCameraGetLensControlEnable();
                                break;

                            case EID_HMIRROR:
                                r4aCameraSetHorizontalMirror(value);
                                enableValues[EID_HMIRROR] = r4aCameraGetHorizontalMirror();
                                break;

                            case EID_VFLIP:
                                r4aCameraSetVerticalFlip(value);
                                enableValues[EID_VFLIP] = r4aCameraGetVerticalFlip();
                                break;

                            case EID_WPC:
                                r4aCameraSetWpcEnable(value);
                                enableValues[EID_WPC] = r4aCameraGetWpc();
                                break;
                            }
                            break;
                        }
                    }

                    // Walk the list of supported value options
                    for (index = 0; index < valueListCount; index++)
                    {
                        const char * field;

                        // Determine if this option was specified
                        field = valueList[index]._tagText;
                        if (strcmp(field, fieldName) == 0)
                        {
                            // Process the option
                            switch (valueList[index]._valueId)
                            {
                            case VID_AE_LEVEL:
                                r4aCameraSetAutomaticExposureLevel(value);
                                values[VID_AE_LEVEL] = r4aCameraGetAutomaticExposureLevel();
                                break;

                            case VID_AEC_VALUE:
                                r4aCameraSetAutomaticExposureControl(value);
                                values[VID_AEC_VALUE] = r4aCameraGetAutomaticExposureCorrection();
                                break;

                            case VID_AGC_GAIN:
                                r4aCameraSetAutomaticGainControl(value);
                                values[VID_AGC_GAIN] = r4aCameraGetAutomaticGainControl();
                                break;

                            case VID_BRIGHTNESS:
                                r4aCameraSetBrightness(value);
                                values[VID_BRIGHTNESS] = r4aCameraGetBrightness();
                                break;

                            case VID_CONTRAST:
                                r4aCameraSetContrast(value);
                                values[VID_CONTRAST] = r4aCameraGetContrast();
                                break;

                            case VID_DENOISE:
                                r4aCameraSetDenoise(value);
                                values[VID_DENOISE] = r4aCameraGetDenoise();
                                break;

                            case VID_GAIN_CEILING:
                                r4aCameraSetGainCeiling((gainceiling_t)value);
                                values[VID_GAIN_CEILING] = r4aCameraGetGainCeiling();
                                break;

                            case VID_QUALITY:
                                r4aCameraSetQuality(value);
                                values[VID_QUALITY] = r4aCameraGetQuality();
                                break;

                            case VID_SATURATION:
                                r4aCameraSetSaturation(value);
                                values[VID_SATURATION] = r4aCameraGetSaturation();
                                break;

                            case VID_SHARPNESS:
                                r4aCameraSetSharpness(value);
                                values[VID_SHARPNESS] = r4aCameraGetSharpness();
                                break;

                            case VID_EFFECTS:
                                r4aCameraSetSpecialEffect(value);
                                values[VID_EFFECTS] = r4aCameraGetSpecialEffect();
                                break;

                            case VID_WB_MODE:
                                r4aCameraSetWhiteBalanceMode(value);
                                values[VID_WB_MODE] = r4aCameraGetWhiteBalanceMode();
                                break;
                            }
                            break;
                        }
                    }

                    // Discard some frames
                    r4aCameraFrameBufferDiscard(4);
                }

                // Done with the options
                r4aFree(fieldName, "HTTP options");
            }
        }

        // Divide the page into three parts
        webPage += "<table>\n";

        // Display the image
        webPage += "  <tr><td colspan=3><img src=\"../jpeg\"></td></tr>\n";

        // Start the left half of the page
        webPage += "  <tr>\n";
        webPage += "    <td>\n";

        // Add the refresh button
        r4aOv2640WebPageAddButton(webPage, currentWebPage, "Refresh");

        // Display the image attributes
        r4aOv2640WebPageAddImageAttributes(webPage);

        // End the left half of the page
        webPage += "    </td>\n";

        // Separate the two halves (width in pixels)
        webPage += "    <td width=25>&nbsp;</td>\n";

        // Start the right half of the page
        webPage += "    <td>\n";
        webPage += "      <table>\n";

        // Add the controls to change the image enables
        webPage += "        <tr><td colspan=3><h3>Enables</h3></td></tr>\n";
        for (index = 0; index < enableCount; index++)
            r4aOv2640WebPageAddEnable(webPage,
                                      currentWebPage,
                                      enables[index]._titleText,
                                      enables[index]._tagText,
                                      "Toggle",
                                      enableValues[enables[index]._valueId]);

        // Add the controls to change the image values
        webPage += "        <tr><td colspan=3><h3>Values</h3></td></tr>\n";
        for (index = 0; index < valueListCount; index++)
            r4aOv2640WebPageAddValue(webPage,
                                     currentWebPage,
                                     valueList[index]._titleText,
                                     valueList[index]._tagText,
                                     "Select",
                                     valueList[index]._minValue,
                                     valueList[index]._maxValue,
                                     values[valueList[index]._valueId]);

        // End the right half of the page
        webPage += "      </table>\n";
        webPage += "    </td>\n";
        webPage += "  </tr>\n";
        webPage += "</table>\n";
    }

    // Display the supported frame sizes
    r4aOv2640WebPageAddFrameSizes(webPage);

    // Display the supported pixel formats
    r4aOv2640WebPageAddPixelFormats(webPage);

    // End the page
    webPage += "  </body>\n";
    webPage += "</html>\n";

    // Respond to the request with the web page
    httpd_resp_send(request, webPage.c_str(), HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

