# ChronosDB Engine

A lightweight, non-relational database engine built in C++ specifically optimized for storing, querying, and analyzing time-series signal data.

This project was developed as a final assignment for the Advanced C++ (ZCUZ) course. It strictly adheres to modern object-oriented principles, utilizing the STL, custom templates, and RAII memory management.

## Key Features

* **Custom Binary Storage**: Highly optimized append-only binary format that eliminates text-parsing overhead.
* **Sparse In-Memory Indexing**: Fast `O(log N)` time-range queries without scanning the whole file.
* **Streaming Analytics**: Calculates signal statistics (Min, Max, Mean, Variance) on the fly using Welford's algorithm.
* **Qt 6 GUI**: A responsive desktop application built with `QtWidgets` and `QtCharts` for visualizing massive time-series datasets (includes downsampling).
* **Built-in Benchmarking**: An integrated performance testing suite comparing the native binary engine against JSON and SQLite backends.

## Tech Stack

* **Language**: C++17
* **Build System**: CMake (>= 3.16)
* **GUI Framework**: Qt 6 (Core, Widgets, Charts)
* **Documentation**: Doxygen

## Building from Source

Ensure you have a C++17 compliant compiler and Qt 6 installed on your system.

```bash
# Clone the repository
git clone https://github.com/yourusername/ChronosDB-Engine.git
cd ChronosDB-Engine

# Create a build directory
mkdir build && cd build

# Configure and compile
cmake ..
cmake --build . --config Release
```

## Running the Application

After building, the main GUI executable can be found in the `build/gui/` directory.

```bash
./gui/ChronosDB_GUI
```

You can use the **"Generate Test Database"** button in the UI to quickly create a synthetic dataset for testing and exploration.

## Documentation

All core classes and public interfaces are documented using Doxygen. You can generate the HTML documentation by running `doxygen Doxyfile` (if configured in your environment).
