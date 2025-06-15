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

/**
 * @brief Thin wrapper around MR::MeshLoad utilities.
 */
class MeshLoader {
public:
    /**
     * @brief Load a mesh from any supported format.
     * @param path The file path to the mesh.
     * @return An optional containing the loaded mesh if successful, or std::nullopt if loading failed.
     */
    auto Load(const std::string &path) const {
        return MeshLoad::fromAnySupportedFormat(path);
    }
};

/**
 * @brief Provides various mesh cleaning operations.
 */
class MeshCleaner {
public:
    /**
     * @brief Remove degenerate triangles from the mesh.
     * @param mesh The mesh to clean.
     */
    void RemoveDegenerateTriangles(MR::Mesh &mesh) const
    {
        fixMeshDegeneracies(mesh, {
            .maxDeviation = 1e-5f * mesh.computeBoundingBox().diagonal(),
            .tinyEdgeLength = 1e-3f,
        });
    }

    /**
     * @brief Fill all topological holes in the mesh.
     * @param mesh The mesh to process.
     */
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

    /**
     * @brief Perform normal-based mesh denoising.
     * @param mesh The mesh to clean.
     */
    void Cleanup(MR::Mesh &mesh) const
    {
        meshDenoiseViaNormals(mesh);
    }
};

/**
 * @brief Helper for saving meshes back to disk.
 */
class MeshSaver {
public:
    /**
     * @brief Save the given mesh to file.
     * @param mesh The mesh to save.
     * @param path The file path to save the mesh to.
     * @return True if the mesh was saved successfully, false otherwise.
     */
    bool Save(const MR::Mesh &mesh, const std::string &path) const
    {
        auto res = MeshSave::toAnySupportedFormat(mesh, path);
        return res.has_value();
    }
};

/**
 * @brief High level mesh repair pipeline using MeshLoader/Cleaner/Saver.
 */
class MeshRepairer {
public:
    /**
     * @brief Convenience method to repair an STL file in one call.
     * @param inputPath The file path to the input STL file.
     * @param outputPath The file path to save the repaired STL file.
     * @return True if the repair was successful, false otherwise.
     */
    static bool repairSTLFile(const std::string &inputPath, const std::string &outputPath)
    {
        MeshRepairer repairer;
        return repairer.repair(inputPath, outputPath);
    }

    /**
     * @brief Run the repair pipeline on the given input file.
     * @param inputPath The file path to the input mesh file.
     * @param outputPath The file path to save the repaired mesh file.
     * @return True if the repair was successful, false otherwise.
     */
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
    MeshLoader  loader_; ///< Utility for loading meshes.
    MeshCleaner cleaner_; ///< Utility for cleaning meshes.
    MeshSaver   saver_; ///< Utility for saving meshes.
};