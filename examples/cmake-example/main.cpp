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
    MR::AffineXf3f xf3f, yf3f, zf3f;
    MR::Matrix3<float> A; // 直接用默认的矩阵就行
    MR::Vector3<float> bx(10.0, 0.0, 0.0); // x轴正向平移10单位

    MR::Vector3<float> by(0.0, 10.0, 0.0); // y轴正向平移10单位
    MR::Vector3<float> bz(0.0, 0.0, 10.0); // z轴正向平移10单位

    xf3f = MR::AffineXf3f( A, bx);
    yf3f = MR::AffineXf3f( A, by);
    zf3f = MR::AffineXf3f( A, bz);
    //mesh1.transform(xf3f);
    mesh1.transform(yf3f);
    //mesh1.transform(zf3f);

    // 将两个网格存储在同一个ply中
    mesh1.addPart(mesh2);
    std::filesystem::path outFilePath = "transform_testy.ply";
    auto saveRes = MR::MeshSave::toPly(mesh1, outFilePath);
    return 0;
}