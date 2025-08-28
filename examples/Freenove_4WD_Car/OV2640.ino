/**********************************************************************
  OV2640.ino

  OV2640 camera support routines
**********************************************************************/

#ifdef  USE_OV2640

//****************************************
// Constants
//****************************************

// URI handler structure for GET /jpeg
const httpd_uri_t ov2640JpegPage =
{
    .uri      = "/jpeg",
    .method   = HTTP_GET,
    .handler  = r4aOv2640JpegHandler,
    .user_ctx = (void *)ov2640ProcessWebServerFrameBuffer,
    .is_websocket = true,
    .handle_ws_control_frames = false,
    .supported_subprotocol = nullptr,
};

// URI handler for getting OV2640 details
const httpd_uri_t ov2640DetailsUri =
{
    .uri       = "/OV2640",
    .method    = HTTP_GET,
    .handler   = r4aOv2640WebPage,
    .user_ctx  = (void *)&webServer,
    .is_websocket = true,
    .handle_ws_control_frames = false,
    .supported_subprotocol = nullptr,
};

//*********************************************************************
// Process the web server's frame buffer
// Inputs:
//   frameBuffer: Buffer containing the raw image data
//   display: Address of Print object for output
// Outputs:
//   Returns true if the processing was successful and false upon error
bool ov2640ProcessWebServerFrameBuffer(camera_fb_t * frameBuffer,
                                       Print * display)
{
    return true;
}

#endif  // USE_OV2640
