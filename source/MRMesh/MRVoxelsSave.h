#pragma once
#if !defined( __EMSCRIPTEN__) && !defined( MRMESH_NO_VOXEL )
#include "MRIOFilters.h"
#include "MRExpected.h"
#include "MRProgressCallback.h"
#include "MRVoxelPath.h"
#include "MRSimpleVolume.h"
#include <filesystem>

namespace MR
{

namespace VoxelsSave
{

/// \addtogroup IOGroup
/// \{

MRMESH_API extern const IOFilters Filters;

/// Save raw voxels file, writing parameters in name
MRMESH_API VoidOrErrStr saveRaw( const std::filesystem::path& path, const VdbVolume& vdbVolume,
                                                    ProgressCallback callback = {} );

/// Save vdb voxels file, using openVdb methods
MRMESH_API VoidOrErrStr toVdb( const std::filesystem::path& path, const VdbVolume& vdbVolume,
                                                    ProgressCallback callback = {} );

MRMESH_API VoidOrErrStr toAnySupportedFormat( const std::filesystem::path& path, const VdbVolume& vdbVolume,
                                                                 ProgressCallback callback = {} );


/// save the slice by the active plane through the sliceNumber to an image file
MRMESH_API VoidOrErrStr saveSliceToImage( const std::filesystem::path& path, const VdbVolume& vdbVolume, const SlicePlane& slicePlain, int sliceNumber, ProgressCallback callback = {} );

// stores together all data for save voxel object as a group of images
struct SavingSettings
{
    // path to directory where you want to save images
    std::filesystem::path path;
    // format for file names, you should specify a placeholder for number and extension, eg "slice_{0:0{1}}.tif"
    std::string format = "slice_{0:0{1}}.tif";
    // Plane which the object is sliced by. XY, XZ, or YZ
    SlicePlane slicePlane;
    // Callback reporting progress
    ProgressCallback cb = {};
};

/// save all slices by the active plane through all voxel planes along the active axis to an image file
MRMESH_API VoidOrErrStr saveAllSlicesToImage( const VdbVolume& vdbVolume, const SavingSettings& settings );

/// \}

}

}
#endif
