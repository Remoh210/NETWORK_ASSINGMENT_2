#include "cBuffer.h"
#define MAX_BUFFER_SIZE 1500
#include <iostream>
#include <algorithm>
#define GROW 100

cBuffer::cBuffer(unsigned int size)
    : m_writeIndex(0)
    , m_readIndex(0)
{
    m_buffer.resize(size);
}

void cBuffer::setBuffer(char* rcvBuffer, int bufferLenght)
{
    m_buffer.resize(bufferLenght);
    for(int i = 0; i < bufferLenght; i++)
        m_buffer[i] = rcvBuffer[i];

    m_readIndex = 0;
    m_writeIndex = 0;
}

char* cBuffer::getBuffer()
{
    m_retBuffer = new char[m_buffer.size() + 1];
    for(int i = 0; i < m_buffer.size(); i++)
        m_retBuffer[i] = m_buffer.at(i);

    m_retBuffer[m_buffer.size()] = '\0';
    return m_retBuffer;
}

void cBuffer::clearBuffer()
{
    m_buffer = "";
    m_readIndex = 0;
    m_writeIndex = 0;
}

void cBuffer::WriteInt32LE(unsigned int index, int value)
{
  
    if(index >= m_buffer.size() - 4) 
        m_buffer.resize(index + GROW);

    char firstByteBE = (value & 0x000000FF) >> 0;
    char secondByteBE = (value & 0x0000FF00) >> 8;
    char thirdByteBE = (value & 0x00FF0000) >> 16;
    char fourthByteBE = (value & 0xFF000000) >> 24;

    m_buffer[index + 0] = firstByteBE;
    m_buffer[index + 1] = secondByteBE;
    m_buffer[index + 2] = thirdByteBE;
    m_buffer[index + 3] = fourthByteBE;
}

void cBuffer::WriteInt32LE(int value)
{

    if(m_writeIndex >= m_buffer.size() - 4) 
        m_buffer.resize(m_writeIndex + GROW);

    char firstByteBE = (value & 0x000000FF) >> 0;
    char secondByteBE = (value & 0x0000FF00) >> 8;
    char thirdByteBE = (value & 0x00FF0000) >> 16;
    char fourthByteBE = (value & 0xFF000000) >> 24;

    m_buffer[m_writeIndex + 0] = firstByteBE;
    m_buffer[m_writeIndex + 1] = secondByteBE;
    m_buffer[m_writeIndex + 2] = thirdByteBE;
    m_buffer[m_writeIndex + 3] = fourthByteBE;

    m_writeIndex += 4;
}

void cBuffer::WriteInt16LE(unsigned int index, short value)
{
    if(index >= m_buffer.size() - 2) 
        m_buffer.resize(index + GROW);

    char firstByteBE = (value & 0x00FF) >> 0;
    char secondByteBE = (value & 0xFF00) >> 8;

    m_buffer[index + 0] = firstByteBE;
    m_buffer[index + 1] = secondByteBE;
}

void cBuffer::WriteInt16LE(short value)
{
    if(m_writeIndex >= m_buffer.size() - 2) 
        m_buffer.resize(m_writeIndex + GROW);

    char firstByteBE = (value & 0x00FF) >> 0;
    char secondByteBE = (value & 0xFF00) >> 8;

    m_buffer[m_writeIndex + 0] = firstByteBE;
    m_buffer[m_writeIndex + 1] = secondByteBE;

    m_writeIndex += 2;
}

void cBuffer::WriteChar(unsigned int index, char value)
{
   
    if(index == m_buffer.size()) 
        m_buffer.resize(index + GROW);

    m_buffer[index] = value;
}

void cBuffer::WriteChar(char value)
{
    if(m_writeIndex == m_buffer.size()) 
        m_buffer.resize(m_writeIndex + GROW);

    m_buffer[m_writeIndex] = value;

    m_writeIndex += 1;
}

int cBuffer::ReadInt32LE(unsigned int index)
{
    int fourthByteBE = (int)m_buffer[index + 0];
    int thirdByteBE = (int)m_buffer[index + 1];
    int secondByteBE = (int)m_buffer[index + 2];
    int firstByteBE = (int)m_buffer[index + 3];
 
    return (firstByteBE << 24) | (secondByteBE << 16) | (thirdByteBE << 8) | (fourthByteBE << 0);
}

int cBuffer::ReadInt32LE()
{
 
    int fourthByteBE = (int)m_buffer[m_readIndex + 0];
    int thirdByteBE = (int)m_buffer[m_readIndex + 1];
    int secondByteBE = (int)m_buffer[m_readIndex + 2];
    int firstByteBE = (int)m_buffer[m_readIndex + 3];
    m_readIndex += 4;

    return (firstByteBE << 24) | (secondByteBE << 16) | (thirdByteBE << 8) | (fourthByteBE << 0);
}

short cBuffer::ReadInt16LE(unsigned int index)
{

    short secondByteBE = (short)m_buffer[index + 0];
    short firstByteBE = (short)m_buffer[index + 1];
    return (firstByteBE << 8) | (secondByteBE << 0);
}

short cBuffer::ReadInt16LE()
{
 
    short secondByteBE = (short)m_buffer[m_readIndex + 0];
    short firstByteBE = (short)m_buffer[m_readIndex + 1];
    m_readIndex += 2;

    return (firstByteBE << 8) | (secondByteBE << 0);
}

char cBuffer::ReadChar(unsigned int index)
{
    return (char)m_buffer[index];
}

char cBuffer::ReadChar()
{
    m_readIndex += 1;

    return (char)m_buffer[(m_readIndex) - 1];
}