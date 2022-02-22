#pragma once

#include "MRMeshFwd.h"
#include <tl/expected.hpp>
#include <filesystem>
#include <ostream>
#include <string>
#include "MRIOFilters.h"

namespace MR
{

namespace LinesSave
{

MRMESH_API extern const IOFilters Filters;

// saves in .mrpolyline file
MRMESH_API tl::expected<void, std::string> toMrLines( const Polyline& polyline, const std::filesystem::path& file );
MRMESH_API tl::expected<void, std::string> toMrLines( const Polyline& polyline, std::ostream& out );

// detects the format from file extension and saves polyline in it
MRMESH_API tl::expected<void, std::string> toAnySupportedFormat( const Polyline& polyline, const std::filesystem::path& file );

} //namespace LinesSave

} //namespace MR