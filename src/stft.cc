#include "stft.h"
#include <fstream>
#include <iostream>
#ifdef WITH_MPI
#include <mpi.h>
#endif

#define PI 3.14159265358979323846

rarray<std::complex<double>, 2> stft::stft_dft(
    const rarray<std::complex<double>, 1> vec, const size_t window_size)
{
  const size_t spectrogram_size = vec.size() - window_size + 1;
  rarray<std::complex<double>, 2> spectrogram(spectrogram_size, window_size);

#pragma omp parallel for default(none) shared(vec, window_size, spectrogram_size, spectrogram)
  for (size_t i = 0; i < spectrogram_size; i++)
  {
    rarray<std::complex<double>, 1> dft = stft::dft(vec, i, window_size);

    for (size_t j = 0; j < window_size; j++)
      spectrogram[i][j] = dft[j];
  }
  return spectrogram;
}

rarray<std::complex<double>, 1> stft::dft(
    const rarray<std::complex<double>, 1> signal,
    size_t vec_begin, size_t vec_size)
{
  rarray<std::complex<double>, 1> output(vec_size);
#pragma omp parallel for default(none) shared(signal, vec_begin, vec_size, output)
  for (size_t k = 0; k < vec_size; k++)
  {
    std::complex<double> sum = 0.;
    for (size_t t = 0; t < vec_size; t++)
    {
      double angle = 2 * PI * t * k / vec_size;
      sum += signal[t + vec_begin] * std::exp(std::complex<double>(0, -angle));
    }
    output[k] = sum;
  }
  return output;
}

rarray<std::complex<double>, 2> stft::stft_fft(
    const rarray<std::complex<double>, 1> vec, const size_t window_size)
{
  const size_t spectrogram_size = vec.size() - window_size + 1;
  rarray<std::complex<double>, 2> spectrogram(spectrogram_size, window_size);

#pragma omp parallel for default(none) shared(vec, window_size, spectrogram_size, spectrogram)
  for (size_t i = 0; i < spectrogram_size; i++)
  {
    rarray<std::complex<double>, 1> fft = stft::fft(vec, i, window_size);

    for (size_t j = 0; j < window_size; j++)
      spectrogram[i][j] = fft[j];
  }
  return spectrogram;
}

rarray<std::complex<double>, 2> stft::stft_ff(
    const rarray<std::complex<double>, 1> vec, const size_t window_size)
{
  size_t num_stages = log2(window_size) + 1;
  size_t spectrogram_size = vec.size() - window_size + 1;
  rarray<std::complex<double>, 2> fft(window_size, num_stages),
      spectrogram(spectrogram_size, window_size);
  rarray<std::complex<double>, 2> tw(window_size, num_stages - 1);
  rarray<std::complex<double>, 2> buffer(window_size / 2, num_stages);

  // Perform FFT on 1st N signals
  rarray<std::complex<double>, 1> subvec(window_size);
  for (size_t i = 0; i < window_size; i++)
    subvec[i] = vec[i];
  transform(subvec, fft, tw);

  for (size_t i = 0; i < window_size; i++)
    spectrogram[0][i] = fft[i][num_stages - 1];

  // Prepare buffer
  for (size_t stage = 0; stage < num_stages; stage++)
  {
    size_t count = 1 << stage;
    for (size_t r = 0; (2 * r + 1) * count < window_size; r++)
    {
      size_t buffer_idx = reverse_bits(r, log2(window_size / 2));
      for (size_t c = 0; c < count; c++)
        buffer[buffer_idx + c][stage] = fft[(2 * r + 1) * count + c][stage];
    }
  }

  // Compute the remaining FFTs
  for (size_t i = 1; i + window_size - 1 < vec.size(); i++)
  {
    for (size_t s = 1; s < num_stages; s++)
    {
      size_t count = 1 << (s - 1);
      size_t m = ((i - 1) % (window_size / (1 << s))) * count;
      for (size_t k = 0; k < count; k++)
      {
        std::complex<double> xr;
        if (s == 1)
        {
          xr = vec[i + window_size - 1];
          fft[window_size - (1 << (s - 1)) + k][s - 1] = xr;
        }
        else
          xr = fft[window_size - (1 << (s - 1)) + k][s - 1];
        fft[window_size - (1 << s) + k][s] =
            buffer[m + k][s - 1] + tw[window_size - (1 << s) + k][s - 1] * xr;
        fft[window_size - (1 << s) + k + count][s] =
            buffer[m + k][s - 1] - tw[window_size - (1 << s) + k + count][s - 1] * xr;
      }
      // Update buffer
      for (size_t k = 0; k < count; k++)
        buffer[m + k][s - 1] = fft[window_size - (1 << (s - 1)) + k][s - 1];
    }
    // Write to output variable after completing all stages
    for (size_t j = 0; j < window_size; j++)
      spectrogram[i][j] = fft[j][num_stages - 1];
  }

  return spectrogram;
}

void stft::stft_qpff_parallel(
    const rarray<rarray<std::complex<double>, 1>, 2> B,
    const rarray<std::complex<double>, 2> tw,
    const size_t window_size, const size_t s,
    rarray<rarray<std::complex<double>, 1>, 1> F)
{
  const size_t count = (1 << s);

#pragma omp parallel for default(none) shared(B, tw, window_size, s, F, count)
  for (size_t i = 0; i < window_size / 2; i++)
  {
    rarray<std::complex<double>, 1> f(1 << (s + 1));

    for (size_t k = 0; k < count; k++)
    {
      std::complex<double> xr, x0, x1;
      xr = B[i][0][k] * tw[window_size - (1 << (s + 1)) + k][s];
      x0 = B[i][1][k] + xr;
      x1 = B[i][1][k] - xr;

      f[k] = x0;
      f[k + count] = x1;
    }

    F[i] = f;
  }
}

rarray<std::complex<double>, 2> stft::stft_qpff(
    const rarray<std::complex<double>, 1> vec, const size_t window_size)
{
  size_t num_stages = log2(window_size);
  size_t spectrogram_size = vec.size() - window_size + 1;
  rarray<std::complex<double>, 2> fft(window_size, num_stages + 1),
      spectrogram(spectrogram_size, window_size);
  rarray<std::complex<double>, 2> tw(window_size, num_stages);

  rarray<rarray<std::complex<double>, 1>, 2>
      M(window_size / 2, num_stages),
      M_prime(window_size / 2, num_stages);
  for (int r = 0; r < window_size / 2; r++)
  {
    for (int c = 0; c < num_stages; c++)
    {
      M[r][c] = rarray<std::complex<double>, 1>(1 << c);
      M_prime[r][c] = rarray<std::complex<double>, 1>(1 << c);
    }
  }

  rarray<rarray<std::complex<double>, 1>, 2> B(window_size / 2, 2);
  rarray<rarray<std::complex<double>, 1>, 1> prev(window_size / 2);
  rarray<rarray<std::complex<double>, 1>, 1> F(window_size / 2);

  // Perform FFT on 1st N signals
  rarray<std::complex<double>, 1> subvec(window_size);
  for (size_t i = 0; i < window_size; i++)
    subvec[i] = vec[i];
  transform(subvec, fft, tw);

  for (size_t i = 0; i < window_size; i++)
    spectrogram[0][i] = fft[i][num_stages];

  // Initialize M
  for (size_t s = 0; s < num_stages; s++)
  {
    size_t num_rows = window_size / (1 << (s + 1));
    for (size_t k = 0; k < num_rows; k++)
    {
      size_t row_idx = reverse_bits(k, log2(num_rows));
      for (size_t m = 0; m < (1 << s); m++)
        M[row_idx][s][m] = fft[(2 * k + 1) * (1 << s) + m][s];
    }
  }

  int n = 1;
  // Loop over all remaining signals, step size window_size/2
  while ((n + window_size / 2 - 1) + window_size - 1 < vec.size())
  {
    for (int s = 0; s < num_stages; s++)
    {
      // Prepare B
      for (int i = 0; i < window_size / 2; i++)
      {
        if (s == 0)
        {
          M_prime[i][s][0] = vec[i + n + window_size - 1];
          prev[i] = M_prime[i][s];
        }
        B[i][0] = prev[i];
        B[i][1] = M[i][s];
      }

      // Compute stage s of nth FFT using B
      stft::stft_qpff_parallel(B, tw, window_size, s, F);
      prev = F;

      // Update M and M' if not the last stage
      int idx = window_size / (1 << (s + 2));
      if (s < num_stages - 1)
      {
        for (int r = 0; r < idx; r++)
          M_prime[r][s + 1] = F[window_size / 2 - idx + r];
        for (int r = 0; r < window_size / 2 - idx; r++)
          M[idx + r][s + 1] = F[r];
      }
    }

    // Copy last stage of nth FFT to result
    for (int i = 0; i < window_size / 2; i++)
    {
      for (size_t j = 0; j < window_size; j++)
        spectrogram[n + i][j] = F[i][j];
    }

    // Swap buffers and move to next batch
    rarray<rarray<std::complex<double>, 1>, 2> temp = M;
    M = M_prime;
    M_prime = temp;
    n += window_size / 2;
  }

  // Compute the remaining (less than size of a batch) FFTs
  while (n < vec.size() - window_size + 1)
  {
    for (int i = 0; i < window_size; i++)
      spectrogram[n][i] = vec[n + i];
    transform(spectrogram[n]);
    n++;
  }

  return spectrogram;
}

rarray<std::complex<double>, 1> stft::fft(
    const rarray<std::complex<double>, 1> signal,
    size_t vec_begin, size_t vec_size)
{
  rarray<std::complex<double>, 1> subsignal(vec_size);
  for (size_t i = 0; i < vec_size; i++)
    subsignal[i] = signal[vec_begin + i];

  stft::transform(subsignal);
  return subsignal;
}

bool stft::transform(rarray<std::complex<double>, 1> vec)
{
  // Length variables
  size_t n = vec.size();
  int levels = 0; // Compute levels = floor(log2(n))
  for (size_t temp = n; temp > 1U; temp >>= 1)
    levels++;
  if ((size_t)1U << levels != n)
    return false; // n is not a power of 2

  // Trigonometric tables
  rarray<std::complex<double>, 1> exptable(n / 2);
  for (size_t i = 0; i < n / 2; i++)
    exptable[i] = std::polar(1.0, -2. * PI * i / n);

  // Bit-reversed addressing permutation
  for (size_t i = 0; i < n; i++)
  {
    size_t j = stft::reverse_bits(i, levels);
    if (j > i)
    {
      std::complex<double> temp = vec[i];
      vec[i] = vec[j];
      vec[j] = temp;
    }
  }

  // Cooley-Tukey decimation-in-time radix-2 FFT
  for (size_t size = 2; size <= n; size *= 2)
  {
    size_t halfsize = size / 2;
    size_t tablestep = n / size;
    for (size_t i = 0; i < n; i += size)
    {
      for (size_t j = i, k = 0; j < i + halfsize; j++, k += tablestep)
      {
        size_t l = j + halfsize;
        std::complex<double> temp = vec[l] * exptable[k];
        vec[l] = vec[j] - temp;
        vec[j] += temp;
      }
    }
    if (size == n) // Prevent overflow in 'size *= 2'
      break;
  }

  return true;
}

bool stft::transform(rarray<std::complex<double>, 1> vec,
                     rarray<std::complex<double>, 2> fft,
                     rarray<std::complex<double>, 2> tw)
{
  // Length variables
  size_t n = vec.size();
  int levels = 0; // Compute levels = floor(log2(n))
  for (size_t temp = n; temp > 1U; temp >>= 1)
    levels++;
  if ((size_t)1U << levels != n)
    return false; // n is not a power of 2

  // Trigonometric tables
  rarray<std::complex<double>, 1> exptable(n / 2);
  for (size_t i = 0; i < n / 2; i++)
    exptable[i] = std::polar(1.0, -2. * PI * i / n);

  // Bit-reversed addressing permutation
  for (size_t i = 0; i < n; i++)
  {
    size_t j = stft::reverse_bits(i, levels);
    if (j > i)
    {
      std::complex<double> temp = vec[i];
      vec[i] = vec[j];
      vec[j] = temp;
    }
  }
  for (size_t i = 0; i < n; i++)
    fft[i][0] = vec[i];

  // Cooley-Tukey decimation-in-time radix-2 FFT
  for (size_t size = 2; size <= n; size *= 2)
  {
    size_t halfsize = size / 2;
    size_t tablestep = n / size;
    for (size_t i = 0; i < n; i += size)
    {
      for (size_t j = i, k = 0; j < i + halfsize; j++, k += tablestep)
      {
        size_t l = j + halfsize;
        std::complex<double> temp = vec[l] * exptable[k];
        vec[l] = vec[j] - temp;
        vec[j] += temp;
        fft[j][log2(size)] = vec[j];
        fft[l][log2(size)] = vec[l];
        tw[j][log2(size) - 1] = exptable[k];
        tw[l][log2(size) - 1] = exptable[k];
      }
    }
    if (size == n) // Prevent overflow in 'size *= 2'
      break;
  }

  return true;
}

size_t stft::reverse_bits(size_t val, int width)
{
  size_t result = 0;
  for (int i = 0; i < width; i++, val >>= 1)
    result = (result << 1) | (val & 1U);
  return result;
}

#ifdef WITH_MPI
rarray<std::complex<double>, 2> stft_mpi::stft_dft(
    const rarray<std::complex<double>, 1> vec, const size_t window_size)
{
  size_t spectrogram_size = vec.size() - window_size + 1;
  rarray<std::complex<double>, 2> spectrogram(spectrogram_size, window_size);
  rarray<std::complex<double>, 1> dft;

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  size_t chunksize = spectrogram_size / size;
  size_t start_idx = rank * chunksize;
  size_t end_idx = (rank == size - 1) ? spectrogram_size : (rank + 1) * chunksize;

#pragma omp parallel for default(none) shared(vec, window_size, start_idx, end_idx, dft, spectrogram)
  for (size_t i = start_idx; i < end_idx; i++)
  {
    dft = stft::dft(vec, i, window_size);
    for (size_t j = 0; j < window_size; j++)
      spectrogram[i][j] = dft[j];
  }

  if (rank == 0)
    for (int p = 1; p < size; p++)
    {
      size_t recv_chunksize = (p == size - 1) ? spectrogram_size - p * chunksize : chunksize;
      MPI_Recv(&spectrogram[p * chunksize][0],
               recv_chunksize * window_size, MPI_DOUBLE_COMPLEX, p, 0,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
  else
    MPI_Send(&spectrogram[start_idx][0],
             (end_idx - start_idx) * window_size, MPI_DOUBLE_COMPLEX, 0, 0,
             MPI_COMM_WORLD);

  return spectrogram;
}

rarray<std::complex<double>, 2> stft_mpi::stft_fft(
    const rarray<std::complex<double>, 1> vec, const size_t window_size)
{
  size_t spectrogram_size = vec.size() - window_size + 1;
  rarray<std::complex<double>, 2> spectrogram(spectrogram_size, window_size);

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  size_t chunksize = spectrogram_size / size;
  size_t remaining = spectrogram_size % size;
  size_t start_idx = rank * chunksize + std::min((size_t)rank, remaining);
  size_t local_chunksize = chunksize + (rank < remaining);
  size_t end_idx = start_idx + local_chunksize;
  // size_t start_idx = rank * chunksize;
  // size_t end_idx = (rank == size - 1) ? spectrogram_size : (rank + 1) * chunksize;

#pragma omp parallel for default(none) shared(vec, window_size, start_idx, end_idx, spectrogram)
  for (size_t i = start_idx; i < end_idx; i++)
  {
    rarray<std::complex<double>, 1> fft = stft::fft(vec, i, window_size);
    for (size_t j = 0; j < window_size; j++)
      spectrogram[i][j] = fft[j];
  }

  if (rank == 0)
    for (int p = 1; p < size; p++)
    {
      // size_t recv_chunksize = (p == size - 1) ? spectrogram_size - p * chunksize : chunksize;
      // MPI_Recv(&spectrogram[p * chunksize][0], recv_chunksize * window_size,
      //          MPI_DOUBLE_COMPLEX, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      size_t recv_start_idx = p * chunksize + std::min((size_t)p, remaining);
      size_t recv_chunksize = chunksize + (p < remaining);
      size_t recv_end_idx = recv_start_idx + recv_chunksize;

      for (size_t i = recv_start_idx; i < recv_end_idx; ++i)
      {
        MPI_Recv(&spectrogram[i][0],  window_size, MPI_DOUBLE_COMPLEX, p, i,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      }
    }
  else
    // MPI_Send(&spectrogram[start_idx][0], (end_idx - start_idx) * window_size,
    //          MPI_DOUBLE_COMPLEX, 0, 0, MPI_COMM_WORLD);
    for (size_t i = start_idx; i < end_idx; ++i)
    {
      MPI_Send(&spectrogram[i][0], window_size, MPI_DOUBLE_COMPLEX, 0, i,
               MPI_COMM_WORLD);
    }

  return spectrogram;
}

rarray<std::complex<double>, 2> stft_mpi::stft_qpff(
    const rarray<std::complex<double>, 1> vec, const size_t window_size)
{
  size_t num_stages = log2(window_size);
  size_t spectrogram_size = vec.size() - window_size + 1;
  rarray<std::complex<double>, 2> fft(window_size, num_stages + 1),
      spectrogram(spectrogram_size, window_size);
  rarray<std::complex<double>, 2> tw(window_size, num_stages);

  rarray<rarray<std::complex<double>, 1>, 2>
      M(window_size / 2, num_stages),
      M_prime(window_size / 2, num_stages);
#pragma omp paralle for default(none) shared(M, M_prime, window_size, num_stages)
  for (int r = 0; r < window_size / 2; r++)
  {
    for (int c = 0; c < num_stages; c++)
    {
      M[r][c] = rarray<std::complex<double>, 1>(1 << c);
      M_prime[r][c] = rarray<std::complex<double>, 1>(1 << c);
    }
  }

  rarray<rarray<std::complex<double>, 1>, 2> B(window_size / 2, 2);
  rarray<rarray<std::complex<double>, 1>, 1> prev(window_size / 2);
  rarray<rarray<std::complex<double>, 1>, 1> F(window_size / 2);

  // Perform FFT on 1st N signals
  rarray<std::complex<double>, 1> subvec(window_size);
#pragma omp parallel for default(none) shared(subvec, vec, window_size)
  for (size_t i = 0; i < window_size; i++)
    subvec[i] = vec[i];
  stft::transform(subvec, fft, tw);

#pragma omp paralle for default(none) shared(window_size, spectrogram, num_stages, fft)
  for (size_t i = 0; i < window_size; i++)
    spectrogram[0][i] = fft[i][num_stages];

  // Initialize M
#pragma omp parallel for default(none) shared(window_size, M, num_stages, fft)
  for (size_t s = 0; s < num_stages; s++)
  {
    size_t num_rows = window_size / (1 << (s + 1));
    for (size_t k = 0; k < num_rows; k++)
    {
      size_t row_idx = stft::reverse_bits(k, log2(num_rows));
      for (size_t m = 0; m < (1 << s); m++)
        M[row_idx][s][m] = fft[(2 * k + 1) * (1 << s) + m][s];
    }
  }

  int n = 1;
  // Loop over all remaining signals, step size window_size/2
  while ((n + window_size / 2 - 1) + window_size - 1 < vec.size())
  {
    for (int s = 0; s < num_stages; s++)
    {
      // Prepare B
#pragma omp parallel for default(none) shared(window_size, M_prime, prev, vec, B, n, s, M)
      for (int i = 0; i < window_size / 2; i++)
      {
        if (s == 0)
        {
          M_prime[i][s][0] = vec[i + n + window_size - 1];
          prev[i] = M_prime[i][s];
        }
        B[i][0] = prev[i];
        B[i][1] = M[i][s];
      }

      // Compute stage s of nth FFT using B
#pragma omp parallel for default(none) shared(window_size, F, s)
      for (int i = 0; i < window_size / 2; i++)
        F[i] = rarray<std::complex<double>, 1>(1 << (s + 1));
      stft_mpi::stft_qpff_parallel(B, tw, window_size, s, F);
      prev = F;

      // Update M and M' if not the last stage
      int idx = window_size / (1 << (s + 2));
      if (s < num_stages - 1)
      {
#pragma omp parallel for default(none) shared(M_prime, window_size, idx, s, F)
        for (int r = 0; r < idx; r++)
          M_prime[r][s + 1] = F[window_size / 2 - idx + r];
#pragma omp parallel for default(none) shared(M, window_size, idx, s, F)
        for (int r = 0; r < window_size / 2 - idx; r++)
          M[idx + r][s + 1] = F[r];
      }
    }

    // Copy last stage of nth FFT to result
#pragma omp parallel for default(none) shared(window_size, spectrogram, n, F)
    for (int i = 0; i < window_size / 2; i++)
    {
      for (size_t j = 0; j < window_size; j++)
        spectrogram[n + i][j] = F[i][j];
    }

    // Swap buffers and move to next batch
    rarray<rarray<std::complex<double>, 1>, 2> temp = M;
    M = M_prime;
    M_prime = temp;
    n += window_size / 2;
  }

  // Compute the remaining (less than size of a batch) FFTs
  while (n < vec.size() - window_size + 1)
  {
#pragma omp parallel for default(none) shared(spectrogram, vec, window_size, n)
    for (int i = 0; i < window_size; i++)
      spectrogram[n][i] = vec[n + i];
    stft::transform(spectrogram[n]);
    n++;
  }

  return spectrogram;
}

void stft_mpi::stft_qpff_parallel(
    const rarray<rarray<std::complex<double>, 1>, 2> B,
    const rarray<std::complex<double>, 2> tw,
    const size_t window_size, const size_t s,
    rarray<rarray<std::complex<double>, 1>, 1> F)
{
  int rank, size, chunksize, remaining, local_chunksize, offset;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  chunksize = (window_size / 2) / size;
  remaining = (window_size / 2) % size;

  local_chunksize = rank < remaining ? chunksize + 1 : chunksize;
  offset = rank * chunksize + (remaining > rank ? rank : remaining);

  const size_t count = (1 << s);
  const size_t f_count = (1 << (s + 1));

  rarray<std::complex<double>, 1> f(local_chunksize * f_count);

#pragma omp parallel for default(none) shared(B, tw, window_size, s, local_chunksize, offset, count, f_count, f)
  for (int i = 0; i < local_chunksize; i++)
  {
    for (size_t k = 0; k < count; k++)
    {
      std::complex<double> xr, x0, x1;
      xr = B[i + offset][0][k] * tw[window_size - f_count + k][s];
      x0 = B[i + offset][1][k] + xr;
      x1 = B[i + offset][1][k] - xr;

      f[i * f_count + k] = x0;
      f[i * f_count + k + count] = x1;
    }
  }

  // Compute recvcounts and displacements for all ranks
  rarray<int, 1> recvcounts(size), displs(size);
#pragma omp parallel for default(none) shared(recvcounts, displs, f_count, size, remaining, chunksize)
  for (int p = 0; p < size; p++)
  {
    int p_chunksize = (p < remaining) ? chunksize + 1 : chunksize;
    recvcounts[p] = p_chunksize * f_count;
    displs[p] = (p * chunksize + std::min(p, remaining)) * f_count;
  }

  // Gather all partial results into F on all ranks
  rarray<std::complex<double>, 2> recv_F(window_size / 2, f_count);
  MPI_Allgatherv(f.data(), local_chunksize * f_count, MPI_DOUBLE_COMPLEX,
                 &recv_F[0][0], recvcounts.data(), displs.data(),
                 MPI_DOUBLE_COMPLEX, MPI_COMM_WORLD);

#pragma omp parallel for default(none) shared(window_size, F, recv_F)
  for (int i = 0; i < window_size / 2; i++)
    F[i] = recv_F[i];
}
#endif // WITH_MPI
