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
    MR::Mesh mesh1 = MR::MeshLoad::fromAnyStl(inFilePath1).value();
    std::filesystem::path inFilePath2 = "mesh.stl";
    MR::Mesh mesh2 = MR::MeshLoad::fromAnyStl(inFilePath2).value();

    MR::AffineXf3f xf3f;
    Vector3<float> a1(1.0, 0.0, 0.0);
    Vector3<float> a2(0.0, 1.0, 0.0);
    Vector3<float> a3(0.0, 0.0, 1.0);
    Vector3<float> b(10.0, 0.0, 1.0);

    xf3f = MR::AffineXf3f( a1, b);

    mesh1.transform(xf3f);

    if (mesh1.has_value()) {
        std::filesystem::path outFilePath = "transform_test.ply";
        auto saveRes = MR::MeshSave::toPly(mesh1, outFilePath);
        if ( !saveRes.has_value() )
            std::cerr << saveRes.error() << "\n";
    } else
        std::cerr << loadRes.error() << "\n";
    return 0;
}