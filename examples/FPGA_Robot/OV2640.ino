/**********************************************************************
  OV2640.ino

  OV2640 camera support routines
**********************************************************************/

#ifdef  USE_OV2640

//****************************************
// URI structures
//****************************************

#define OV2640_DETAILS_PAGE     "ov2640-details"
#define OV2640_JPEG_PAGE        "ov2640-jpeg"

// URI handler structure for GET OV2640_JPEG_PAGE
const httpd_uri_t ov2640JpegPageUri =
{
    .uri      = OV2640_JPEG_PAGE,
    .method   = HTTP_GET,
    .handler  = r4aOv2640JpegHandler,
    .user_ctx = (void *)ov2640JpegHandlerProcessFrameBuffer,
    .is_websocket = true,
    .handle_ws_control_frames = false,
    .supported_subprotocol = nullptr,
};

// URI handler for getting OV2640 details
const httpd_uri_t ov2640DetailsUri =
{
    .uri       = OV2640_DETAILS_PAGE,
    .method    = HTTP_GET,
    .handler   = ov2640WebPage,
    .user_ctx  = (void *)&webServer,
    .is_websocket = true,
    .handle_ws_control_frames = false,
    .supported_subprotocol = nullptr,
};

//*********************************************************************
// Register the URI handlers
// Inputs:
//   object: Address of a R4A_WEB_SERVER data structure
// Outputs:
//   Returns true if the all of the URI handlers were installed and
//   false upon failure
bool ov2640RegisterUriHandlers(R4A_WEB_SERVER * object)
{
    esp_err_t error;

    do
    {
#ifdef  USE_OV2640
        // Verify that the camera is enabled and initialized
        if (ov2640Present)
        {
            // Add the OV2640 JPEG image page
            error = httpd_register_uri_handler(object->_webServer, &ov2640JpegPageUri);
            if (error != ESP_OK)
            {
                if (r4aWebServerDebug)
                    r4aWebServerDebug->printf("ERROR: Failed to register JPEG handler, error: %d!\r\n", error);
                break;
            }

            // Add the OV2640 details page
            error = httpd_register_uri_handler(object->_webServer, &ov2640DetailsUri);
            if (error != ESP_OK)
            {
                if (r4aWebServerDebug)
                    r4aWebServerDebug->printf("ERROR: Failed to register OV2640 details page, error: %d!\r\n", error);
                break;
            }
        }
#endif  // USE_OV2640

        // Successfully registered the handlers
        return true;
    } while (0);

    // Display the error message
    if (r4aWebServerDebug)
        r4aWebServerDebug->printf("ERROR: Failed to register URI handler, error: %d!\r\n", error);
    return false;
}

//*********************************************************************
// Display the camera web page
esp_err_t ov2640WebPage(httpd_req_t *request)
{
    // Return the camera status web page if camera is online
    if (ov2640Present)
        return r4aOv2640WebPage(request);

    // Return an error page

    // Build the web page
    String webPage("");
    webPage += "<!DOCTYPE html>\n";
    webPage += "<html lang=\"en\">\n";
    webPage += "  <head>\n";
    webPage += "    <meta charset=\"utf-8\">\n";
    webPage += "    <title>OV2640 Details</title>\n";
    webPage += "  </head>\n";
    webPage += "  <body>\n";
    webPage += "<h1>ERROR: OV2640 not responding to address ";
    char i2cAddress[16];
    sprintf(i2cAddress, "0x%x", OV2640_I2C_ADDRESS);
    webPage += i2cAddress;
    webPage += " on the I2C bus!</h1>\n";
    webPage += "  </body>\n";
    webPage += "</html>\n";

    // Respond to the request with the web page
    httpd_resp_send(request, webPage.c_str(), HTTPD_RESP_USE_STRLEN);
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

#endif  // USE_OV2640
