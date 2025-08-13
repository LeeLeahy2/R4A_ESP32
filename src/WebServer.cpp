/**********************************************************************
  WebServer.cpp

  Robots-For-All (R4A)
  Web server implementation
**********************************************************************/

#include "R4A_ESP32.h"

//****************************************
// Globals
//****************************************

Print * r4aWebServerDebug;
bool r4aWebServerEnable = true;
const char * r4aWebServerDownloadArea;
const char * r4aWebServerNvmArea;

//*********************************************************************
// Check for extension
bool r4aWebServerCheckExtension(R4A_WEB_SERVER * object,
                                const char * path,
                                const char * extension)
{
    int extensionLength;
    int pathLength;

    // Get the lengths
    pathLength = strlen(path);
    extensionLength = strlen(extension);

    // Check for extension
    return ((pathLength >= extensionLength)
        && (strcmp(&path[pathLength - extensionLength], extension) == 0));
}

//*********************************************************************
// Handle the web server errors
esp_err_t r4aWebServerError (httpd_req_t *request,
                             httpd_err_code_t error)
{
    const R4A_TAG_NAME_T methodName[] =
    {
        {HTTP_GET, "GET"},
        {HTTP_POST, "POST"},
    };
    const int methodNameEntries = sizeof(methodName) / sizeof(methodName[0]);
    int index;
    const char * method;
    static char line[1000];

    // Get the method name
    method = nullptr;
    for (index = 0; index < methodNameEntries; index++)
        if (request->method == methodName[index].tag)
        {
            method = methodName[index].name;
            break;
        }

    // Send the error to the browser
    sprintf(line, "<html><body>%s: %s<br>\r\n%s\r\n</body>\r\n</html>\r\n",
            (method ? method : "Request"),
            request->uri,
            r4aHttpErrorName[error]);
    httpd_resp_send_err(request, error, line);

    // Display the error locally
    if (r4aWebServerDebug)
    {
        if (!method)
            method = "Unknown";
        r4aWebServerDebug->printf("Request: %s(%d) %s\r\n",
                                  method, request->method, request->uri);
        r4aWebServerDebug->printf("%s\r\n", r4aHttpErrorName[error]);
    }

    return ESP_OK;
}

//*********************************************************************
// Download a file from the robot to the browser
esp_err_t r4aWebServerFileDownload(httpd_req_t *request)
{
    uint8_t * buffer;
    const size_t bufferLength = 8192;
    size_t bytesRead;
    const char * dataType;
    File file;
    const char * path;
    esp_err_t status;
    R4A_WEB_SERVER * object;

    // Get the web server data structure
    object = (R4A_WEB_SERVER *)request->user_ctx;

    do
    {
        buffer = nullptr;

        // Get the file name
        path = request->uri;

        // Remove the download prefix
        if (strncmp(r4aWebServerNvmArea, path, strlen(r4aWebServerNvmArea)) != 0)
        {
            if (r4aWebServerDebug)
                r4aWebServerDebug->printf("ERROR: Not a file download request\r\n");
            httpd_resp_send_err(request, HTTPD_500_INTERNAL_SERVER_ERROR, "Not a file download request");
            break;
        }
        path = &path[strlen(r4aWebServerNvmArea) - 1];

        // Determine if the file exists
        if (LittleFS.exists(path) == false)
        {
            if (r4aWebServerDebug)
                r4aWebServerDebug->printf("ERROR: File does not exist!\r\n");
            httpd_resp_send_err(request, HTTPD_404_NOT_FOUND, "File does not exist");
            break;
        }

        // Determine the data type
        dataType = nullptr;
        if (r4aWebServerCheckExtension(object, path, ".txt")
            || r4aWebServerCheckExtension(object, path, ".log"))
            dataType = "text/plain";
        if (!dataType)
        {
            if (r4aWebServerDebug)
                r4aWebServerDebug->printf("ERROR: Unknowo file type!\r\n");
            httpd_resp_send_err(request, HTTPD_500_INTERNAL_SERVER_ERROR, "Unknown file type");
            break;
        }

        // Allocate the buffer
        buffer = (uint8_t *)r4aMalloc(bufferLength, "WebServer data buffer (buffer)");
        if (!buffer)
        {
            if (r4aWebServerDebug)
                r4aWebServerDebug->printf("ERROR: Failed to allocate the data buffer\r\n");
            httpd_resp_send_err(request, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to allocate data buffer");
            break;
        }

        // Open the file
        file = LittleFS.open(path, FILE_READ);
        if (!file)
        {
            if (r4aWebServerDebug)
                r4aWebServerDebug->printf("ERROR: Failed to open file %s\r\n", path);
            httpd_resp_send_err(request, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to open existing file");
            break;
        }

        // Verify that the file is not a directory
        if (file.isDirectory())
        {
            if (r4aWebServerDebug)
                r4aWebServerDebug->printf("ERROR: File %s is a directory!\r\n", path);
            httpd_resp_send_err(request, HTTPD_500_INTERNAL_SERVER_ERROR, "Unable to download a directory");
            break;
        }

        // Send the file contents to the browser
        do
        {
            // Read data from the file
            bytesRead = file.read(buffer, bufferLength);

            // Send a partial response
            if (bytesRead > 0)
                status = httpd_resp_send_chunk(request, (char *)buffer, bytesRead);
            else
                status = httpd_resp_send_chunk(request, NULL, 0);

            // Process the error
            if (status != ESP_OK)
            {
                if (r4aWebServerDebug)
                    r4aWebServerDebug->printf("ERROR: Failed to send %d data bytes to browser\r\n", bytesRead);
                httpd_resp_send_err(request, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send data to browser");
                break;
            }
        } while (bytesRead > 0);

        // Free the data buffer
        r4aFree((void **)&buffer, "WebServer data buffer (buffer)");

        // Close the file
        file.close();
        return ESP_OK;
    } while (0);

    // Close the file if necessary
    if (file)
        file.close();

    // Free the data buffer
    if (buffer)
        r4aFree((void **)&buffer, "WebServer data buffer (buffer)");

    // Failed to access the requested page
    return ESP_FAIL;
}

//*********************************************************************
// Start the web server
bool r4aWebServerStart(R4A_WEB_SERVER * object)
{
    esp_err_t error;

    // Generate default configuration
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = object->_port;

    // Start the httpd server
    do
    {
        // Update the configuration
        object->_configUpdate(object, &config);

        // Start the web server
        error = httpd_start(&object->_webServer, &config);
        if (error != ESP_OK)
        {
            if (r4aWebServerDebug)
                r4aWebServerDebug->printf("ERROR: Failed to start the web server, error: %d!\r\n",
                                          error);
            break;
        }
        if (r4aWebServerDebug)
            r4aWebServerDebug->printf("Started the web server\r\n");

        // Register URI handlers
        if (!object->_registerUriHandlers(object))
            break;
        if (r4aWebServerDebug)
            r4aWebServerDebug->printf("Registered URI handlers\r\n");

        // Register the error handlers
        if (!object->_registerErrorHandlers(object))
            break;
        if (r4aWebServerDebug)
            r4aWebServerDebug->printf("Registered error handlers\r\n");

        // Display the web server path
        Serial.printf("Starting web-server: http://%s:%d\r\n",
                      WiFi.localIP().toString().c_str(),
                      object->_port);

        // Successful server initialization
        return true;
    } while (0);

    // Stop the web server
    r4aWebServerStop(object);

    // The web server failed to start
    return false;
}

//*********************************************************************
// Stop the web server
void r4aWebServerStop(R4A_WEB_SERVER * object)
{
    if (object->_webServer)
    {
        if (r4aWebServerDebug)
            Serial.println("Stopping the web-server");

        // Stop the web server
        httpd_stop(object->_webServer);
        object->_webServer = nullptr;
    }
}

//*********************************************************************
// Update the camera processing state
void r4aWebServerUpdate(R4A_WEB_SERVER * object, bool wifiConnected)
{
    // Start the web server if necessary
    if ((!object->_webServer) && wifiConnected && r4aWebServerEnable)
        r4aWebServerStart(object);

    // Stop the web server if necessary
    if (object->_webServer && ((!wifiConnected) || (!r4aWebServerEnable)))
        r4aWebServerStop(object);
}
