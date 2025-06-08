#include <iostream>
#include "Application.h"

int main() {
    try {
        Application app(1280, 720, "3D Slicer");
        app.Run();
    } catch (const std::exception& ex) {
        std::cerr << "Fatal: " << ex.what() << std::endl;
        return -1;
    }
    return 0;
}