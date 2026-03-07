#include <fftw3.h>
#include <iostream>
#include <unistd.h>
#include <omp.h>
#include "rarray"
#include "stft_io.h"
#include <math.h>

#define THRESHOLD 1e-9
#define RELATIVE_THRESHOLD 1e-10

int verify_fftw_omp(rarray<std::complex<double>, 1> signal,
                    rarray<std::complex<double>, 2> stft,
                    unsigned long window_size)
{
  int result = 0;

  fftw_init_threads();
  fftw_plan_with_nthreads(omp_get_max_threads());

  fftw_complex *in, *out;
  in = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * window_size);
  out = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * window_size);

  for (int i = 0; i < (signal.size() - window_size) + 1; i++)
  {
    for (int j = 0; j < window_size; j++)
    {
      in[j][0] = signal[j + i].real();
      in[j][1] = signal[j + i].imag();
    }

    fftw_plan p = fftw_plan_dft_1d(window_size, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(p);
    fftw_destroy_plan(p);

    // for (int j = 0; j < window_size; j++)
    // {
    //   fprintf(stdout, "(%f, %f) ", out[j][0], out[j][1]);
    // }
    // fprintf(stdout, "\n");

    if (i >= stft.shape()[0] || window_size != stft.shape()[1])
    {
      fprintf(stderr, "Incorrect dimensions.\n");
      result = 1;
    }
    if (result == 1) break;
    for (int j = 0; j < stft.shape()[1]; j++)
    {
      if (out[j][0] > THRESHOLD && fabs(stft[i][j].real() - out[j][0]) > THRESHOLD)
      {
        fprintf(
          stderr, 
          "Incorrect real part at (%d, %d) (absolute error): expected %f but got %f.\n",
          i, j, out[j][0], stft[i][j].real());
        result = 1;
      }
      else if (out[j][0] != 0 && fabs(stft[i][j].real() - out[j][0]) / out[j][0] > RELATIVE_THRESHOLD)
      {
        fprintf(
          stderr, 
          "Incorrect real part at (%d, %d) (relative error): expected %f but got %f.\n",
          i, j, out[j][0], stft[i][j].real());
        result = 1;
      }

      if (out[j][1] <= THRESHOLD && fabs(stft[i][j].imag() - out[j][1]) > THRESHOLD)
      {
        fprintf(
          stderr, 
          "Incorrect imaginary part at (%d, %d) (absolute error): expected %f but got %f.\n",
          i, j, out[j][1], stft[i][j].imag());
        result = 1;
      }
      else if (out[j][1] > THRESHOLD && fabs(stft[i][j].imag() - out[j][1]) / out[j][1] > RELATIVE_THRESHOLD)
      {
        fprintf(
          stderr,
          "Incorrect real part at (%d, %d) (relative error): expected %f but got %f.\n",
          i, j, out[j][1], stft[i][j].imag());
        result = 1;
      }
    }
    if (result == 1) break;
  }

  fftw_free(in);
  fftw_free(out);

  fftw_cleanup_threads();

  if (result == 0)
    fprintf(stdout, "Validation succeeded.\n");
  else
    fprintf(stdout, "Validation failed.\n");
  return result;
}

int verify_stft(rarray<std::complex<double>, 1> signal,
                unsigned long window_size,
                const char *parallel_cstr,
                const char *output_cstr,
                auto output_shape)
{
  rarray<std::complex<double>, 2> stft(output_shape[0], output_shape[1]);
  if (stft_io::read_stft(output_cstr, stft) != 0)
  {
    fprintf(stderr, "Unable to read output file: %s.\n", output_cstr);
    return 1;
  }

  std::string parallel = parallel_cstr ? std::string(parallel_cstr) : "";
  if (parallel == "omp")
    return verify_fftw_omp(signal, stft, window_size);

  return 0;
}

int main(int argc, char *argv[])
{
  int helpFlag = 0;
  char *outputFile = NULL;
  char *inputFile = NULL;
  int c;

  opterr = 0;

  while ((c = getopt(argc, argv, "ho:f:")) != -1)
    switch (c)
    {
    case 'h':
      helpFlag = 1;
      break;
    case 'o':
      outputFile = optarg;
      break;
    case 'f':
      inputFile = optarg;
      break;
    case '?':
      if (optopt == 'o' || optopt == 'f')
        fprintf(stderr, "Option -%c requires an argument.\n", optopt);
      else if (isprint(optopt))
        fprintf(stderr, "Unknown option `-%c'.\n", optopt);
      else
        fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
      return 1;
    default:
      abort();
    }

  if (helpFlag)
  {
    fprintf(stdout,
            "Usage: %s [-h] [-s] [-a algorithm] [-p parallel] [-o output] [num-samples] [window-size]\n"
            "\n"
            "Arguments:\n"
            "  num-samples\tlong, size of input signal\n"
            "  window-size\tlong, less than num-samples, power of 2, size of STFT window\n"
            "\n"
            "Options:\n"
            "  -h\t\tshow this help message and exit\n"
            "  -o\t\toutput file: [/path/to/file]\n"
            "  -f\t\tinput file: [/path/to/file]\n",
            argv[0]);
    return 0;
  }

  if (optind + 2 != argc)
  {
    fprintf(stderr, "Invalid arguments. See -h for usage.\n");
    return 1;
  }

  unsigned long num_samples = 0;
  unsigned long window_size = 0;
  try
  {
    num_samples = std::stoul(*(argv + optind));
  }
  catch (std::invalid_argument const &arg)
  {
    fprintf(stderr, "Invalid arguments [num-samples]. See -h for usage.\n");
    return 1;
  }
  catch (std::out_of_range const &arg)
  {
    fprintf(stderr, "Out of range arguments [num-samples]. See -h for usage.\n");
    return 1;
  }
  try
  {
    window_size = std::stoul(*(argv + optind + 1));
  }
  catch (std::invalid_argument const &arg)
  {
    fprintf(stderr, "Invalid arguments [window-size]. See -h for usage.\n");
    return 1;
  }
  catch (std::out_of_range const &arg)
  {
    fprintf(stderr, "Out of range arguments [window-size]. See -h for usage.\n");
    return 1;
  }

  if (num_samples < window_size)
  {
    fprintf(stderr, "num-samples must be greater than window-size. See -h for usage.\n");
    return 1;
  }
  if (window_size == 0 || num_samples == 0)
  {
    fprintf(stderr, "num-samples and window-size must be positive. See -h for usage.\n");
    return 1;
  }
  if ((window_size & (window_size - 1)) != 0)
  {
    fprintf(stderr, "window-size must be power of 2. See -h for usage.\n");
    return 1;
  }

  rarray<std::complex<double>, 1> signal(num_samples);
  if (inputFile != NULL)
  {
    if (stft_io::read_signal(inputFile, signal) != 0)
    {
      fprintf(stderr, "Failed to read input file %s.\n", inputFile);
      return 1;
    }
  }
  else
    for (size_t i = 0; i < num_samples; i++)
      signal[i] = std::complex<double>(double(i), 0.0);

  rarray<std::complex<double>, 2> stft(num_samples - window_size + 1, window_size);
  if (stft_io::read_stft(outputFile, stft) != 0)
  {
    fprintf(stderr, "Failed to read output file %s.\n", outputFile);
    return 1;
  }

  return verify_fftw_omp(signal, stft, window_size);
}
