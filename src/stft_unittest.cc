#include "stft.h"
#include "gtest/gtest.h"
#include <complex>
#include <fftw3.h>
#include <iostream>

#define THRESHOLD 1e-10
#define RELATIVE_THRESHOLD 1e-14

enum class SignalType
{
  REAL,
  IMAGINARY,
  MIXED,
};

enum class Algorithm
{
  FFT,
  DFT,
  FF,
  QPFF
};

// Generate signal according to type.
rarray<std::complex<double>, 1> generate_signal(int num_samples, SignalType type)
{
  rarray<std::complex<double>, 1> signal(num_samples);
  for (int i = 0; i < num_samples; i++)
  {
    switch (type)
    {
    case SignalType::REAL:
      signal[i] = std::complex<double>(double(pow(2, i)), 0.0);
      break;
    case SignalType::IMAGINARY:
      signal[i] = std::complex<double>(0.0, double(pow(2, i)));
      break;
    case SignalType::MIXED:
      signal[i] = std::complex<double>(double(pow(2, i)), double(pow(2, i)));
      break;
    }
  }
  return signal;
}

// Compute expected results using FFTW and return in the same type for comparison.
rarray<std::complex<double>, 2> compute_expected_stft(
    const rarray<std::complex<double>, 1> &signal,
    int num_samples, int window_size)
{
  int frames = num_samples - window_size + 1;
  rarray<std::complex<double>, 2> expected(frames, window_size);

  fftw_complex *in, *out;
  fftw_plan p;
  in = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * window_size);
  out = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * window_size);
  p = fftw_plan_dft_1d(window_size, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

  for (int j = 0; j < frames; j++)
  {
    for (int i = 0; i < window_size; i++)
    {
      std::complex<double> val = signal[j + i];
      in[i][0] = val.real();
      in[i][1] = val.imag();
    }

    fftw_execute(p);

    for (int i = 0; i < window_size; i++)
      expected[j][i] = std::complex<double>(out[i][0], out[i][1]);
  }

  fftw_destroy_plan(p);
  fftw_free(in);
  fftw_free(out);

  return expected;
}

// Compute actual results.
rarray<std::complex<double>, 2> compute_actual_stft(
    rarray<std::complex<double>, 1> signal, int window_size,
    Algorithm a)
{
  switch (a)
  {
  case Algorithm::FFT:
    return stft::stft_fft(signal, window_size);
  case Algorithm::DFT:
    return stft::stft_dft(signal, window_size);
  case Algorithm::FF:
    return stft::stft_ff(signal, window_size);
  case Algorithm::QPFF:
    return stft::stft_qpff(signal, window_size);
  }
  return {};
}

class STFTTest : public ::testing::TestWithParam<std::tuple<SignalType, Algorithm>>
{
protected:
  const int num_samples = 16;
  const int window_size = 8;
};

TEST(FFTW, SameInput)
{
  const int num_samples = 256;
  const int window_size = 8;
  auto signal = generate_signal(num_samples, SignalType::MIXED);
  auto result1 = compute_expected_stft(signal, num_samples, window_size);
  auto result2 = compute_expected_stft(signal, num_samples, window_size);

  int expected_row_size = num_samples - window_size + 1;
  for (int j = 0; j < expected_row_size; j++)
  {
    for (int i = 0; i < window_size; i++)
    {
      EXPECT_DOUBLE_EQ(result1[j][i].real(), result2[j][i].real())
          << "Incorrect real part at (" << j << "," << i << ").";
      EXPECT_DOUBLE_EQ(result1[j][i].imag(), result2[j][i].imag())
          << "Incorrect imaginary part at (" << j << "," << i << ").";
    }
  }
}

TEST_P(STFTTest, MatchFFTW)
{
  auto [signal_type, method] = GetParam();

  // Calculate actual results.
  auto signal = generate_signal(num_samples, signal_type);
  auto result = compute_actual_stft(signal, window_size, method);

  // Check dimensions of results.
  int expected_row_size = num_samples - window_size + 1;
  EXPECT_EQ(result.shape()[0], expected_row_size) << "Incorrect row size.";
  EXPECT_EQ(result.shape()[1], window_size) << "Incorrect column size.";

  // Calculate expected results using FFTW.
  rarray<std::complex<double>, 2> expected = compute_expected_stft(signal, num_samples, window_size);

  // Check near equality with tolerance if the expected value is zero.
  // Otherwise, check relative error is within defined threshold.
  for (int j = 0; j < expected_row_size; j++)
  {
    for (int i = 0; i < window_size; i++)
    {
      if (expected[j][i].real() == 0)
      {
        EXPECT_NEAR(expected[j][i].real(), result[j][i].real(), THRESHOLD)
            << "Incorrect real part at (" << j << "," << i << ").";
      }
      else
      {
        EXPECT_LT(abs((expected[j][i].real() - result[j][i].real()) / expected[j][i].real()), RELATIVE_THRESHOLD)
            << "Incorrect real part at (" << j << "," << i << "): expected " << expected[j][i].real() << " but got " << result[j][i].real();
      }

      if (expected[j][i].imag() == 0)
      {
        EXPECT_NEAR(expected[j][i].imag(), result[j][i].imag(), THRESHOLD)
            << "Incorrect imaginary part at (" << j << "," << i << ").";
      }
      else
      {
        EXPECT_LT(abs((expected[j][i].imag() - result[j][i].imag()) / expected[j][i].imag()), RELATIVE_THRESHOLD)
            << "Incorrect imaginary part at (" << j << "," << i << "): expected " << expected[j][i].imag() << " but got " << result[j][i].imag();
      }
    }
  }
}

INSTANTIATE_TEST_SUITE_P(
    STFTVariants,
    STFTTest,
    ::testing::Combine(
        ::testing::Values(SignalType::REAL, SignalType::IMAGINARY, SignalType::MIXED),
        ::testing::Values(Algorithm::FFT, Algorithm::DFT, Algorithm::FF, Algorithm::QPFF)));
