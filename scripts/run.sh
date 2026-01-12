#!/bin/bash
# Run script for hft_elements project

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default values
BUILD_TYPE="release"
COMPONENT=""

# Parse arguments
show_usage() {
    cat << EOF
Usage: $0 [BUILD_TYPE] COMPONENT [ARGS...]

Build types:
    debug, release (default: release)

Components:
    feed_simulator  - Run feed_simulator
    book_server     - Run book_server
    data_collector  - Run data_collector
    all             - Run all components in separate terminals

Examples:
    $0 feed_simulator --help
    $0 release feed_simulator --file data/sample.dbn --port 9550
    $0 debug book_server --mbo-host localhost --mbo-port 9550
    $0 data_collector --host localhost --port 9551 --output ./output
    $0 all

Component-specific arguments are passed through to the binary.
EOF
}

# Parse command line arguments
if [[ $# -eq 0 ]]; then
    show_usage
    exit 1
fi

# First argument might be build type
case $1 in
    debug|Debug|DEBUG)
        BUILD_TYPE="debug"
        shift
        ;;
    release|Release|RELEASE)
        BUILD_TYPE="release"
        shift
        ;;
esac

# Second argument should be component
if [[ $# -eq 0 ]]; then
    echo -e "${RED}Error: Component name required${NC}"
    show_usage
    exit 1
fi

COMPONENT="$1"
shift

# Get project root (parent of scripts directory)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Build directory based on build type
BUILD_DIR="${PROJECT_ROOT}/build/${BUILD_TYPE}"

# Check if build exists
if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${RED}Error: Build directory not found: ${BUILD_DIR}${NC}"
    echo -e "${YELLOW}Run: ./scripts/build.sh ${BUILD_TYPE}${NC}"
    exit 1
fi

# Run component
case $COMPONENT in
    feed_simulator|simulator)
        BINARY="${BUILD_DIR}/examples/mbo_to_mbl/feed_simulator/feed_simulator"
        if [ ! -f "$BINARY" ]; then
            echo -e "${RED}Error: Binary not found: ${BINARY}${NC}"
            exit 1
        fi
        echo -e "${GREEN}Running feed_simulator (${BUILD_TYPE})...${NC}"
        exec "$BINARY" "$@"
        ;;
    
    book_server|server)
        BINARY="${BUILD_DIR}/examples/mbo_to_mbl/book_server/book_server"
        if [ ! -f "$BINARY" ]; then
            echo -e "${RED}Error: Binary not found: ${BINARY}${NC}"
            exit 1
        fi
        echo -e "${GREEN}Running book_server (${BUILD_TYPE})...${NC}"
        exec "$BINARY" "$@"
        ;;
    
    data_collector|collector)
        BINARY="${BUILD_DIR}/examples/mbo_to_mbl/data_collector/data_collector"
        if [ ! -f "$BINARY" ]; then
            echo -e "${RED}Error: Binary not found: ${BINARY}${NC}"
            exit 1
        fi
        echo -e "${GREEN}Running data_collector (${BUILD_TYPE})...${NC}"
        exec "$BINARY" "$@"
        ;;
    
    all)
        echo -e "${GREEN}=== Starting All Components ===${NC}"
        echo -e "${YELLOW}Note: This requires sample data and will run components in background${NC}"
        echo ""
        
        # Check binaries exist
        SIMULATOR="${BUILD_DIR}/examples/mbo_to_mbl/feed_simulator/feed_simulator"
        SERVER="${BUILD_DIR}/examples/mbo_to_mbl/book_server/book_server"
        COLLECTOR="${BUILD_DIR}/examples/mbo_to_mbl/data_collector/data_collector"
        
        if [ ! -f "$SIMULATOR" ] || [ ! -f "$SERVER" ] || [ ! -f "$COLLECTOR" ]; then
            echo -e "${RED}Error: One or more binaries not found${NC}"
            exit 1
        fi
        
        # Create output directory
        mkdir -p "${PROJECT_ROOT}/output"
        
        # Start simulator
        echo -e "${BLUE}[1/3] Starting simulator on port 9550...${NC}"
        if [ -f "${PROJECT_ROOT}/data/sample.dbn" ]; then
            "$SIMULATOR" --file "${PROJECT_ROOT}/data/sample.dbn" --port 9550 &
            SIMULATOR_PID=$!
            echo -e "  PID: $SIMULATOR_PID"
        else
            echo -e "${YELLOW}  Warning: No sample.dbn found, skipping simulator${NC}"
            SIMULATOR_PID=""
        fi
        
        # Give simulator time to start
        sleep 2
        
        # Start server
        echo -e "${BLUE}[2/3] Starting MBL server on port 9551...${NC}"
        "$SERVER" --mbo-host localhost --mbo-port 9550 --snapshot-port 9551 &
        SERVER_PID=$!
        echo -e "  PID: $SERVER_PID"
        
        # Give server time to start
        sleep 2
        
        # Start collector
        echo -e "${BLUE}[3/3] Starting collector...${NC}"
        "$COLLECTOR" --host localhost --port 9551 --output "${PROJECT_ROOT}/output" &
        COLLECTOR_PID=$!
        echo -e "  PID: $COLLECTOR_PID"
        
        echo ""
        echo -e "${GREEN}=== All Components Started ===${NC}"
        [ -n "$SIMULATOR_PID" ] && echo -e "Simulator: PID ${SIMULATOR_PID} (port 9550)"
        echo -e "Server:    PID ${SERVER_PID} (port 9551)"
        echo -e "Collector: PID ${COLLECTOR_PID} (output: ./output)"
        echo ""
        echo -e "${YELLOW}To stop all:${NC}"
        [ -n "$SIMULATOR_PID" ] && echo -e "  kill ${SIMULATOR_PID} ${SERVER_PID} ${COLLECTOR_PID}"
        echo -e "  or press Ctrl+C"
        
        # Wait for user interrupt
        trap "echo ''; echo 'Stopping...'; [ -n '$SIMULATOR_PID' ] && kill $SIMULATOR_PID 2>/dev/null; kill $SERVER_PID $COLLECTOR_PID 2>/dev/null; exit 0" INT TERM
        wait
        ;;
    
    -h|--help|help)
        show_usage
        exit 0
        ;;
    
    *)
        echo -e "${RED}Error: Unknown component: ${COMPONENT}${NC}"
        echo -e "Valid components: feed_simulator, book_server, data_collector, all"
        show_usage
        exit 1
        ;;
esac

