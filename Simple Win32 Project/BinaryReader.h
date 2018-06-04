#pragma once
#include <string>

typedef std::string String;

class BinaryReader
{
	public:

		BinaryReader(const char* Filename);
		bool IsOpen();
		void Close();

		char  ReadByte();
		int   ReadInt32();
		float ReadSingle();
		char* ReadBytes(int Count);

		std::string ReadString();

	private:
		int fp;

};