#ifndef BIN2UTF8_H
#define BIN2UTF8_H

#include <cstdint>
#include <string>

int encodeFile(std::string filename);

int decodeFile(std::string filename, std::string BCLFile);

void genBCLCharCodes(uint32_t *BCLCharCodes, int *BCLHighestUsed);

uint32_t ByteToCharCode(unsigned char byteVal, uint32_t *BCLCharCodes, int *BCLHighestUsed, unsigned char *BCLByte);

char BCLCharCodeToByte(uint32_t charCodeVal, uint32_t *BCLCharCodes, unsigned char *BCLByte);

void loadBCL(unsigned char* BCLByte, std::string filename);

#endif
