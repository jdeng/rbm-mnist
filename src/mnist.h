#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <exception>

#include <zlib.h>

struct Sample
{
	uint8_t label_;
	std::vector<int> data_;
};

namespace mnist
{
struct Exception: public std::exception
{
	std::string message_;
	Exception(const std::string& msg): message_(msg) {}
	~Exception() noexcept(true) {}
};

struct GzipFile
{
	gzFile fp_;

	GzipFile(const char *path, const char *mode)
	{
		fp_ = gzopen(path, mode);
		if (!fp_) throw Exception("failed to open file");
	}

	~GzipFile() { if (fp_) gzclose(fp_); }

	int32_t read_int() 
	{
		uint8_t buf[4];
		if (gzread(fp_, buf, sizeof buf) != sizeof(buf)) throw Exception("failed to read an integer");
		return int32_t(buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3]);
	}

	uint8_t read_byte()
	{
		uint8_t i;
		if (gzread(fp_, &i, sizeof(i)) != sizeof(i)) throw Exception("failed to read an byte");
		return i;
	}
};
	
int read(const std::string& images, const std::string& labels, std::vector<Sample>& samples)
{
	GzipFile gfi(images.c_str(), "rb"), gfl(labels.c_str(), "rb");

	int label_magic = gfl.read_int();
	int label_count = gfl.read_int();

	std::cout << "Labels: magic=" << label_magic << ", count=" << label_count << std::endl;

	int image_magic = gfi.read_int();
	int image_count = gfi.read_int();
	int image_rows = gfi.read_int();
	int image_cols = gfi.read_int();

	std::cout << "Images: magic=" << image_magic << ", count=" << image_count << ", rows=" << image_rows << ", cols=" << image_cols << std::endl;

	if (label_count != image_count) throw Exception("counts don't match");

	samples.resize(label_count);
	for (int i = 0; i < label_count; ++i)
	{
		Sample& sample = samples[i];
		sample.label_ = gfl.read_byte();

		sample.data_.resize(image_rows * image_cols);
		for (int j = 0; j < image_rows * image_cols; ++j)
			sample.data_[j] = int(gfi.read_byte());
	} 		

	std::cout << samples.size() << " samples read" << std::endl;
	return samples.size();
}

}


