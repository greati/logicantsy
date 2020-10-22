#include "cli/cli11apps.h"

int main(int argc, char** argv) {
    ltsy::MainCLI11App main_app;
    CLI11_PARSE(main_app, argc, argv);
    return 0;
}
