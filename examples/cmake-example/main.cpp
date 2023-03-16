#include "MRMesh/MRAffineXf3.h"
#include "MRMesh/MRMesh.h"
#include "MRMesh/MRMeshRelax.h"
#include "MRMesh/MRMeshSubdivide.h"
#include "MRMesh/MRTorus.h"
#include "MRMesh/MRMeshLoad.h"
#include "MRMesh/MRMeshSave.h"

int main()
{
    std::filesystem::path inFilePath1 = "mesh.stl";
    MR::Mesh mesh1 = MR::MeshLoad::fromAnyStl(inFilePath);
    std::filesystem::path inFilePath2 = "mesh.stl";
    MR::Mesh mesh2 = MR::MeshLoad::fromAnyStl(inFilePath);

    MR::AffineXf3f xf3f = MR::AffineXf3f(3.0, 0);
    mesh1.transform(xf3);

    if (mesh1.has_value()) {
        std::filesystem::path outFilePath = "transform_test.ply";
        auto saveRes = MR::MeshSave::toPly(mesh1, outFilePath);
        if ( !saveRes.has_value() )
            std::cerr << saveRes.error() << "\n";
    } else
        std::cerr << loadRes.error() << "\n";
    return 0;
}