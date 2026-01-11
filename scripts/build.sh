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
    -j, --jobs N              - Number of parallel jobs (default: $(nproc))
    -h, --help                - Show this help message

Examples:
    $0                        - Build release version
    $0 debug                  - Build debug version
    $0 release --clean        - Clean and build release
    $0 debug --tests          - Build debug with tests
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
echo -e "Parallel jobs: ${YELLOW}${JOBS}${NC}"
echo -e "Tests: ${YELLOW}$([ $TESTS -eq 1 ] && echo 'Enabled' || echo 'Disabled')${NC}"
echo ""

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
echo -e "Binaries location:"
echo -e "  - dbn_simulator: ${YELLOW}${BUILD_DIR}/src/simulator/dbn_simulator${NC}"
echo -e "  - mbl_server:    ${YELLOW}${BUILD_DIR}/src/server/mbl_server${NC}"
echo -e "  - mbl_collector: ${YELLOW}${BUILD_DIR}/src/collector/mbl_collector${NC}"
echo ""
echo -e "To run: ${YELLOW}./scripts/run.sh ${BUILD_TYPE,,}${NC}"

