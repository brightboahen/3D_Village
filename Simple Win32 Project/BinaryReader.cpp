#include "stdafx.h"

#include "BinaryReader.h"

#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <string>

BinaryReader::BinaryReader(const char* Filename)
{
	fp = _open(Filename, O_BINARY | O_RDONLY);
}

bool BinaryReader::IsOpen()
{
	return fp !=-1;
}

void BinaryReader::Close()
{
	_close(fp);
	fp = -1;
}

char BinaryReader::ReadByte()
{
	char c;
	_read(fp,&c,1);
	return c;
}
int BinaryReader::ReadInt32()
{
	int c;
	_read(fp,&c,4);
	return c;
}
float BinaryReader::ReadSingle()
{
	float c;
	_read(fp,&c,4);
	return c;
}
char* BinaryReader::ReadBytes(int Count)
{
	char* c = new char[Count+1];
	memset(c,0,Count+1);
	_read(fp,c,Count);
	return c;
}

std::string BinaryReader::ReadString()
{
	int len = (int)ReadByte();

	char *c = ReadBytes(len);

	std::string s;

	s.assign(c);

	delete[] c;

	return s;
}
