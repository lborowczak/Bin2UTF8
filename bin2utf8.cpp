//`1234567890-=qwertyuiop[]\asdfghjkl;'zxcvbnm,./~!@#$%^&*()_+QWERTYUIOP{}|ASDFGHJKL:"ZXCVBNM<>? ????????????????????????????????????????????????????????aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
/*

bin2utf8: A program for converting binary files to UTF-8, inspired by and using some pseudocode from http://robbi-985.homeip.net/blog/?p=1845.

*/

#include "bin2utf8.hpp"

#include "libs/utf8/utf8.h"
#include <iostream>
#include <fstream>
#include <locale>
#include <codecvt>
#include <cstdint>
#include <sstream>

int BUFFERSIZE = 8192;

int main (int argc, char *argv[]) {
  std::string filename = "", coding="", BCLFile="";

  //Handle arguments
  if (argc < 2) {
    std::cout << "Program usage: " << argv[0] << " filename [decode bclfile]" << std::endl;
    return 0;
  }
  else {
    filename = argv[1];
  }
  if (argc > 3) {
    coding = argv[2];
    BCLFile = argv[3];
  }

  if (coding.compare("decode") != 0) {
    //encoding
    std::cout << "encoding!" << std::endl;
    return encodeFile(filename);
  }
  else {
    return decodeFile(filename, BCLFile);
  }
}

int encodeFile(std::string filename) {
  unsigned char BCLByte[255];
  uint32_t BCLCharCodes[255];
  int BCLHighestUsed = -1;

  //Open the file and files we will write to
  std::ifstream fileReader(filename, std::ios::binary);
  std::ofstream fileWriter(filename + ".txt"), BCLWriter(filename + ".bcl", std::ios::binary);

  //Check if the files are readable and writeable
  if (fileReader && fileWriter && BCLWriter) {
    //Generate array of character codes
    genBCLCharCodes(BCLCharCodes, &BCLHighestUsed);
    BCLHighestUsed = -1;

    char *fileBuffer = new char[BUFFERSIZE];
    fileReader.read(fileBuffer, BUFFERSIZE);

    //Output as UTF-8
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;

    //Read and write in chunks of size BUFFERSIZE
    while (fileReader) {

      std::stringstream outputString;

      for (int i = 0; i < BUFFERSIZE; i++) {
        outputString << converter.to_bytes(ByteToCharCode(fileBuffer[i], BCLCharCodes, &BCLHighestUsed, BCLByte));
      }
      fileWriter << outputString.str();

      fileReader.read(fileBuffer, BUFFERSIZE);
    }
    fileReader.close();

    //Write rest of bytes
    int bytesLeft = fileReader.gcount();
    if (bytesLeft > 0){
      for (int i = 0; i < bytesLeft; i++) {
        fileWriter << converter.to_bytes(ByteToCharCode(fileBuffer[i], BCLCharCodes, &BCLHighestUsed, BCLByte));
      }
    }
    fileWriter.close();
    delete[] fileBuffer;
    BCLWriter.write((char *)BCLByte, 256);
    return 0;
  }
  else {
    std::cout << "The file does not exist or is in a read-only directory or filesystem; exiting." << std::endl;
    fileReader.close();
    fileWriter.close();
    BCLWriter.close();
    return -1;
  }

}

void genBCLCharCodes(uint32_t *BCLCharCodes, int *BCLHighestUsed) {
  //Based on pseudocode provided at the link at the top of the file
  int loopIndex;
  //add normal characters
  for (loopIndex = 0x23; loopIndex <= 0x7E; loopIndex++) {
    (*BCLHighestUsed)++;
    BCLCharCodes[*BCLHighestUsed] = loopIndex;
  }

  loopIndex = 0;
  //Add UTF-8 characters (Chinese characters)
  while (*BCLHighestUsed < 255) {
    (*BCLHighestUsed)++;
    BCLCharCodes[*BCLHighestUsed] = (0x3900 + loopIndex);
    loopIndex++;
  }
}

uint32_t ByteToCharCode(unsigned char byteVal, uint32_t *BCLCharCodes, int *BCLHighestUsed, unsigned char *BCLByte) {
  //Based on pseudocode provided at the link at the top of the file
  //Search for matching byte in lookup table and return the character code.
  for (int checkIndex = 0; checkIndex <= *BCLHighestUsed; checkIndex++) {
    if (BCLByte[checkIndex] == byteVal) {
      return BCLCharCodes[checkIndex];
    }
  }

  //None existed - add new entry for this byte, using next available character code.
  if (*BCLHighestUsed < 255) {
    (*BCLHighestUsed)++;
    BCLByte[*BCLHighestUsed] = byteVal;
    return BCLCharCodes[*BCLHighestUsed];
  }
  else {
    //There must be a bug somewhere - all 256 lookup entries are already used.
  }
  return -1;
}

int decodeFile(std::string filename, std::string BCLFile) {
  unsigned char *BCLByte = new unsigned char[255];
  uint32_t BCLCharCodes[255];
  int BCLHighestUsed = -1;
  char *fullFile;

  std::ifstream fileReader(filename, std::ios::ate), BCLReader(BCLFile);
  std::ofstream fileWriter(filename + ".raw", std::ios::binary);

  if(fileReader && BCLReader && fileWriter){
    //Close BCLReader, only used to check if the file is readable.
    BCLReader.close();

    //Generate array of character codes
    genBCLCharCodes(BCLCharCodes, &BCLHighestUsed);
    BCLHighestUsed = -1;

    loadBCL(BCLByte, BCLFile);

    //Read in the whole file
    std::streampos fileSize = fileReader.tellg();
    fullFile = new char[fileSize];
    fileReader.seekg(0, std::ios::beg);
    fileReader.read(fullFile, fileSize);
    fileReader.close();

    std::stringstream outputString;

    //Set up iterator for UTF-8 library
    char *utf8iter = fullFile;

    uint32_t charCode;

    //While there is another utf-8 character...
    while(charCode = utf8::unchecked::next(utf8iter)) {
      char result = BCLCharCodeToByte(charCode, BCLCharCodes, BCLByte);
      fileWriter << result;
    }

    return 0;
  }
  else {
    std::cout << "The file does not exist or is in a read-only directory or filesystem; exiting." << std::endl;
    fileReader.close();
    BCLReader.close();
    fileWriter.close();
  }
}

void loadBCL(unsigned char* BCLByte, std::string filename) {
  std::ifstream fileReader;
  fileReader.open(filename, std::ios::binary);
  fileReader.read((char *)BCLByte, 256);
  fileReader.close();

}




char BCLCharCodeToByte(uint32_t charCodeVal, uint32_t *BCLCharCodes, unsigned char *BCLByte) {
  //Based on pseudocode provided at the link at the top of the file
  //TODO turn into map for faster lookup
  //Search for matching character code in lookup table and return the byte.
  for (int checkIndex = 0; checkIndex <= 255; checkIndex++) {
    if (BCLCharCodes[checkIndex] == charCodeVal) {
      //std::cout << "returning value from index " << checkIndex <<"; charCodeVal = " << charCodeVal <<"; BCLBytes is "<< BCLByte << std::endl;
      return BCLByte[checkIndex];
    }
  }

  //IMPORTANT: Torch-rnn will write CR+LF at the end of its output, making text files 2
  //bytes longer than you requested of it. As CR and LF are not characters in our
  //lookup table (BCLCharCode[]), we will not have returned a value yet! You can choose
  //your own default value to return in this case here:
  return 0;

  //Not included in this pseudocode: I decided to keep track of the total number of these
  //mismatches and display an error at the end of conversion according to how many failed
  //(i.e. nothing serious if only 2 failed).
}
