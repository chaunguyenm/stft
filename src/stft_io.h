#ifndef STFT_IO_H
#define STFT_IO_H

#include <complex>
#include "rarray"

namespace stft_io
{
  int read_signal(const std::string &filePath, rarray<std::complex<double>, 1> signal);
  int write_signal(const std::string &filePath, rarray<std::complex<double>, 1> signal);
  int read_stft(const std::string &filePath, rarray<std::complex<double>, 2> stft);
  int write_stft(const std::string &filePath, rarray<std::complex<double>, 2> stft);
}

#endif
