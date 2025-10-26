#include "BaseWriter.h"

namespace BrightLib{namespace Media{

	StreamWriter::StreamWriter(char * file_name):out_file_ptr(nullptr)
	{
		out_file_ptr = fopen(file_name, "wb+");
	}

	int StreamWriter::write(uint8_t *buf, size_t len)
	{
		if (out_file_ptr)
		{
			fwrite(buf, 1, len, out_file_ptr);
			fflush(out_file_ptr);
		}
		//m_output.write((char*)buf, len);
		return 0;
	}

}}
