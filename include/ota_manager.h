#pragma once

// Configure OTA callbacks and start the OTA service.
void initOTA();

// Poll OTA events; call frequently from the main loop.
void handleOTA();