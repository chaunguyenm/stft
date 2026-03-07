#include "stft.h"
#include "rarray"
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <map>
#include <string>
#include <functional>
#include <iostream>
#include <cstring>
#include "stft_io.h"

#ifdef WITH_MPI
#include <mpi.h>
#endif

rarray<std::complex<double>, 2> compute_stft(
    rarray<std::complex<double>, 1> signal,
    unsigned long window_size,
    const char *algorithm_cstr,
    const char *parallel_cstr)
{
  using namespace std;
  using STFTFunc = function<rarray<complex<double>, 2>(rarray<complex<double>, 1>, size_t)>;

  string algorithm = algorithm_cstr ? string(algorithm_cstr) : "";
  string parallel = parallel_cstr ? string(parallel_cstr) : "";

  auto normalize = [](string s)
  {
    for (auto &c : s)
      c = tolower(c);
    return s;
  };
  algorithm = normalize(algorithm);
  parallel = normalize(parallel);

  // key = (algorithm, parallel)
  const map<pair<string, string>, STFTFunc> stft_map = {
      {{"dft", "omp"}, stft::stft_dft},
      {{"fft", "omp"}, stft::stft_fft},
      {{"qpff", "omp"}, stft::stft_qpff},

#ifdef WITH_MPI
      {{"dft", "mpi"}, stft_mpi::stft_dft},
      {{"fft", "mpi"}, stft_mpi::stft_fft},
      {{"qpff", "mpi"}, stft_mpi::stft_qpff},

      {{"dft", "hybrid"}, stft_mpi::stft_dft},
      {{"fft", "hybrid"}, stft_mpi::stft_fft},
      {{"qpff", "hybrid"}, stft_mpi::stft_qpff},
#endif

      {{"ff", ""}, stft::stft_ff}, // no parallel options
  };

  rarray<complex<double>, 2> stft;

  auto v = stft_map.find({algorithm, parallel});
  if (v == stft_map.end() && parallel.empty())
  {
    v = stft_map.find({algorithm, ""});
  }

  if (v != stft_map.end())
  {
    rarray<complex<double>, 1> sig_copy = signal;
    return v->second(sig_copy, window_size);
  }

  fprintf(stderr, "Unrecognized algorithm or parallel scheme. See -h for usage.\n");
  return stft;
}

int main(int argc, char *argv[])
{
  int helpFlag = 0;
  int silentFlag = 0;
  char *algorithm = NULL;
  char *parallel = NULL;
  char *outputFile = NULL;
  char *inputFile = NULL;
  int c;

  opterr = 0;

  while ((c = getopt(argc, argv, "hsa:o:p:f:")) != -1)
    switch (c)
    {
    case 'h':
      helpFlag = 1;
      break;
    case 's':
      silentFlag = 1;
      break;
    case 'a':
      algorithm = optarg;
      break;
    case 'o':
      outputFile = optarg;
      break;
    case 'p':
      parallel = optarg;
      break;
    case 'f':
      inputFile = optarg;
      break;
    case '?':
      if (optopt == 'a' || optopt == 'o' || optopt == 'p' || optopt == 'f')
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
            "  -s\t\tsilent (do not print any output)\n"
            "  -a\t\tSTFT algorithm: dft, fft (default), ff\n"
            "  -p\t\tparallel scheme: omp (default), mpi (must be run with mpirun)\n"
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
  {
    for (size_t i = 0; i < num_samples; i++)
      signal[i] = std::complex<double>(double(i), 0.0);
  }
  // std::cout << signal << "\n";

  int rank, size, provided;
  if (parallel != NULL && strncmp(parallel, "mpi", strlen("mpi")) == 0)
  {
#ifdef WITH_MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
#else
    fprintf(stderr, "MPI support not compiled. Recompile with -DWITH_MPI.\n");
    return 1;
#endif
  }
  else if (parallel != NULL && strncmp(parallel, "hybrid", strlen("hybrid")) == 0)
  {
#ifdef WITH_MPI
    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
#else
    fprintf(stderr, "MPI support not compiled. Recompile with -DWITH_MPI.\n");
    return 1;
#endif
  }
  else
  {
    rank = 0;
    size = 0;
    provided = 0;
  }

  rarray<std::complex<double>, 2> stft;
  stft = compute_stft(signal, window_size, (const char *)algorithm, (const char *)parallel);

  if (rank == 0)
  {
    if (outputFile != NULL)
      stft_io::write_stft(outputFile, stft);
    else if (silentFlag == 0)
      std::cout << stft << "\n";
  }

  if (parallel != NULL && strncmp(parallel, "mpi", strlen("mpi")) == 0)
  {
#ifdef WITH_MPI
    MPI_Finalize();
#endif
  }
  else if (parallel != NULL && strncmp(parallel, "hybrid", strlen("hybrid")) == 0)
  {
#ifdef WITH_MPI
    MPI_Finalize();
#endif
  }

  return 0;
}
