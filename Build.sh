

set -e

echo "==> Compiling main.c...."
gcc main.c -o vulkan_test -lvulkan -lglfw -lm
echo "==> Compilation Successful!"



case "$1" in
    run)
        echo "==> Running program..."
        ./vulkan_test
        ;;
    memtest)
        echo "==> Running memtest with Valgrind..."
        if command -b valgring &> /dev/null; then
            valgrind --leak-check=full --show-leak-kinds=definite,indirect --suppressions=vulkan.supp ./vulkan_test
        else
            echo "Error: valgrind command is not avaiable on your system, try installing valgrind"
            exit 1
        fi
        ;;
    "")
        echo "==> Done. (Pass 'run' or 'memtest' to execute)"
        ;;
    *)
        echo "Usage: $0 [run|memtest]"
        exit 1
        ;;
    esac

