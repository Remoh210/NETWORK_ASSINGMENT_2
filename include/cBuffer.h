#ifndef _cBuffer_HG
#define _cBuffer_HG

#include <string>

class cBuffer {
public:
    cBuffer(unsigned int size);

    unsigned int bufferSize() { return this->m_buffer.size(); }
    void setBuffer(char* rcvBuffer, int bufferLenght);
    char* getBuffer();
    void clearBuffer();
    void WriteInt32LE(unsigned int index, int value);
    void WriteInt32LE(int value);
    void WriteInt16LE(unsigned int index, short value);
    void WriteInt16LE(short value);
    void WriteChar(unsigned int index, char value);
    void WriteChar(char value);
    int ReadInt32LE(unsigned int index);
    int ReadInt32LE();
    short ReadInt16LE(unsigned int index);
    short ReadInt16LE();
    char ReadChar(unsigned int index);
    char ReadChar();

private:
    std::string m_buffer;       
    char* m_retBuffer;          
    unsigned int m_readIndex;   
    unsigned int m_writeIndex;  
};

#endif
