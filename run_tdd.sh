#!/bin/bash

# Wrapper script to run TDD tests with Bsymbolic disabled
# Usage: run_tdd.sh [test_target]

# Extract product name from build.sh arguments or default
PRODUCT_NAME=""
for arg in "$@"; do
    case $arg in
        --product-name=*)
            PRODUCT_NAME="${arg#*=}"
            break
            ;;
        --product-name)
            PRODUCT_NAME="$2"
            break
            ;;
    esac
done

if [ -z "$PRODUCT_NAME" ]; then
    echo "Usage: run_tdd.sh --product-name <product_name> [other_build_args]"
    exit 1
fi

# Get all arguments except --product-name
ARGS=()
for arg in "$@"; do
    case $arg in
        --product-name=*)
            shift # skip --product-name=value
            ;;
        --product-name)
            shift # skip --product-name
            shift # skip value
            ;;
        *)
            ARGS+=("$arg")
            ;;
    esac
done

# If no test target specified, use ability_runtime_test
if [ ${#ARGS[@]} -eq 0 ]; then
    ARGS+=("--build-target" "ability_runtime_test")
fi

echo "Running TDD with Bsymbolic disabled..."
echo "Command: ./build.sh --product-name $PRODUCT_NAME ${ARGS[*]} --define ability_runtime_disable_bsymbolic_for_test=true"

# Run build with Bsymbolic disabled for test
./build.sh --product-name "$PRODUCT_NAME" "${ARGS[@]}" --define ability_runtime_disable_bsymbolic_for_test=true