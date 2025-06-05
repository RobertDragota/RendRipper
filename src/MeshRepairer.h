//
// Created by drago on 6/5/2025.
//

#ifndef MESHREPAIRER_H
#define MESHREPAIRER_H

#include <MRMesh/MRMeshLoad.h>
#include <MRMesh/MRMeshSave.h>
#include <MRMesh/MRMesh.h>
#include <MRMesh/MRRegionBoundary.h>
#include <MRMesh/MRNormalDenoising.h>
#include <MRMesh/MRBox.h>
#include <MRMesh/MRMeshFixer.h>
#include <iostream>
#include <filesystem>

#include "MRMesh/MRMeshFillHole.h"

using namespace MR;

class MeshRepairer {
public:
    static bool repairSTLFile(const std::string& inputPath, const std::string& outputPath) {
        std::cout << "Loading mesh from: " << inputPath << std::endl;

        // Load the STL file
        auto meshResult = MeshLoad::fromAnySupportedFormat(inputPath);
        if (!meshResult.has_value()) {
            std::cerr << "Failed to load STL file: " << inputPath << std::endl;
            return false;
        }

        MR::Mesh mesh = std::move(meshResult.value());
        std::cout << "Loaded mesh with " << mesh.topology.numValidFaces() << " faces" << std::endl;

        // Step 1: Remove degenerate triangles
        std::cout << "Removing degenerate triangles..." << std::endl;
        removeDegenerateTriangles(mesh);

        // Step 2: Fill holes to make mesh watertight
        std::cout << "Filling holes..." << std::endl;
        fillAllHoles(mesh);

        // Step 3: Basic mesh cleanup
        std::cout << "Cleaning up mesh..." << std::endl;
        cleanupMesh(mesh);

        // Save the repaired mesh
        std::cout << "Saving repaired mesh to: " << outputPath << std::endl;
        auto saveResult = MeshSave::toAnySupportedFormat(mesh, outputPath);
        if (!saveResult.has_value()) {
            std::cerr << "Failed to save repaired mesh" << std::endl;
            return false;
        }

        std::cout << "Mesh repair completed successfully!" << std::endl;
        std::cout << "Final mesh has " << mesh.topology.numValidFaces() << " faces" << std::endl;

        return true;
    }

private:
    static void removeDegenerateTriangles(MR::Mesh& mesh) {
        fixMeshDegeneracies( mesh, {
         .maxDeviation = 1e-5f * mesh.computeBoundingBox().diagonal(),
         .tinyEdgeLength = 1e-3f,
     } );
    }

    static void fillAllHoles(MR::Mesh& mesh) {

        std::vector<EdgeId> holeEdges = mesh.topology.findHoleRepresentiveEdges();

        for ( EdgeId e : holeEdges )
            {
            // Setup filling parameters
            FillHoleParams params;
            params.metric = MR::getUniversalMetric( mesh );
            // Fill hole represented by `e`
            fillHole( mesh, e, params );
            }
    }

    static void cleanupMesh(MR::Mesh& mesh) {
        meshDenoiseViaNormals( mesh );
    }
};



#endif //MESHREPAIRER_H
