#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "IO_utility.h"
#include "GCodeParser.h"
#include <fstream>
#include <cstdio>

TEST_CASE("FileIO read text file") {
    const char* fname = "tmp_test.txt";
    {
        std::ofstream out(fname);
        out << "hello";
    }
    std::string contents = IO_utility::FileIO::ReadTextFile(fname);
    CHECK(contents == "hello");
    std::remove(fname);
}

TEST_CASE("GCodeParser parses layers") {
    const char* gcodeFile = "tmp_test.gcode";
    std::ofstream out(gcodeFile);
    out << "G1 X0 Y0 Z0 E0\n";
    out << "G1 X1 Y0 Z0 E1\n";
    out << "G1 X1 Y1 Z0 E2\n";
    out << "G1 X0 Y1 Z0.2 E3\n";
    out.close();

    GCodeParser parser;
    std::vector<std::vector<GCodeColoredVertex> > layers;
    std::vector<float> layerZs;
    parser.Parse(gcodeFile, layers, layerZs);
    std::remove(gcodeFile);

    REQUIRE(layerZs.size() == 2);
    CHECK(layerZs[0] == doctest::Approx(0.0f));
    CHECK(layerZs[1] == doctest::Approx(0.2f));
    REQUIRE(layers.size() == 2);
    CHECK(layers[0].size() == 4); // two segments
    CHECK(layers[1].size() == 2); // one segment
}
