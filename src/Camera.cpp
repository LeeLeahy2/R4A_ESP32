/**********************************************************************
  Camera.cpp

  Robots-For-All (R4A)
  Camera support
**********************************************************************/

#include "R4A_ESP32.h"

//****************************************
// Constants
//****************************************

const R4A_FRAME_SIZE_TO_FORMAT r4aEsp32CameraFrameFormat[] =
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
const int r4aEsp32CameraFrameFormatEntries = sizeof(r4aEsp32CameraFrameFormat)
                                           / sizeof(r4aEsp32CameraFrameFormat[0]);

const R4A_PIXEL_FORMAT_TO_FORMAT r4aEsp32CameraPixelFormat[] =
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
const int r4aEsp32CameraPixelFormatEntries = sizeof(r4aEsp32CameraPixelFormat)
                                           / sizeof(r4aEsp32CameraPixelFormat[0]);

//*********************************************************************
// Lookup the frame size details
const R4A_CAMERA_FRAME * r4aCameraFindFrameDetails(framesize_t frameSize)
{
    // Verify the frameSize
    if ((frameSize >= 0) && (frameSize < FRAMESIZE_INVALID))
    {
        // Walk the translation table
        for (int index = 0; index < r4aEsp32CameraFrameFormatEntries; index++)
            // Check for a matching frame size
            if (frameSize == r4aEsp32CameraFrameFormat[index]._frameSize)
                return r4aCameraFindFrameSize(r4aEsp32CameraFrameFormat[index]._r4aFrameFormat);
    }
    return nullptr;
}

//*********************************************************************
// Lookup the pixel format details
const R4A_CAMERA_PIXEL * r4aCameraFindPixelDetails(pixformat_t pixelFormat)
{
    // Verify the frameSize
    if ((pixelFormat >= 0) && (pixelFormat < r4aEsp32CameraPixelFormatEntries))
    {
        // Walk the translation table
        for (int index = 0; index < r4aEsp32CameraPixelFormatEntries; index++)
            if (pixelFormat == r4aEsp32CameraPixelFormat[index]._pixelFormat)
                return r4aCameraFindPixelFormat(r4aEsp32CameraPixelFormat[index]._r4aPixelFormat);
    }
    return nullptr;
}

//*********************************************************************
// Discard multiple frame buffers
void r4aCameraFrameBufferDiscard(int framesToDiscard)
{
    camera_fb_t * frameBuffer;
    int frameCount;
    uint32_t startMsec;

    frameCount = 0;
    startMsec = millis();
    while (((millis() - startMsec) < 1000) && (frameCount < framesToDiscard))
    {
        frameBuffer = r4aCameraFrameBufferGet();
        if (frameBuffer)
        {
            r4aCameraFrameBufferFree(frameBuffer);
            frameCount += 1;
        }
    }
}

//*********************************************************************
// Free the frame buffer
void r4aCameraFrameBufferFree(camera_fb_t * frameBuffer)
{
    esp_camera_fb_return(frameBuffer);
}

//*********************************************************************
// Get a frame buffer
camera_fb_t * r4aCameraFrameBufferGet()
{
    camera_fb_t * frameBuffer;
    int frameCount;
    uint32_t startMsec;

    startMsec = millis();
    while ((millis() - startMsec) < 1000)
    {
        frameBuffer = esp_camera_fb_get();
        if (frameBuffer)
            break;
    }
    return frameBuffer;
}

//*********************************************************************
// Get the automatic exposure correction value
int r4aCameraGetAutomaticExposureCorrection()
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Return the automatic exposure correction value
    return (int)sensor->status.aec;
}

//*********************************************************************
// Get the automatic exposure enable/disable
int r4aCameraGetAutomaticExposureEnable()
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Return the automatic exposure enable/disable value
    return (int)sensor->status.aec2;
}

//*********************************************************************
// Get the automatic exposure level
int r4aCameraGetAutomaticExposureLevel()
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Return the automatic exposure level
    return (int)sensor->status.ae_level;
}

//*********************************************************************
// Get the automatic gain control value
int r4aCameraGetAutomaticGainControl()
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Return the automatic gain control value
    return (int)sensor->status.agc_gain;
}

//*********************************************************************
// Get the automatic white balance enable/disable value
int r4aCameraGetAutomaticWhiteBalanceEnable()
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Return the automatic white balance value
    return (int)sensor->status.awb;
}

//*********************************************************************
// Get the bpc value
int r4aCameraGetBpc()
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Return the bpc value
    return (int)sensor->status.bpc;
}

//*********************************************************************
// Get the brightness value
int r4aCameraGetBrightness()
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Return the brightness value
    return (int)sensor->status.brightness;
}

//*********************************************************************
// Get the external clock (XCLK) frequency
int r4aCameraGetClockHz()
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Return the XCLK frequency in Hertz
    return (int)sensor->xclk_freq_hz;
}

//*********************************************************************
// Get the color bar enable/disable value
int r4aCameraGetColorBarEnable()
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Return the color bar value
    return (int)sensor->status.colorbar;
}

//*********************************************************************
// Get the contrast value
int r4aCameraGetContrast()
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Return the contrast
    return (int)sensor->status.contrast;
}

//*********************************************************************
// Get the dcw value
int r4aCameraGetDcw()
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Return the dcw value
    return (int)sensor->status.dcw;
}

//*********************************************************************
// Get the denoise value
int r4aCameraGetDenoise()
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Return the denoise value
    return (int)sensor->status.denoise;
}

//*********************************************************************
// Get the exposure control enable value
int r4aCameraGetExposureControlEnable()
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Return the exposure control enable value
    return (int)sensor->status.aec;
}

//*********************************************************************
// Get the frame size
int r4aCameraGetFrameSize()
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Return the frame size value
    return (int)sensor->status.framesize;
}

//*********************************************************************
// Get the gain ceiling
int r4aCameraGetGainCeiling()
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Return the gain ceiling value
    return (int)sensor->status.gainceiling;
}

//*********************************************************************
// Get the gain control enable value
int r4aCameraGetGainControlEnable()
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Return the gain control enable value
    return (int)sensor->status.agc;
}

//*********************************************************************
// Get the horizontal mirror state
int r4aCameraGetHorizontalMirror()
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Return the horizontal mirror value
    return (int)sensor->status.hmirror;
}

//*********************************************************************
// Get the lens control enable value
int r4aCameraGetLensControlEnable()
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Return the lens control enable value
    return (int)sensor->status.lenc;
}

//*********************************************************************
// Get the pixel format
int r4aCameraGetPixelFormat()
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Return the pixel format value
    return (int)sensor->pixformat;
}

//*********************************************************************
// Get the quality
int r4aCameraGetQuality()
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Return the quality value
    return (int)sensor->status.quality;
}

//*********************************************************************
// Get the raw GMA enable value
int r4aCameraGetRawGmaEnable()
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Return the raw GMA enable value
    return (int)sensor->status.raw_gma;
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
        return R4A_ERROR_NO_SENSOR;

    // Return the initialization status
    status = sensor->get_reg(sensor, regAddress, 0xff);
    if ((status < 0) && display)
        display->printf("Failed to initialize sensor status structure!\r\n");
    return status;
}

//*********************************************************************
// Get the saturation level
int r4aCameraGetSaturation()
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Return the saturation level
    return (int)sensor->status.saturation;
}

//*********************************************************************
// Get the sensor data structure
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
// Get the sharpness
int r4aCameraGetSharpness()
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Return the sharpness
    return (int)sensor->status.sharpness;
}

//*********************************************************************
// Get the special effect
int r4aCameraGetSpecialEffect()
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Return the special effect value
    return (int)sensor->status.special_effect;
}

//*********************************************************************
// Get the vertical flip state
int r4aCameraGetVerticalFlip()
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Return the vertical flip value
    return (int)sensor->status.vflip;
}

//*********************************************************************
// Get the white balance mode
int r4aCameraGetWhiteBalanceMode()
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Return the white balance mode
    return (int)sensor->status.wb_mode;
}

//*********************************************************************
// Get the WPC value
int r4aCameraGetWpc()
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Return the WPC value
    return (int)sensor->status.wpc;
}

//*********************************************************************
// Get the initialization status
int r4aCameraInitStatus(Print * display)
{
    sensor_t * sensor;   // Sensor routine pointers
    int status;

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(display);
    if (sensor == nullptr)
        return -1;

    // Return the initialization status
    status = sensor->init_status(sensor);
    if (status == 0)
        display->printf("Sensor status initialized!\r\n");
    else
        display->printf("Failed to initialize sensor status structure!\r\n");
    return status;
}

//*********************************************************************
// Set the automatic exposure control gain value
int r4aCameraSetAutomaticExposureControl(int gain)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Set the automatic exposure control gain value
    return sensor->set_aec_value(sensor, gain);
}

//*********************************************************************
// Enable or disable automatic exposure control
int r4aCameraSetAutomaticExposureControlEnable(int enable)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Set the automatic exposure control 2
    return sensor->set_aec2(sensor, enable);
}

//*********************************************************************
// Set the automatic exposure level
int r4aCameraSetAutomaticExposureLevel(int level)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Set the automatic exposure level
    return sensor->set_ae_level(sensor, level);
}

//*********************************************************************
// Set the automatic gain control
int r4aCameraSetAutomaticGainControl(int gain)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Set the automatic gain control
    return sensor->set_agc_gain(sensor, gain);
}

//*********************************************************************
// Enable or disable automatic white balance
int r4aCameraSetAutomaticWhiteBalance(int enable)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Set the automatic white balance
    return sensor->set_whitebal(sensor, enable);
}

//*********************************************************************
// Enable or disable bpc
int r4aCameraSetBpc(int enable)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Set the bpc
    return sensor->set_bpc(sensor, enable);
}

//*********************************************************************
// Set the brightness
int r4aCameraSetBrightness(int level)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Set the brightness
    return sensor->set_brightness(sensor, level);
}

//*********************************************************************
// Enable or disable color bar
int r4aCameraSetColorBar(int enable)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Set the color bar
    return sensor->set_colorbar(sensor, enable);
}

//*********************************************************************
// Set the contrast
int r4aCameraSetContrast(int level)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Set the contrast
    return sensor->set_contrast(sensor, level);
}

//*********************************************************************
// Enable or disable dcw
int r4aCameraSetDcw(int enable)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Set the dcw
    return sensor->set_dcw(sensor, enable);
}

//*********************************************************************
// Set the denoise
int r4aCameraSetDenoise(int level)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Set the denoise level
    return sensor->set_denoise(sensor, level);
}

//*********************************************************************
// Enable or disable exposure control
int r4aCameraSetExposureControl(int enable)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Set the exposure control
    return sensor->set_exposure_ctrl(sensor, enable);
}

//*********************************************************************
// Set the external clock frequency
int r4aCameraSetExternalClockFrequency(int timer, int xclk)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Set the external clock frequency
    return sensor->set_xclk(sensor, timer, xclk);
}

//*********************************************************************
// Set frame size
int r4aCameraSetFrameSize(framesize_t framesize)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Set the frame size
    return sensor->set_framesize(sensor, framesize);
}

//*********************************************************************
// Set gain ceiling
int r4aCameraSetGainCeiling(gainceiling_t gainceiling)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Set the gain ceiling
    return sensor->set_gainceiling(sensor, gainceiling);
}

//*********************************************************************
// Enable or disable gain control
int r4aCameraSetGainControlEnable(int enable)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Set the gain control enable
    return sensor->set_gain_ctrl(sensor, enable);
}

//*********************************************************************
// Enable or disable horizontal mirror
int r4aCameraSetHorizontalMirror(int enable)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Set the horizontal mirror
    return sensor->set_hmirror(sensor, enable);
}

//*********************************************************************
// Enable or disable lens control
int r4aCameraSetLensControlEnable(int enable)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Set the lens control enable
    return sensor->set_lenc(sensor, enable);
}

//*********************************************************************
// Set pixel format
int r4aCameraSetPixelFormat(pixformat_t pixformat)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Set the pixel format
    return sensor->set_pixformat(sensor, pixformat);
}

//*********************************************************************
// Set PLL frequency
int r4aCameraSetPllFrequency(int bypass,
                             int mul,
                             int sys,
                             int root,
                             int pre,
                             int seld5,
                             int pclken,
                             int pclk)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Set the PLL frequency
    return sensor->set_pll(sensor,
                           bypass,
                           mul,
                           sys,
                           root,
                           pre,
                           seld5,
                           pclken,
                           pclk);
}

//*********************************************************************
// Set quality
int r4aCameraSetQuality(int quality)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Set the quality
    return sensor->set_quality(sensor, quality);
}

//*********************************************************************
// Enable or disable raw GMA
int r4aCameraSetRawGmaEnable(int enable)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Set the raw GMA enable
    return sensor->set_raw_gma(sensor, enable);
}

//*********************************************************************
// Set raw resolution
int r4aCameraSetRawResolution(int startX,
                              int startY,
                              int endX,
                              int endY,
                              int offsetX,
                              int offsetY,
                              int totalX,
                              int totalY,
                              int outputX,
                              int outputY,
                              bool scale,
                              bool binning)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Set the raw resolution
    return sensor->set_res_raw(sensor,
                               startX,
                               startY,
                               endX,
                               endY,
                               offsetX,
                               offsetY,
                               totalX,
                               totalY,
                               outputX,
                               outputY,
                               scale,
                               binning);
}

//*********************************************************************
// Set register
int r4aCameraSetRegister(int reg, int mask, int value)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Set the register
    return sensor->set_reg(sensor, reg, mask, value);
}

//*********************************************************************
// Set saturation
int r4aCameraSetSaturation(int level)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Set the saturation
    return sensor->set_saturation(sensor, level);
}

//*********************************************************************
// Set sharpness
int r4aCameraSetSharpness(int level)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Set the sharpness
    return sensor->set_sharpness(sensor, level);
}

//*********************************************************************
// Set special effect
int r4aCameraSetSpecialEffect(int effect)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Set the special effect
    return sensor->set_special_effect(sensor, effect);
}

//*********************************************************************
// Enable or disable vertical flip
int r4aCameraSetVerticalFlip(int enable)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Set the vertical flip
    return sensor->set_vflip(sensor, enable);
}

//*********************************************************************
// Set white balance mode
int r4aCameraSetWhiteBalanceMode(int mode)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Set the white balance mode
    return sensor->set_wb_mode(sensor, mode);
}

//*********************************************************************
// Enable or disable WPC
int r4aCameraSetWpcEnable(int enable)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(nullptr);
    if (sensor == nullptr)
        return R4A_ERROR_NO_SENSOR;

    // Set the WPC enable
    return sensor->set_wpc(sensor, enable);
}

//*********************************************************************
// Verify the camera tables
void r4aEsp32CameraVerifyTables()
{
    uint64_t foundEsp32;
    uint64_t foundR4a;
    int index;

    // Verify the camera tables
    r4aCameraVerifyTables();

    // Check for duplicates in the r4aEsp32CameraFrameFormat table
    foundEsp32 = 0;
    foundR4a = 0;
    for (index = 0; index < r4aEsp32CameraFrameFormatEntries; index++)
    {
        if (foundEsp32 & (1 << r4aEsp32CameraFrameFormat[index]._frameSize))
        {
            Serial.printf("ERROR: Duplicate _frameSize entry at r4aEsp32CameraFrameFormat[%d]\r\n", index);
            r4aReportFatalError("Duplicate _frameSize entry in r4aEsp32CameraFrameFormat table!");
        }
        else if (foundR4a & (1 << r4aEsp32CameraFrameFormat[index]._r4aFrameFormat))
        {
            Serial.printf("ERROR: Duplicate _r4aFrameFormat entry at r4aEsp32CameraFrameFormat[%d]\r\n", index);
            r4aReportFatalError("Duplicate _r4aFrameFormat entry in r4aEsp32CameraFrameFormat table!");
        }
        else
        {
            foundEsp32 |= 1 << r4aEsp32CameraFrameFormat[index]._frameSize;
            foundR4a |= 1 << r4aEsp32CameraFrameFormat[index]._r4aFrameFormat;
        }
    }

    // Verify the r4aEsp32CameraFrameFormat table size
    framesize_t frameMax = FRAMESIZE_INVALID;
    if (r4aEsp32CameraFrameFormatEntries != frameMax)
    {
        Serial.printf("ERROR: Too %s entries in r4aEsp32CameraFrameFormat table!\r\n",
                      r4aEsp32CameraFrameFormatEntries > frameMax ? "many" : "few");
        r4aReportFatalError("Fix r4aEsp32CameraFrameFormat table entries to match framesize_t!");
    }

    // Verify the r4aEsp32CameraPixelFormat table
    foundEsp32 = 0;
    foundR4a = 0;
    for (index = 0; index < r4aEsp32CameraPixelFormatEntries; index++)
    {
        if (foundEsp32 & (1 << r4aEsp32CameraPixelFormat[index]._pixelFormat))
        {
            Serial.printf("ERROR: Duplicate _pixelFormat entry at r4aEsp32CameraPixelFormat[%d]\r\n", index);
            r4aReportFatalError("Duplicate _pixelFormat entry in r4aEsp32CameraPixelFormat table!");
        }
        else if (foundR4a & (1 << r4aEsp32CameraPixelFormat[index]._r4aPixelFormat))
        {
            Serial.printf("ERROR: Duplicate _r4aPixelFormat entry at r4aEsp32CameraPixelFormat[%d]\r\n", index);
            r4aReportFatalError("Duplicate _r4aPixelFormat entry in r4aEsp32CameraPixelFormat table!");
        }
        else
        {
            foundEsp32 |= 1 << r4aEsp32CameraPixelFormat[index]._pixelFormat;
            foundR4a |= 1 << r4aEsp32CameraPixelFormat[index]._r4aPixelFormat;
        }
    }

    // Verify the r4aEsp32CameraPixelFormat table size
    int pixelMax = (int)PIXFORMAT_RGB555 + 1;
    if (r4aEsp32CameraPixelFormatEntries != pixelMax)
    {
        Serial.printf("ERROR: Too %s entries in r4aEsp32CameraPixelFormat table!\r\n",
                      r4aEsp32CameraPixelFormatEntries > pixelMax ? "many" : "few");
        r4aReportFatalError("Fix r4aEsp32CameraPixelFormat table entries to match pixformat_t!");
    }
}
