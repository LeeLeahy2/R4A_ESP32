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
    .handler  = r4aOv2640JpegHandler,
    .user_ctx = (void *)&ov2640Parameters,
    .is_websocket = true,
    .handle_ws_control_frames = false,
    .supported_subprotocol = nullptr,
};

//****************************************
// Locals
//****************************************

bool ov2640JpegDisplayTime;  // Set to true to display the JPEG conversion time

//*********************************************************************
// Process the frame buffer
// Inputs:
//   frameBuffer: Buffer containing the raw image data
//   display: Address of Print object for output
// Outputs:
//   Returns true if the processing was successful and false upon error
bool ov2640ProcessFrameBuffer(camera_fb_t * frameBuffer,
                              Print * display)
{
    return true;
}

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
