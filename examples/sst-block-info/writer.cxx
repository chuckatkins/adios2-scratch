#include <algorithm>
#include <iostream>
#include <random>
#include <vector>

#define OMPI_SKIP_MPICXX
#include <mpi.h>

#include <adios2.h>

int main(int argc, char **argv)
{
  if(argc != 3)
  {
    std::cerr << "Usage: " << argv[0] << " [engine] [filename]" << std::endl;
    return 1;
  }

  MPI_Init(&argc, &argv);

  int mpiSize, mpiRank;
  MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);

  size_t count = 100;
  size_t shape = mpiSize * count;
  size_t start = mpiRank*count;
  std::vector<double> data(count);

  std::random_device rDev;
  std::default_random_engine rEng(rDev());
  std::uniform_real_distribution<> rDist(0, 1);

  try
  {
    adios2::ADIOS adios(MPI_COMM_WORLD, adios2::DebugON);
    auto io = adios.DeclareIO("TestIO");
    auto var = io.DefineVariable<double>("foo", {shape}, {start}, {count});

    io.SetEngine(argv[1]);
    auto engine = io.Open(argv[2], adios2::Mode::Write);

    size_t step = 0;
    while(true)
    {
      std::generate(data.begin(), data.end(), [&](){return rDist(rEng);});

      if(mpiRank == 0)
      {
        std::clog << "Step " << step << " begin..." << std::flush;
      }
      engine.BeginStep();
      engine.Put(var, data.data());
      engine.EndStep();
      if(mpiRank == 0)
      {
        std::clog << "end" << std::endl;
      }
      ++step;
    }
  }
  catch(const std::exception &ex)
  {
    std::cerr << "Error: " << ex.what() << std::endl;
    return 2;
  }

  return 0;
}
