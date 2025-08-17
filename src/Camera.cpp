/**********************************************************************
  Camera.cpp

  Robots-For-All (R4A)
  Camera support
**********************************************************************/

#include "R4A_ESP32.h"

//****************************************
// Constants
//****************************************

const R4A_FRAME_SIZE_TO_FORMAT r4aCameraFrameFormat[] =
{
    {FRAMESIZE_96X96,   R4A_FRAME_SIZE_96x96},      //   96 x   96
    {FRAMESIZE_QQVGA,   R4A_FRAME_SIZE_QQVGA},      //  160 x  120
    {FRAMESIZE_128X128, R4A_FRAME_SIZE_128x128},    //  128 x  128
    {FRAMESIZE_QCIF,    R4A_FRAME_SIZE_QCIF},       //  176 x  144
    {FRAMESIZE_HQVGA,   R4A_FRAME_SIZE_HQVGA},      //  240 x  176
    {FRAMESIZE_240X240, R4A_FRAME_SIZE_240x240},    //  240 x  240
    {FRAMESIZE_QVGA,    R4A_FRAME_SIZE_QVGA},       //  320 x  240
    {FRAMESIZE_320X320, R4A_FRAME_SIZE_320x320},    //  320 x  320
    {FRAMESIZE_CIF,     R4A_FRAME_SIZE_CIF},        //  400 x  296
    {FRAMESIZE_HVGA,    R4A_FRAME_SIZE_HVGA},       //  480 x  320
    {FRAMESIZE_VGA,     R4A_FRAME_SIZE_VGA},        //  640 x  480
    {FRAMESIZE_SVGA,    R4A_FRAME_SIZE_SVGA},       //  800 x  600
    {FRAMESIZE_XGA,     R4A_FRAME_SIZE_XGA},        // 1024 x  768
    {FRAMESIZE_HD,      R4A_FRAME_SIZE_HD},         // 1200 x  720
    {FRAMESIZE_SXGA,    R4A_FRAME_SIZE_SXGA},       // 1280 x 1024
    {FRAMESIZE_UXGA,    R4A_FRAME_SIZE_UXGA},       // 1600 x 1200
    {FRAMESIZE_FHD,     R4A_FRAME_SIZE_FHD},        // 1920 x 1080
    {FRAMESIZE_P_HD,    R4A_FRAME_SIZE_P_HD},       //  720 x 1280
    {FRAMESIZE_P_3MP,   R4A_FRAME_SIZE_P_3MP},      //  864 x 1536
    {FRAMESIZE_QXGA,    R4A_FRAME_SIZE_QXGA},       // 2048 x 1536
    {FRAMESIZE_QHD,     R4A_FRAME_SIZE_QHD},        // 2560 x 1440
    {FRAMESIZE_WQXGA,   R4A_FRAME_SIZE_WQXGA},      // 2560 x 1600
    {FRAMESIZE_P_FHD,   R4A_FRAME_SIZE_P_FHD},      //  864 x 1536
    {FRAMESIZE_QSXGA,   R4A_FRAME_SIZE_QSXGA},      // 2560 x 1920
    {FRAMESIZE_5MP,     R4A_FRAME_SIZE_5MP},        // 2592 x 1944
};
const int r4aCameraFrameFormatEntries = sizeof(r4aCameraFrameFormat)
                                      / sizeof(r4aCameraFrameFormat[0]);

const R4A_PIXEL_FORMAT_TO_FORMAT r4aCameraPixelFormat[] =
{
    {PIXFORMAT_RGB565,      R4A_PIXEL_FORMAT_RGB565},       // 2BPP/RGB565
    {PIXFORMAT_YUV422,      R4A_PIXEL_FORMAT_YUV422},       // 2BPP/YUV422
    {PIXFORMAT_YUV420,      R4A_PIXEL_FORMAT_YUV420},       // 1.5BPP/YUV420
    {PIXFORMAT_GRAYSCALE,   R4A_PIXEL_FORMAT_GRAYSCALE},    // 1BPP/GRAYSCALE
    {PIXFORMAT_JPEG,        R4A_PIXEL_FORMAT_JPEG},         // JPEG/COMPRESSED
    {PIXFORMAT_RGB888,      R4A_PIXEL_FORMAT_RGB888},       // 3BPP/RGB888
    {PIXFORMAT_RAW,         R4A_PIXEL_FORMAT_RAW},          // RAW
    {PIXFORMAT_RGB444,      R4A_PIXEL_FORMAT_RGB444},       // 3BP2P/RGB444
    {PIXFORMAT_RGB555,      R4A_PIXEL_FORMAT_RGB555},       // 3BP2P/RGB555
};
const int r4aCameraPixelFormatEntries = sizeof(r4aCameraPixelFormat)
                                      / sizeof(r4aCameraPixelFormat[0]);

//****************************************
// Locals
//****************************************

//*********************************************************************
// Lookup the frame size details
const R4A_CAMERA_FRAME * r4aCameraFindFrameDetails(framesize_t frameSize)
{
    // Verify the frameSize
    if ((frameSize >= 0) && (frameSize < FRAMESIZE_INVALID))
    {
        // Walk the translation table
        for (int index = 0; index < r4aCameraFrameFormatsEntries; index++)
            // Check for a matching frame size
            if (frameSize == r4aCameraFrameFormat[index]._frameSize)
            {
                index = r4aCameraFrameFormat[index]._r4aFrameFormat;
                return &r4aCameraFrameFormats[index];
            }
    }
    return nullptr;
}

//*********************************************************************
// Lookup the pixel format details
const R4A_CAMERA_PIXEL * r4aCameraFindPixelDetails(pixformat_t pixelFormat)
{
    // Verify the frameSize
    if ((pixelFormat >= 0) && (pixelFormat < r4aCameraPixelFormatEntries))
    {
        // Walk the translation table
        for (int index = 0; index < r4aCameraPixelFormatEntries; index++)
            if (pixelFormat == r4aCameraPixelFormat[index]._pixelFormat)
            {
                index = r4aCameraPixelFormat[index]._r4aPixelFormat;
                return &r4aCameraPixelFormats[index];
            }
    }
    return nullptr;
}

//*********************************************************************
// Get the initialization status
sensor_t * r4aCameraGetSensor(Print * display)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = esp_camera_sensor_get();
    if (sensor == nullptr)
        display->printf("ERROR: Failed to locate sensor structure!\r\n");
    return sensor;
}

//*********************************************************************
// Get a register value
int r4aCameraGetRegister(int regAddress, Print * display)
{
    sensor_t * sensor;   // Sensor routine pointers
    int status;

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(display);
    if (sensor == nullptr)
        return -1;

    // Return the initialization status
    status = sensor->get_reg(sensor, regAddress, 0xff);
    if ((status < 0) && display)
        display->printf("Failed to initialize sensor status structure!\r\n");
    return status;
}

//*********************************************************************
// Verify the camera tables
void r4aEsp32CameraVerifyTables()
{
    int index;

    // Verify the camera tables
    r4aCameraVerifyTables();

    // Verify the r4aCameraFrameFormat table
    if (FRAMESIZE_INVALID != r4aCameraFrameFormatEntries)
        r4aReportFatalError("Fix r4aCameraFrameFormat to match R4A_FRAME_SIZE_t!");
    for (index = 0; index < FRAMESIZE_INVALID; index++)
        if (index != r4aCameraFrameFormat[index]._frameSize)
        {
            Serial.printf("r4aCameraFrameFormat[%d]._frameSize != %d\r\n", index, index);
            r4aReportFatalError("Fix r4aCameraFrameFormat table entries!");
        }

    // Verify the r4aCameraPixelFormat table
    for (index = 0; index < r4aCameraPixelFormatEntries; index++)
        if (index != r4aCameraPixelFormat[index]._pixelFormat)
        {
            Serial.printf("r4aCameraPixelFormat[%d]._frameSize != %d\r\n", index, index);
            r4aReportFatalError("Fix r4aCameraPixelFormat table entries!");
        }
}
