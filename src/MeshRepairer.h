#pragma once

#include <MRMesh/MRMesh.h>
#include <MRMesh/MRMeshLoad.h>
#include <MRMesh/MRMeshSave.h>
#include <MRMesh/MRRegionBoundary.h>
#include <MRMesh/MRNormalDenoising.h>
#include <MRMesh/MRBox.h>
#include <MRMesh/MRMeshFixer.h>
#include "MRMesh/MRMeshFillHole.h"

#include <iostream>
#include <filesystem>
#include <optional>

using namespace MR;

class MeshLoader {
public:
    auto Load(const std::string &path) const
    {
        return MeshLoad::fromAnySupportedFormat(path);
    }
};

class MeshCleaner {
public:
    void RemoveDegenerateTriangles(MR::Mesh &mesh) const
    {
        fixMeshDegeneracies(mesh, {
            .maxDeviation = 1e-5f * mesh.computeBoundingBox().diagonal(),
            .tinyEdgeLength = 1e-3f,
        });
    }

    void FillAllHoles(MR::Mesh &mesh) const
    {
        std::vector<EdgeId> holeEdges = mesh.topology.findHoleRepresentiveEdges();
        for (EdgeId e : holeEdges)
        {
            FillHoleParams params;
            params.metric = MR::getUniversalMetric(mesh);
            fillHole(mesh, e, params);
        }
    }

    void Cleanup(MR::Mesh &mesh) const
    {
        meshDenoiseViaNormals(mesh);
    }
};

class MeshSaver {
public:
    bool Save(const MR::Mesh &mesh, const std::string &path) const
    {
        auto res = MeshSave::toAnySupportedFormat(mesh, path);
        return res.has_value();
    }
};

class MeshRepairer {
public:
    static bool repairSTLFile(const std::string &inputPath, const std::string &outputPath)
    {
        MeshRepairer repairer;
        return repairer.repair(inputPath, outputPath);
    }

    bool repair(const std::string &inputPath, const std::string &outputPath)
    {
        std::cout << "Loading mesh from: " << inputPath << std::endl;
        auto meshOpt = loader_.Load(inputPath);
        if (!meshOpt)
        {
            std::cerr << "Failed to load STL file: " << inputPath << std::endl;
            return false;
        }

        MR::Mesh mesh = std::move(*meshOpt);
        std::cout << "Loaded mesh with " << mesh.topology.numValidFaces() << " faces" << std::endl;

        std::cout << "Removing degenerate triangles..." << std::endl;
        cleaner_.RemoveDegenerateTriangles(mesh);

        std::cout << "Filling holes..." << std::endl;
        cleaner_.FillAllHoles(mesh);

        std::cout << "Cleaning up mesh..." << std::endl;
        cleaner_.Cleanup(mesh);

        std::cout << "Saving repaired mesh to: " << outputPath << std::endl;
        if (!saver_.Save(mesh, outputPath))
        {
            std::cerr << "Failed to save repaired mesh" << std::endl;
            return false;
        }

        std::cout << "Mesh repair completed successfully!" << std::endl;
        std::cout << "Final mesh has " << mesh.topology.numValidFaces() << " faces" << std::endl;
        return true;
    }

private:
    MeshLoader  loader_;
    MeshCleaner cleaner_;
    MeshSaver   saver_;
};

