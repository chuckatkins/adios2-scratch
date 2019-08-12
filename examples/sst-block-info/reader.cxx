#include <iostream>

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

  try
  {
    adios2::ADIOS adios(MPI_COMM_WORLD, adios2::DebugON);
    auto io = adios.DeclareIO("TestIO");

    io.SetEngine(argv[1]);
    auto engine = io.Open(argv[2], adios2::Mode::Read);

    while(engine.BeginStep() == adios2::StepStatus::OK)
    {
      size_t step = engine.CurrentStep();
      auto var = io.InquireVariable<double>("foo");
      if(var)
      {
        auto blocks = engine.BlocksInfo(var, step);
        std::cout << "Step " << step << ", " << blocks.size() << " block(s)"
                  << std::endl;
      }
      else
      {
        std::cerr << "No variable found" << std::endl;
        break;
      }
      engine.EndStep();
    }
  }
  catch(const std::exception &ex)
  {
    std::cerr << "Error: " << ex.what() << std::endl;
    return 2;
  }

  return 0;
}
