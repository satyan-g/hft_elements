#!/bin/bash
# Build script for hft_elements project

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Default values
BUILD_TYPE="Release"
CLEAN=0
TESTS=0
JOBS=$(nproc)
USE_DOCKER=0

# Parse arguments
show_usage() {
    cat << EOF
Usage: $0 [BUILD_TYPE] [OPTIONS]

Build types:
    debug, Debug, DEBUG       - Debug build with symbols, no optimization
    release, Release, RELEASE - Release build with optimizations (default)

Options:
    --clean                   - Clean build directory before building
    --tests                   - Build with tests enabled
    --docker                  - Build using Docker (GCC 14 + CMake 3.28)
    -j, --jobs N              - Number of parallel jobs (default: $(nproc))
    -h, --help                - Show this help message

Examples:
    $0                        - Build release version locally
    $0 debug                  - Build debug version locally
    $0 release --clean        - Clean and build release locally
    $0 release --docker       - Build release in Docker (GCC 14)
    $0 debug --docker --clean - Clean and build debug in Docker
EOF
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        debug|Debug|DEBUG)
            BUILD_TYPE="Debug"
            shift
            ;;
        release|Release|RELEASE)
            BUILD_TYPE="Release"
            shift
            ;;
        --clean)
            CLEAN=1
            shift
            ;;
        --tests)
            TESTS=1
            shift
            ;;
        --docker)
            USE_DOCKER=1
            shift
            ;;
        -j|--jobs)
            JOBS="$2"
            shift 2
            ;;
        -h|--help)
            show_usage
            exit 0
            ;;
        *)
            echo -e "${RED}Error: Unknown argument: $1${NC}"
            show_usage
            exit 1
            ;;
    esac
done

# Get project root (parent of scripts directory)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Build directory based on build type
BUILD_DIR="${PROJECT_ROOT}/build/${BUILD_TYPE,,}"

echo -e "${GREEN}=== HFT Elements Build Script ===${NC}"
echo -e "Build type: ${YELLOW}${BUILD_TYPE}${NC}"
echo -e "Build directory: ${YELLOW}${BUILD_DIR}${NC}"
echo -e "Docker build: ${YELLOW}$([ $USE_DOCKER -eq 1 ] && echo 'Yes (GCC 14)' || echo 'No (local)')${NC}"
echo -e "Parallel jobs: ${YELLOW}${JOBS}${NC}"
echo -e "Tests: ${YELLOW}$([ $TESTS -eq 1 ] && echo 'Enabled' || echo 'Disabled')${NC}"
echo ""

# Docker build
if [ $USE_DOCKER -eq 1 ]; then
    echo -e "${GREEN}Building with Docker...${NC}"
    
    # Build Docker image if it doesn't exist
    if ! docker image inspect hft-build:latest >/dev/null 2>&1; then
        echo -e "${YELLOW}Building Docker image (first time only)...${NC}"
        docker build -f "${PROJECT_ROOT}/docker/Dockerfile.build" -t hft-build:latest "${PROJECT_ROOT}"
    fi
    
    # Clean build directory if requested
    if [ $CLEAN -eq 1 ]; then
        echo -e "${YELLOW}Cleaning build directory...${NC}"
        rm -rf "${BUILD_DIR}"
    fi
    
    # Create build directory
    mkdir -p "${BUILD_DIR}"
    
    # Build CMake arguments
    CMAKE_ARGS="-DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_C_COMPILER=gcc-12 -DCMAKE_CXX_COMPILER=g++-12"
    if [ $TESTS -eq 1 ]; then
        CMAKE_ARGS="${CMAKE_ARGS} -DBUILD_TESTS=ON"
    fi
    
    # Run build in Docker with mounted workspace
    echo -e "${GREEN}Running build in Docker container...${NC}"
    docker run --rm \
        -v "${PROJECT_ROOT}:/workspace" \
        -w /workspace \
        -u "$(id -u):$(id -g)" \
        hft-build:latest \
        bash -c "mkdir -p build/${BUILD_TYPE,,} && cd build/${BUILD_TYPE,,} && \
                 cmake ${CMAKE_ARGS} ../.. && \
                 cmake --build . -j${JOBS}"
    
    echo ""
    echo -e "${GREEN}=== Docker Build Complete ===${NC}"
    echo -e "Binaries are in: ${YELLOW}${BUILD_DIR}${NC}"
    echo -e "Built with: ${YELLOW}GCC 12 + CMake 3.22 (Ubuntu 22.04)${NC}"
    echo -e "Static linking: ${YELLOW}libstdc++ and libgcc${NC}"
    echo -e "glibc: ${YELLOW}2.35 (host compatible)${NC}"
    echo ""
    
else
    # Local build
    # Clean if requested
    if [ $CLEAN -eq 1 ]; then
        echo -e "${YELLOW}Cleaning build directory...${NC}"
        rm -rf "$BUILD_DIR"
    fi

    # Create build directory
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    # Configure with CMake
    echo -e "${GREEN}Configuring with CMake...${NC}"
    CMAKE_ARGS=(
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
    )

    if [ $TESTS -eq 1 ]; then
        CMAKE_ARGS+=(-DBUILD_TESTS=ON)
    fi

    cmake "${CMAKE_ARGS[@]}" "$PROJECT_ROOT"

    # Build
    echo -e "${GREEN}Building...${NC}"
    cmake --build . -j"$JOBS"

    # Run tests if enabled
    if [ $TESTS -eq 1 ]; then
        echo -e "${GREEN}Running tests...${NC}"
        ctest --output-on-failure
    fi

    echo ""
    echo -e "${GREEN}=== Build Complete ===${NC}"
fi

# Show available utilities
if [ -f "${BUILD_DIR}/examples/utilities/hft_host_diag/hft_host_diag" ]; then
    echo -e "Utilities:"
    echo -e "  - hft_host_diag: ${YELLOW}${BUILD_DIR}/examples/utilities/hft_host_diag/hft_host_diag${NC}"
fi

if [ -f "${BUILD_DIR}/examples/utilities/dbncat/dbncat" ]; then
    echo -e "  - dbncat:        ${YELLOW}${BUILD_DIR}/examples/utilities/dbncat/dbncat${NC}"
fi

echo ""
echo -e "To run: ${YELLOW}./scripts/run.sh ${BUILD_TYPE,,} <component>${NC}"
