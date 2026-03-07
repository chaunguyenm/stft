#include "stft_io.h"
#include <iostream>
#include <netcdf>
using namespace netCDF;
using namespace netCDF::exceptions;

int stft_io::read_signal(
  const std::string &filePath, rarray<std::complex<double>, 1> signal
)
{
  try
  {
    // Open file
    NcFile *dataFile = new NcFile(filePath, NcFile::read);

    // Read dimensions
    NcDim xDim = dataFile->getDim("x");
    NcDim cDim = dataFile->getDim("c");

    int nx = xDim.getSize();
    int nc = cDim.getSize();

    int ex = signal.shape()[0];
    if ((ex != nx) || nc != 2)
    {
      std::cerr << "Incorrect dimensions, expected (" << ex << ", 2) but got (" << nx << ", " << nc << ")\n";
      return 1;
    }

    std::cout << nx << " by " << nc << "\n";

    // Create data array in memory
    rarray<double, 2> dataIn(nx, nc);

    // Read variable
    NcVar data = dataFile->getVar("signal");
    data.getVar(&dataIn[0][0]);

    for (int i = 0; i < nx; i++)
      signal[i] = std::complex(dataIn[i][0], dataIn[i][1]);

    // Close file
    delete dataFile;

    // Print data
//    for (int i = 0; i < nx; i++)
//      std::cout << "(" << signal[i].real() << ", " << signal[i].imag() << ") ";

    return 0;
  }
  catch (NcException& e)
  {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}

int stft_io::write_signal(
  const std::string &filePath, rarray<std::complex<double>, 1> signal
)
{
  try
  {
    // Create data array in memory
    int nx = signal.shape()[0];
    rarray<double, 2> dataOut(nx, 2);

    for (int i = 0; i < nx; i++)
    {
      dataOut[i][0] = signal[i].real();
      dataOut[i][1] = signal[i].imag();
    }

    // Create the NetCDF file
    NcFile *dataFile = new NcFile(filePath, NcFile::replace);

    // Create dimensions: x and y correspond to dimensions of stft matrix,
    // c corresponds to real/imaginary part
    NcDim xDim = dataFile->addDim("x", nx);
    NcDim cDim = dataFile->addDim("c", 2);

    // Create variable
    NcVar data = dataFile->addVar("signal", ncDouble, {xDim, cDim});

    // Put variable in file
    data.putVar(&dataOut[0][0]);

    // Close file
    delete dataFile;

    return 0;
  }
  catch (NcException& e)
  {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}

int stft_io::read_stft(
  const std::string &filePath, rarray<std::complex<double>, 2> stft)
{
  try
  {
    // Open file
    NcFile *dataFile = new NcFile(filePath, NcFile::read);

    // Read dimensions
    NcDim xDim = dataFile->getDim("x");
    NcDim yDim = dataFile->getDim("y");
    NcDim cDim = dataFile->getDim("c");

    int nx = xDim.getSize();
    int ny = yDim.getSize();
    int nc = cDim.getSize();

    int ex = stft.shape()[0];
    int ey = stft.shape()[1];
    if ((ex != nx) || (ey != ny) || nc != 2)
    {
      std::cerr << "Incorrect dimensions, expected (" << ex << ", " << ey << ", 2) but got (" << nx << ", " << ny << ", " << nc << ")\n";
      return 1;
    }

    std::cout << nx << " by " << ny << " by " << nc << "\n";

    // Create data array in memory
    rarray<double, 3> dataIn(nx, ny, nc);

    // Read variable
    NcVar data = dataFile->getVar("stft");
    data.getVar(&dataIn[0][0][0]);

    for (int i = 0; i < nx; i++)
      for (int j = 0; j < ny; j++)
        stft[i][j] = std::complex(dataIn[i][j][0], dataIn[i][j][1]);

    // Close file
    delete dataFile;

    // Print data
//    for (int i = 0; i < nx; i++)
//    {
//      for (int j = 0; j < ny; j++)
//        std::cout << "(" << stft[i][j].real() << ", " << stft[i][j].imag() << ") ";
//      std::cout << "\n";
//    }

    return 0;
  }
  catch (NcException& e)
  {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}

int stft_io::write_stft(
    const std::string &filePath,
    rarray<std::complex<double>, 2> stft)
{
  try
  {
    // Create data array in memory
    int nx = stft.shape()[0], ny = stft.shape()[1];
    rarray<double, 3> dataOut(nx, ny, 2);

    for (int i = 0; i < nx; i++)
      for (int j = 0; j < ny; j++)
      {
        dataOut[i][j][0] = stft[i][j].real();
        dataOut[i][j][1] = stft[i][j].imag();
      }

    // Create the NetCDF file
    NcFile *dataFile = new NcFile(filePath, NcFile::replace);

    // Create dimensions: x and y correspond to dimensions of stft matrix,
    // c corresponds to real/imaginary part
    NcDim xDim = dataFile->addDim("x", nx);
    NcDim yDim = dataFile->addDim("y", ny);
    NcDim cDim = dataFile->addDim("c", 2);

    // Create variable
    NcVar data = dataFile->addVar("stft", ncDouble, {xDim, yDim, cDim});

    // Put variable in file
    data.putVar(&dataOut[0][0][0]);

    // Close file
    delete dataFile;

    return 0;
  }
  catch (NcException& e)
  {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}
