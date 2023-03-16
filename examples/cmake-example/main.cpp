#include "MRMesh/MRAffineXf3.h"
#include "MRMesh/MRMesh.h"
#include "MRMesh/MRMeshRelax.h"
#include "MRMesh/MRMeshSubdivide.h"
#include "MRMesh/MRTorus.h"
#include "MRMesh/MRMeshLoad.h"
#include "MRMesh/MRMeshSave.h"
#include <iostream>

int main()
{
    // 加载stl文件
    std::filesystem::path inFilePath1 = "mesh.stl";
    MR::Mesh mesh1 = MR::MeshLoad::fromAnyStl(inFilePath1).value();
    std::filesystem::path inFilePath2 = "mesh.stl";
    MR::Mesh mesh2 = MR::MeshLoad::fromAnyStl(inFilePath2).value();

    // 平移
    MR::AffineXf3f xf3f;
    MR::Matrix3<float> A; // 直接用默认的矩阵就行
    MR::Vector3<float> b(10.0, 0.0, 1.0); // x轴正向平移10单位

    xf3f = MR::AffineXf3f( A, b);
    mesh1.transform(xf3f);

    // 将两个网格存储在同一个ply中
    mesh1.addPart(mesh2);
    std::filesystem::path outFilePath = "transform_test.ply";
    auto saveRes = MR::MeshSave::toPly(mesh1, outFilePath);
    return 0;
}