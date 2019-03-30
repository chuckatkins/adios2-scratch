#include <cstdint>
#include <cstdlib>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <vector>

inline double time_malloc(size_t n)
{
    std::chrono::high_resolution_clock::time_point t0, t1;

    t0 = std::chrono::high_resolution_clock::now();
    void *buf = std::malloc(n);
    t1 = std::chrono::high_resolution_clock::now();
    std::free(buf);

    return (t1-t0).count();
}

template<typename T>
inline double time_calloc(size_t n)
{
    std::chrono::high_resolution_clock::time_point t0, t1;

    t0 = std::chrono::high_resolution_clock::now();
    void *buf = std::calloc(n/sizeof(T), sizeof(T));
    t1 = std::chrono::high_resolution_clock::now();
    std::free(buf);

    return (t1-t0).count();
}

template<typename T>
inline double time_new(size_t n)
{
    std::chrono::high_resolution_clock::time_point t0, t1;

    t0 = std::chrono::high_resolution_clock::now();
    T *buf = new T[n/sizeof(T)];
    t1 = std::chrono::high_resolution_clock::now();
    delete[] buf;

    return (t1-t0).count();
}
template<typename T>
inline double time_vector(size_t n)
{
    std::chrono::high_resolution_clock::time_point t0, t1;

    t0 = std::chrono::high_resolution_clock::now();
    typename std::vector<T> v(n/sizeof(T));
    t1 = std::chrono::high_resolution_clock::now();
    return (t1-t0).count();
}

template<typename T>
inline double time_vector2(size_t n)
{
    std::chrono::high_resolution_clock::time_point t0, t1;

    t0 = std::chrono::high_resolution_clock::now();
    typename std::vector<T> v;
    v.reserve(n/sizeof(T));
    t1 = std::chrono::high_resolution_clock::now();
    return (t1-t0).count();
}

int main(int argc, char **argv)
{
    size_t nBytes = 1024*1024*1024*1ull;
    size_t nCount = 1024*128; // 128k
    size_t tSum;

    std::cout << "Allocating " << nBytes / (1024*1024*1024) << "GB" << std::endl;
    // malloc
    tSum = 0.0;
    for(size_t i = 0; i < nCount; ++i)
    {
        tSum += time_malloc(nBytes);
    }
    std::cout << "malloc: " << (tSum / nCount)/1e3 << "us" << std::endl;

    tSum = 0.0;
    for(size_t i = 0; i < nCount; ++i)
    {
        tSum += time_calloc<uint8_t>(nBytes);
    }
    std::cout << "calloc<uint8_t>: " << (tSum / nCount)/1e3 << "us" << std::endl;

    tSum = 0.0;
    for(size_t i = 0; i < nCount; ++i)
    {
        tSum += time_new<uint8_t>(nBytes);
    }
    std::cout << "new<uint8_t>: " << (tSum / nCount)/1e3 << "us" << std::endl;

    tSum = 0.0;
    for(size_t i = 0; i < nCount; ++i)
    {
        tSum += time_vector2<uint8_t>(nBytes);
    }
    std::cout << "vector<uint8_t>(reserve): " << (tSum / nCount)/1e3 << "us" << std::endl;

    tSum = 0.0;
    for(size_t i = 0; i < nCount/8192; ++i)
    {
        tSum += time_vector<uint8_t>(nBytes);
    }
    std::cout << "vector<uint8_t>: " << (tSum / (nCount/8192))/1e3 << "us" << std::endl;

    return 0;
}
