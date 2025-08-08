/**********************************************************************
  Web_Server.ino

  Web server support routines
**********************************************************************/

//*********************************************************************
// Update the configuration
// Inputs:
//   object: Address of a R4A_WEB_SERVER data structure
//   config: Address of the HTTP config object
void webServerConfigUpdate(R4A_WEB_SERVER * object,
                           httpd_config_t * config)
{
    // Enable matching multiple web pages
    config->uri_match_fn = httpd_uri_match_wildcard;
}

//*********************************************************************
// Register the error handlers
// Inputs:
//   object: Address of a R4A_WEB_SERVER data structure
// Outputs:
//   Returns true if the all of the error handlers were installed and
//   false upon failure
bool webServerRegisterErrorHandlers(R4A_WEB_SERVER * object)
{
    esp_err_t error;

    for (int index = 0; index < r4aHttpErrorCount; index++)
    {
        error = httpd_register_err_handler(object->_webServer,
                                           r4aHttpError[index],
                                           r4aWebServerError);
        if (error != ESP_OK)
        {
            if (r4aWebServerDebug)
                r4aWebServerDebug->printf("ERROR: Failed to register error handler: %s, error: %d!\r\n",
                                          r4aHttpErrorName[index], error);
            return false;
        }
    }
    return true;
}

//*********************************************************************

// URI handler for getting uploaded files
const httpd_uri_t webServerFileDownloadUri =
{
    .uri       = DOWNLOAD_AREA "*",  // Match all URIs of type /path/to/file
    .method    = HTTP_GET,
    .handler   = r4aWebServerFileDownload,
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
bool webServerRegisterUriHandlers(R4A_WEB_SERVER * object)
{
    esp_err_t error;

    do
    {
        // Add the NVM download page
        error = httpd_register_uri_handler(object->_webServer,
                                           &webServerFileDownloadUri);
        if (error != ESP_OK)
        {
            if (r4aWebServerDebug)
                r4aWebServerDebug->printf("ERROR: Failed to register NVM download handler, error: %d!\r\n", error);
            break;
        }

        // Add the jpeg camera image page
        error = httpd_register_uri_handler(object->_webServer, &ov2640JpegPage);
        if (error != ESP_OK)
        {
            if (r4aWebServerDebug)
                r4aWebServerDebug->printf("ERROR: Failed to register JPEG handler, error: %d!\r\n", error);
            break;
        }

        // Successfully registered the handlers
        return true;
    } while (0);

    // Display the error message
    if (r4aWebServerDebug)
        r4aWebServerDebug->printf("ERROR: Failed to register URI handler, error: %d!\r\n", error);
    return false;
}
