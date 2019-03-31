#include <cstdint>
#include <cstdlib>

#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

inline double time_malloc(size_t n)
{
    std::chrono::high_resolution_clock::time_point t0, t1;

    std::array<void *, 10> buf;
    t0 = std::chrono::high_resolution_clock::now();
    for(size_t i = 0; i < 10; ++i)
    {
        buf[i] = std::malloc(n);
    }
    t1 = std::chrono::high_resolution_clock::now();
    for(size_t i = 0; i < 10; ++i)
    {
        std::free(buf[i]);
    }

    return (t1-t0).count()/10;
}

inline double time_calloc(size_t n)
{
    std::chrono::high_resolution_clock::time_point t0, t1;

    std::array<void *, 10> buf;
    t0 = std::chrono::high_resolution_clock::now();
    for(size_t i = 0; i < 10; ++i)
    {
        buf[i] = std::calloc(n, 1);
    }
    t1 = std::chrono::high_resolution_clock::now();
    for(size_t i = 0; i < 10; ++i)
    {
        std::free(buf[i]);
    }

    return (t1-t0).count()/10;
}

inline double time_new(size_t n)
{
    std::chrono::high_resolution_clock::time_point t0, t1;

    std::array<uint8_t *, 10> buf;
    t0 = std::chrono::high_resolution_clock::now();
    for(size_t i = 0; i < 10; ++i)
    {
        buf[i] = new uint8_t[n];
    }
    t1 = std::chrono::high_resolution_clock::now();
    for(size_t i = 0; i < 10; ++i)
    {
        delete[] buf[i];
    }

    return (t1-t0).count()/10;
}
inline double time_vector(size_t n)
{
    std::chrono::high_resolution_clock::time_point t0, t1;

    std::array<std::vector<uint8_t>, 10> buf;
    t0 = std::chrono::high_resolution_clock::now();
    for(size_t i = 0; i < 10; ++i)
    {
        buf[i].resize(n);
    }
    t1 = std::chrono::high_resolution_clock::now();

    return (t1-t0).count()/10;
}

inline double time_vector2(size_t n)
{
    std::chrono::high_resolution_clock::time_point t0, t1;

    std::array<std::vector<uint8_t>, 10> buf;
    t0 = std::chrono::high_resolution_clock::now();
    for(size_t i = 0; i < 10; ++i)
    {
        buf[i].reserve(n);
    }
    t1 = std::chrono::high_resolution_clock::now();

    return (t1-t0).count()/10;
}

int main(int argc, char **argv)
{
    size_t nBytes = 1024*1024*1024*1ull;
    size_t nCount = 1024*128; // 128k
    size_t tSum;

    if(argc == 2)
    {
        nBytes = 1024*1024*1024*std::stoull(argv[1]);
    }
    else
    {
        nBytes = 1024*1024*1024*1ull;
    }

    std::cout << "Allocating " << nBytes/(1024*1024*1024) << "GB" << std::endl;

    tSum = 0.0;
    for(size_t i = 0; i < nCount; ++i)
    {
        tSum += time_malloc(nBytes);
    }
    std::cout << "malloc: "
        << (tSum/nCount)/1e3 << "us" << std::endl;

    tSum = 0.0;
    for(size_t i = 0; i < nCount; ++i)
    {
        tSum += time_calloc(nBytes);
    }
    std::cout << "calloc: "
        << (tSum/nCount)/1e3 << "us" << std::endl;

    tSum = 0.0;
    for(size_t i = 0; i < nCount; ++i)
    {
        tSum += time_new(nBytes);
    }
    std::cout << "new: "
        << (tSum/nCount)/1e3 << "us" << std::endl;

    tSum = 0.0;
    for(size_t i = 0; i < nCount; ++i)
    {
        tSum += time_vector2(nBytes);
    }
    std::cout << "vector(reserve): "
        << (tSum/nCount)/1e3 << "us" << std::endl;

    tSum = 0.0;
    for(size_t i = 0; i < nCount/8192; ++i)
    {
        tSum += time_vector(nBytes);
    }
    std::cout << "vector(resize): "
        << static_cast<size_t>((tSum/(nCount/8192))/1e3) << "us" << std::endl;

    return 0;
}
