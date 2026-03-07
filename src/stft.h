#ifndef STFT_H
#define STFT_H

#include <complex>
#include "rarray"

namespace stft
{
    rarray<std::complex<double>, 2> stft_dft(
        const rarray<std::complex<double>, 1> vec, const size_t window_size);
    rarray<std::complex<double>, 2> stft_fft(
        const rarray<std::complex<double>, 1> vec, const size_t window_size);
    rarray<std::complex<double>, 2> stft_ff(
        const rarray<std::complex<double>, 1> vec, const size_t window_size);
    rarray<std::complex<double>, 2> stft_qpff(
        const rarray<std::complex<double>, 1> vec, const size_t window_size);

    rarray<std::complex<double>, 1> dft(
        const rarray<std::complex<double>, 1> signal,
        size_t vec_begin, size_t vec_size);
    rarray<std::complex<double>, 1> fft(
        const rarray<std::complex<double>, 1> signal,
        size_t vec_begin, size_t vec_size);

    size_t reverse_bits(size_t val, int width);
    bool transform(rarray<std::complex<double>, 1> vec);
    bool transform(rarray<std::complex<double>, 1> vec,
                   rarray<std::complex<double>, 2> fft,
                   rarray<std::complex<double>, 2> tw);
    void stft_qpff_parallel(
        const rarray<rarray<std::complex<double>, 1>, 2> B,
        const rarray<std::complex<double>, 2> tw,
        const size_t window_size, const size_t s,
        rarray<rarray<std::complex<double>, 1>, 1> F);
}

#ifdef WITH_MPI
namespace stft_mpi
{
    rarray<std::complex<double>, 2> stft_dft(
        const rarray<std::complex<double>, 1> vec, const size_t window_size);
    rarray<std::complex<double>, 2> stft_fft(
        const rarray<std::complex<double>, 1> vec, const size_t window_size);
    rarray<std::complex<double>, 2> stft_qpff(
        const rarray<std::complex<double>, 1> vec, const size_t window_size);
    void stft_qpff_parallel(
        const rarray<rarray<std::complex<double>, 1>, 2> B,
        const rarray<std::complex<double>, 2> tw,
        const size_t window_size, const size_t s,
        rarray<rarray<std::complex<double>, 1>, 1> F);
}
#endif // WITH_MPI

#endif
