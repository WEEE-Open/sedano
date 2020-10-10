#pragma once

int serialInitialize(char *path, int setSerial);
char *readBarcode();
int serialTerminate();