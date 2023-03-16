#include "MRMesh/MRAffineXf3.h"
#include "MRMesh/MRMesh.h"
#include "MRMesh/MRMeshRelax.h"
#include "MRMesh/MRMeshSubdivide.h"
#include "MRMesh/MRTorus.h"
#include "MRMesh/MRMeshLoad.h"
#include "MRMesh/MRMeshSave.h"
#include "MRMesh/MRMeshFwd.h"
#include <iostream>
#include "MRMesh/MRBox.h"

int main(int argc, char const *argv[])
{
    if (argc < 3) return 0;
    // 加载stl文件
    std::filesystem::path inFilePath1 = argv[1];
    MR::Mesh mesh1 = MR::MeshLoad::fromAnyStl(inFilePath1).value();
    std::filesystem::path inFilePath2 = argv[1];
    MR::Mesh mesh2 = MR::MeshLoad::fromAnyStl(inFilePath2).value();

    // 平移
    MR::Box3f box = mesh1.getBoundingBox();
    float zDistance = MR::depth(box);
    float yDistance = MR::height(box);
    float xDistance = MR::width(box);
    std::cout << " x:" << xDistance << " y: " << yDistance << " z: " << zDistance << std::endl;
    MR::AffineXf3f xf3f, yf3f, zf3f;
    MR::Matrix3<float> A; // 直接用默认的矩阵就行
    MR::Vector3<float> bx(xDistance, 0.0, 0.0); // x轴正向平移
    MR::Vector3<float> by(0.0, yDistance, 0.0); // y轴正向平移
    MR::Vector3<float> bz(0.0, 0.0, zDistance); // z轴正向平移

    xf3f = MR::AffineXf3f( A, bx);
    yf3f = MR::AffineXf3f( A, by);
    zf3f = MR::AffineXf3f( A, bz);

    int choice = argv[2];

    switch (choice)
    {
    case 0:
        mesh1.transform(xf3f);
        break;
    case 1:
        mesh1.transform(yf3f);
        break;
    case 2:
        mesh1.transform(zf3f);
        break;
    
    default:
        break;
    }

    // 合并mesh存储在ply中
    mesh1.addPart(mesh2);
    std::filesystem::path outFilePath = "transform_test.ply";
    auto saveRes = MR::MeshSave::toPly(mesh1, outFilePath);
    return 0;
}
