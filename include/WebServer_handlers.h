#ifndef WEBSERVER_HANDLERS_H
#define WEBSERVER_HANDLERS_H

#include <WebServer.h>
#include <WiFi.h>
#include <SD.h>
#include <FS.h>

// External variables that need to be defined in main file
extern WebServer server;
extern bool sdCardAvailable;
extern const char* csvFilename;
extern const char* socDecreaseFilename;

//----------------------------------------------------------------------------------------
//        Web server handlers for SD card data logging
//----------------------------------------------------------------------------------------

// Root page - show links and SD card info
void handleRoot() {
  // Send HTML in chunks to avoid String concatenation and heap fragmentation
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  
  server.sendContent("<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>");
  server.sendContent("<title>Ioniq5 Data Logger</title>");
  server.sendContent("<style>body{font-family:Arial;margin:20px;background:#f0f0f0;}");
  server.sendContent("h1{color:#333;}.card{background:white;padding:20px;margin:10px 0;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,0.1);}");
  server.sendContent("a{display:inline-block;padding:10px 20px;margin:5px;background:#007bff;color:white;text-decoration:none;border-radius:5px;}");
  server.sendContent("a:hover{background:#0056b3;}</style></head><body>");
  server.sendContent("<h1>Ioniq5 OBD2 Data Logger</h1>");
  
  server.sendContent("<div class='card'><h2>SD Card Status</h2>");
  if (sdCardAvailable) {
    char buffer[128];
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    uint64_t usedSize = SD.usedBytes() / (1024 * 1024);
    
    server.sendContent("<p>Status: <strong style='color:green'>Connected</strong></p>");
    snprintf(buffer, sizeof(buffer), "<p>Card Size: %lu MB</p>", (unsigned long)cardSize);
    server.sendContent(buffer);
    snprintf(buffer, sizeof(buffer), "<p>Used: %lu MB</p>", (unsigned long)usedSize);
    server.sendContent(buffer);
    
    if (SD.exists(csvFilename)) {
      File file = SD.open(csvFilename, FILE_READ);
      if (file) {
        snprintf(buffer, sizeof(buffer), "<p>Log File Size: %d bytes</p>", file.size());
        server.sendContent(buffer);
        file.close();
      }
    }
  } else {
    server.sendContent("<p>Status: <strong style='color:red'>No SD Card</strong></p>");
  }
  server.sendContent("</div>");
  
  server.sendContent("<div class='card'><h2>Main Log Actions</h2>");
  if (sdCardAvailable && SD.exists(csvFilename)) {
    server.sendContent("<a href='/download'>Download Main Log</a>");
    server.sendContent("<a href='/view'>View Last 50 Lines</a>");
    server.sendContent("<a href='/delete' onclick='return confirm(\"Delete main log file?\")'>Delete Main Log</a>");
  } else {
    server.sendContent("<p>No main log file available</p>");
  }
  server.sendContent("</div>");
  
  server.sendContent("<div class='card'><h2>SoC Decrease Log Actions</h2>");
  if (sdCardAvailable && SD.exists(socDecreaseFilename)) {
    server.sendContent("<a href='/download_soc'>Download SoC Log</a>");
    server.sendContent("<a href='/view_soc'>View Last 50 Lines</a>");
    server.sendContent("<a href='/delete_soc' onclick='return confirm(\"Delete SoC decrease log file?\")'>Delete SoC Log</a>");
  } else {
    server.sendContent("<p>No SoC decrease log file available</p>");
  }
  server.sendContent("</div>");
  
  server.sendContent("<div class='card'><h2>System Info</h2>");
  char buffer[128];
  snprintf(buffer, sizeof(buffer), "<p>IP Address: %s</p>", WiFi.localIP().toString().c_str());
  server.sendContent(buffer);
  snprintf(buffer, sizeof(buffer), "<p>Free Heap: %u bytes</p>", ESP.getFreeHeap());
  server.sendContent(buffer);
  server.sendContent("</div>");
  
  server.sendContent("</body></html>");
  server.sendContent("");  // End chunked transfer
}

// Download the CSV file
void handleDownload() {
  if (!sdCardAvailable || !SD.exists(csvFilename)) {
    server.send(404, "text/plain", "Log file not found");
    return;
  }
  
  File file = SD.open(csvFilename, FILE_READ);
  if (!file) {
    server.send(500, "text/plain", "Failed to open file");
    return;
  }
  
  server.sendHeader("Content-Disposition", "attachment; filename=ioniq5_log.csv");
  server.streamFile(file, "text/csv");
  file.close();
}

// View last 50 lines
void handleView() {
  if (!sdCardAvailable || !SD.exists(csvFilename)) {
    server.send(404, "text/plain", "Log file not found");
    return;
  }
  
  File file = SD.open(csvFilename, FILE_READ);
  if (!file) {
    server.send(500, "text/plain", "Failed to open file");
    return;
  }
  
  // Send HTML header in chunks
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  
  server.sendContent("<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>");
  server.sendContent("<title>View Log</title>");
  server.sendContent("<style>body{font-family:monospace;margin:20px;background:#f0f0f0;}");
  server.sendContent("pre{background:white;padding:15px;border-radius:5px;overflow-x:auto;}</style></head><body>");
  server.sendContent("<h2>Last 50 Lines</h2><a href='/'>Back</a><pre>");
  
  // Read last 50 lines using fixed buffers instead of String
  static char lines[50][256];  // 50 lines, max 256 chars each
  int lineCount = 0;
  int currentPos = 0;
  
  while (file.available()) {
    char c = file.read();
    if (c == '\n') {
      lines[lineCount % 50][currentPos] = '\0';
      lineCount++;
      currentPos = 0;
    } else if (currentPos < 255) {
      lines[lineCount % 50][currentPos++] = c;
    }
  }
  
  int startLine = (lineCount > 50) ? lineCount - 50 : 0;
  for (int i = 0; i < min(lineCount, 50); i++) {
    int idx = (startLine + i) % 50;
    server.sendContent(lines[idx]);
    server.sendContent("\n");
  }
  
  server.sendContent("</pre></body></html>");
  server.sendContent("");  // End chunked transfer
  file.close();
}

// Delete log file
void handleDelete() {
  if (!sdCardAvailable) {
    server.send(404, "text/plain", "SD card not available");
    return;
  }
  
  if (SD.remove(csvFilename)) {
    server.send(200, "text/html", "<html><body><h2>Log file deleted</h2><a href='/'>Back</a></body></html>");
  } else {
    server.send(500, "text/plain", "Failed to delete file");
  }
}

// Download the SoC decrease CSV file
void handleDownloadSoC() {
  if (!sdCardAvailable || !SD.exists(socDecreaseFilename)) {
    server.send(404, "text/plain", "SoC decrease log file not found");
    return;
  }
  
  File file = SD.open(socDecreaseFilename, FILE_READ);
  if (!file) {
    server.send(500, "text/plain", "Failed to open file");
    return;
  }
  
  server.sendHeader("Content-Disposition", "attachment; filename=ioniq5_soc_decrease.csv");
  server.streamFile(file, "text/csv");
  file.close();
}

// View last 50 lines of SoC decrease log
void handleViewSoC() {
  if (!sdCardAvailable || !SD.exists(socDecreaseFilename)) {
    server.send(404, "text/plain", "SoC decrease log file not found");
    return;
  }
  
  File file = SD.open(socDecreaseFilename, FILE_READ);
  if (!file) {
    server.send(500, "text/plain", "Failed to open file");
    return;
  }
  
  // Send HTML header in chunks
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  
  server.sendContent("<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>");
  server.sendContent("<title>View SoC Decrease Log</title>");
  server.sendContent("<style>body{font-family:monospace;margin:20px;background:#f0f0f0;}");
  server.sendContent("pre{background:white;padding:15px;border-radius:5px;overflow-x:auto;}</style></head><body>");
  server.sendContent("<h2>SoC Decrease Log - Last 50 Lines</h2><a href='/'>Back</a><pre>");
  
  // Read last 50 lines using fixed buffers instead of String
  static char lines[50][256];  // 50 lines, max 256 chars each
  int lineCount = 0;
  int currentPos = 0;
  
  while (file.available()) {
    char c = file.read();
    if (c == '\n') {
      lines[lineCount % 50][currentPos] = '\0';
      lineCount++;
      currentPos = 0;
    } else if (currentPos < 255) {
      lines[lineCount % 50][currentPos++] = c;
    }
  }
  
  int startLine = (lineCount > 50) ? lineCount - 50 : 0;
  for (int i = 0; i < min(lineCount, 50); i++) {
    int idx = (startLine + i) % 50;
    server.sendContent(lines[idx]);
    server.sendContent("\n");
  }
  
  server.sendContent("</pre></body></html>");
  server.sendContent("");  // End chunked transfer
  file.close();
}

// Delete SoC decrease log file
void handleDeleteSoC() {
  if (!sdCardAvailable) {
    server.send(404, "text/plain", "SD card not available");
    return;
  }
  
  if (SD.remove(socDecreaseFilename)) {
    server.send(200, "text/html", "<html><body><h2>SoC decrease log file deleted</h2><a href='/'>Back</a></body></html>");
  } else {
    server.send(500, "text/plain", "Failed to delete file");
  }
}

void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

#endif // WEBSERVER_HANDLERS_H
