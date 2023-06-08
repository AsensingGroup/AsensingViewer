// Copyright 2013 Velodyne Acoustics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "vtkPacketFileWriter.h"

#include <cstring>

#ifdef WIN32
char *wchar2char(const wchar_t* source)
{
    char * data;
    int len= WideCharToMultiByte( CP_ACP ,0,source ,wcslen( source ), NULL,0, NULL ,NULL );
    data= new char[len+1];
    WideCharToMultiByte( CP_ACP ,0,source ,wcslen( source ),data,len, NULL ,NULL );
    data[len]= '\0';
    return data;
}

const char *stringToChar(const std::string& str)
{
    //定义一个空的std::wstring
    std::wstring wstr = L"";

    //lpWideCharStr设为NULL,cchWideChar设为0,该步骤用于获取缓冲区大小
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.size(), NULL, 0);

    //创建wchar_t数组作为缓冲区,用于接收转换出来的内容,数组长度为len+1的原因是字符串需要以'\0'结尾,所以+1用来存储'\0'
    wchar_t* wchar = new wchar_t[len + 1];

    //缓冲区和所需缓冲区大小都已确定,执行转换
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.size(), wchar, len);

    //使用'\0'结尾
    wchar[len] = '\0';

    //缓冲区拼接到std::wstring
    wstr.append(wchar);

    const char *data = wchar2char(wchar);
    //切记要清空缓冲区,否则内存泄漏
    delete[]wchar;
    return data;
}
#endif

//--------------------------------------------------------------------------------
vtkPacketFileWriter::vtkPacketFileWriter()
{
  this->PCAPFile = 0;
  this->PCAPDump = 0;
}

//--------------------------------------------------------------------------------
vtkPacketFileWriter::~vtkPacketFileWriter()
{
  this->Close();
}

//--------------------------------------------------------------------------------
bool vtkPacketFileWriter::Open(const std::string& filename)
{
  this->PCAPFile = pcap_open_dead(DLT_EN10MB, 65535);
#ifdef WIN32
  const char *path = stringToChar(filename);
  this->PCAPDump = pcap_dump_open(this->PCAPFile, path);
  delete [] path;
#else
  this->PCAPDump = pcap_dump_open(this->PCAPFile, filename.c_str());
#endif

  if (!this->PCAPDump)
  {
    this->LastError = pcap_geterr(this->PCAPFile);
    pcap_close(this->PCAPFile);
    this->PCAPFile = 0;
    return false;
  }

  this->FileName = filename;
  return true;
}

//--------------------------------------------------------------------------------
bool vtkPacketFileWriter::IsOpen()
{
  return (this->PCAPFile != 0);
}

void vtkPacketFileWriter::Close()
{
  if (this->PCAPFile)
  {
    pcap_dump_close(this->PCAPDump);
    pcap_close(this->PCAPFile);
    this->PCAPFile = 0;
    this->PCAPDump = 0;
    this->FileName.clear();
  }
}

//--------------------------------------------------------------------------------
const std::string& vtkPacketFileWriter::GetLastError()
{
  return this->LastError;
}

//--------------------------------------------------------------------------------
const std::string& vtkPacketFileWriter::GetFileName()
{
  return this->FileName;
}

//--------------------------------------------------------------------------------
// Write an UDP packet from the data (without providing a header, so we construct it)
bool vtkPacketFileWriter::WritePacket(const NetworkPacket& packet)
{
  if (!this->PCAPFile)
  {
    return false;
  }

  struct pcap_pkthdr header;
  header.caplen = packet.GetPacketSize();
  header.len = packet.GetPacketSize();
  header.ts = packet.ReceptionTime;

  pcap_dump((u_char*)this->PCAPDump, &header, packet.GetPacketData());
  return true;
}

//--------------------------------------------------------------------------------
// Write an packet from packetHeader and packetData (which includes the packet header)
bool vtkPacketFileWriter::WritePacket(pcap_pkthdr* packetHeader, const unsigned char* packetData)
{
  pcap_dump((u_char*)this->PCAPDump, packetHeader, packetData);
  return true;
}
