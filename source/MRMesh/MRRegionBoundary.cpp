#include "MRRegionBoundary.h"
#include "MREdgePaths.h"
#include "MRMeshTopology.h"
#include "MRRingIterator.h"
#include "MRBitSet.h"
#include "MRphmap.h"
#include "MRBitSetParallelFor.h"
#include "MRTimer.h"
// test
#include "MRMesh.h"
#include "MRUVSphere.h"
#include "MRGTest.h"

namespace MR
{

EdgeLoop trackRegionBoundaryLoop( const MeshTopology & topology, EdgeId e0, const FaceBitSet * region )
{
    return trackLeftBoundaryLoop( topology, e0, region );
}

EdgeLoop trackBoundaryLoop( const MeshTopology& topology, EdgeId e0, const FaceBitSet* region /*= nullptr */, bool left )
{
    MR_TIMER
    std::function<bool( EdgeId )> isLeftBdEdge;
    std::function<EdgeId( EdgeId )> next;
    if ( left )
    {
        isLeftBdEdge = [&]( EdgeId e ) { return topology.isLeftBdEdge( e, region ); };
        next = [&] ( EdgeId e ) { return topology.next( e ); };
    }
    else
    {
        isLeftBdEdge = [&] ( EdgeId e ) { return topology.isLeftBdEdge( e.sym(), region ); };
        next = [&] ( EdgeId e ) { return topology.prev( e ); };
    }

    EdgeLoop res;

    auto e = e0;
    do
    {
        assert( isLeftBdEdge( e ) );
        res.push_back( e );

        for ( e = next( e.sym() );
             !isLeftBdEdge( e );
             e = next( e ) )
        {
            assert( !isLeftBdEdge( e.sym() ) );
        }
    } while ( e != e0 );

    return res;
}

EdgeLoop trackLeftBoundaryLoop( const MeshTopology& topology, EdgeId e0, const FaceBitSet* region /*= nullptr */ )
{
    return trackBoundaryLoop( topology, e0, region, true );
}

EdgeLoop trackRightBoundaryLoop( const MeshTopology& topology, EdgeId e0, const FaceBitSet* region /*= nullptr */ )
{
    return trackBoundaryLoop( topology, e0, region, false );
}

std::vector<EdgeLoop> findRegionBoundary( const MeshTopology & topology, const FaceBitSet * region )
{
    return findLeftBoundary( topology, region );
}

std::vector<EdgeLoop> findRegionBoundary( const MeshTopology& topology, const FaceBitSet* region /*= nullptr */, bool left )
{
    MR_TIMER

    std::vector<EdgeLoop> res;
    phmap::flat_hash_set<EdgeId> reportedBdEdges;

    std::function<bool( EdgeId )> insert;
    std::function<EdgeLoop( EdgeId )> track;
    if ( left )
    {
        insert = [&] ( EdgeId e ) { return reportedBdEdges.insert( e ).second; };
        track = [&] ( EdgeId e ) { return trackLeftBoundaryLoop( topology, e, region ); };
    }
    else
    {
        insert = [&] ( EdgeId e ) { return reportedBdEdges.insert( e.sym() ).second; };
        track = [&] ( EdgeId e ) { return trackRightBoundaryLoop( topology, e.sym(), region ); };
    }

    for ( auto f : topology.getFaceIds( region ) )
    {
        for ( auto e : leftRing( topology, f ) )
        {
            if ( !topology.isLeftBdEdge( e, region ) )
                continue;
            if ( !insert( e ) )
                continue;
            auto loop = track( e );
            for ( int i = 1; i < loop.size(); ++i )
            {
                [[maybe_unused]] bool inserted = reportedBdEdges.insert( loop[i] ).second;
                assert( inserted );
            }
            res.push_back( std::move( loop ) );
        }
    }

    return res;

}

std::vector<EdgeLoop> findLeftBoundary( const MeshTopology& topology, const FaceBitSet* region /*= nullptr */ )
{
    return findRegionBoundary( topology, region, true );
}

std::vector<EdgeLoop> findRightBoundary( const MeshTopology& topology, const FaceBitSet* region /*= nullptr */ )
{
    return findRegionBoundary( topology, region, false );
}

std::vector<EdgeLoop> findLeftBoundaryInsideMesh( const MeshTopology & topology, const FaceBitSet & region )
{
    MR_TIMER

    std::vector<EdgeLoop> res;
    phmap::flat_hash_set<EdgeId> reportedBdEdges;

    for ( auto f : region )
    {
        for ( auto e : leftRing( topology, f ) )
        {
            if ( !topology.right( e ) || !topology.isLeftBdEdge( e, &region ) )
                continue;
            if ( reportedBdEdges.count( e ) )
                continue;
            auto loop = trackLeftBoundaryLoop( topology, e, &region );
            int holeEdgeIdx = -1;
            for ( int i = 0; i < loop.size(); ++i )
            {
                if ( topology.right( loop[i] ) )
                {
                    [[maybe_unused]] bool inserted = reportedBdEdges.insert( loop[i] ).second;
                    assert( inserted );
                }
                else if ( holeEdgeIdx < 0 )
                    holeEdgeIdx = i;
            }
            if ( holeEdgeIdx >= 0 )
            {
                // found loop goes partially along hole boundary,
                // rotate it to put first hole edge in the end
                assert( holeEdgeIdx + 1 < loop.size() ); // there shall be some not-hole edges
                std::rotate( loop.begin(), loop.begin() + holeEdgeIdx + 1, loop.end() );
                for ( int i = 0; i < loop.size(); )
                {
                    // skip hole edges
                    for ( ; i < loop.size() && !topology.right( loop[i] ); ++i )
                        {}
                    const auto beg = loop.begin() + i;
                    // skip not-hole edges
                    for ( ; i < loop.size() && topology.right( loop[i] ); ++i )
                        {}
                    if ( beg < loop.begin() + i )
                        res.emplace_back( beg, loop.begin() + i );
                }
            }
            else
            {
                // found loop is entirely inside the mesh, and have all valid right faces
                res.push_back( std::move( loop ) );
            }
        }
    }

    return res;
}

UndirectedEdgeBitSet findRegionBoundaryUndirectedEdgesInsideMesh( const MeshTopology& topology, const FaceBitSet & region )
{
    MR_TIMER
    UndirectedEdgeBitSet res( topology.undirectedEdgeSize() );
    BitSetParallelForAll( res, [&]( UndirectedEdgeId ue )
    {
        const auto l = topology.left( ue );
        if ( !l )
            return;
        const auto r = topology.right( ue );
        if ( !r )
            return;
        if ( region.test( l ) != region.test( r ) )
            res.set( ue );
    } );
    return res;
}

FaceBitSet findRegionOuterFaces( const MeshTopology& topology, const FaceBitSet& region )
{
    MR_TIMER;
    FaceBitSet res( topology.faceSize() );
    auto borders = findLeftBoundary( topology, region );
    for ( const auto& loop : borders )
    {
        for ( auto e : loop )
        {
            auto f = topology.right( e );
            if ( f )
                res.set( f );
        }
    }
    return res;
}

VertBitSet getIncidentVerts( const MeshTopology & topology, const FaceBitSet & faces )
{
    MR_TIMER
    VertBitSet res = topology.getValidVerts();
    BitSetParallelFor( res, [&]( VertId v )
    {
        bool incident = false;
        for ( auto e : orgRing( topology, v ) )
        {
            auto f = topology.left( e );
            if ( f.valid() && faces.test( f ) )
            {
                incident = true;
                break;
            }
        }
        if ( !incident )
            res.reset( v );
    } );
    return res;
}

VertBitSet getInnerVerts( const MeshTopology & topology, const FaceBitSet & faces )
{
    MR_TIMER
    VertBitSet res = topology.getValidVerts();
    BitSetParallelFor( res, [&]( VertId v )
    {
        for ( auto e : orgRing( topology, v ) )
        {
            auto f = topology.left( e );
            if ( !f.valid() || !faces.test( f ) )
            {
                res.reset( v );
                break;
            }
        }
    } );
    return res;
}

const VertBitSet & getIncidentVerts( const MeshTopology & topology, const FaceBitSet * faces, VertBitSet & store )
{
    MR_TIMER

    if ( !faces )
        return topology.getValidVerts();

    store = getIncidentVerts( topology, *faces );
    return store;
}

VertBitSet getBoundaryVerts( const MeshTopology & topology, const FaceBitSet * region )
{
    MR_TIMER

    VertBitSet store;
    const VertBitSet & regionVertices = getIncidentVerts( topology, region, store );

    VertBitSet bdVerts( regionVertices.size() );
    BitSetParallelFor( regionVertices, [&]( VertId v )
    {
        if ( topology.isBdVertex( v, region ) )
            bdVerts.set( v );
    } );
    return bdVerts;
}

EdgeBitSet getRegionEdges( const MeshTopology& topology, const FaceBitSet& faces )
{
    MR_TIMER
    EdgeBitSet res( topology.edgeSize() );
    for ( auto f : faces )
    {
        for ( auto e : leftRing( topology, f ) )
        {
            assert( e.valid() );
            res.set( e );
        }
    }
    return res;
}

UndirectedEdgeBitSet getIncidentEdges( const MeshTopology& topology, const FaceBitSet& faces )
{
    MR_TIMER
    UndirectedEdgeBitSet res( topology.undirectedEdgeSize() );
    for ( auto f : faces )
    {
        for ( auto e : leftRing( topology, f ) )
        {
            res.set( e.undirected() );
        }
    }
    return res;
}

UndirectedEdgeBitSet getInnerEdges( const MeshTopology & topology, const VertBitSet& verts )
{
    MR_TIMER
    UndirectedEdgeBitSet res( topology.undirectedEdgeSize() );
    for ( auto v : verts )
    {
        for ( auto e : orgRing( topology, v ) )
        {
            if ( verts.test( topology.dest( e ) ) )
                res.set( e.undirected() );
        }
    }
    return res;
}

UndirectedEdgeBitSet getInnerEdges( const MeshTopology & topology, const FaceBitSet& region )
{
    MR_TIMER
    UndirectedEdgeBitSet res( topology.undirectedEdgeSize() );

    for ( auto f0 : region )
    { 
        EdgeId e[3];
        topology.getTriEdges( f0, e );
        for ( int i = 0; i < 3; ++i )
        {
            assert( topology.left( e[i] ) == f0 );
            FaceId f1 = topology.right( e[i] );
            if ( f0 < f1 && region.test( f1 ) )
                res.set( e[i].undirected() );
        }
    }
    return res;
}

FaceBitSet getIncidentFaces_( const MeshTopology & topology, const VertBitSet & verts )
{
    MR_TIMER
    FaceBitSet res( topology.faceSize() );
    for ( auto v : verts )
    {
        for ( auto e : orgRing( topology, v ) )
        {
            auto f = topology.left( e );
            if ( f.valid() )
                res.set( f );
        }
    } 
    return res;
}

static FaceBitSet getInnerFaces_( const MeshTopology & topology, const VertBitSet & verts )
{
    MR_TIMER
    FaceBitSet res = getIncidentFaces_( topology, verts );
    for ( auto f : res )
    {
        for ( auto e : leftRing( topology, f ) )
        {
            auto v = topology.org( e );
            if ( v.valid() && !verts.test( v ) )
            {
                res.reset( f );
                break;
            }
        }
    } 
    return res;
}

FaceBitSet getIncidentFaces( const MeshTopology & topology, const VertBitSet & verts )
{
    MR_TIMER

    if ( 3 * verts.count() <= 2 * topology.numValidVerts() )
        return getIncidentFaces_( topology, verts );

    return topology.getValidFaces() - getInnerFaces_( topology, topology.getValidVerts() - verts );
}

FaceBitSet getInnerFaces( const MeshTopology & topology, const VertBitSet & verts )
{
    MR_TIMER

    if ( 3 * verts.count() <= topology.numValidVerts() )
        return getInnerFaces_( topology, verts );

    return topology.getValidFaces() - getIncidentFaces_( topology, topology.getValidVerts() - verts );
}

static VertBitSet getIncidentVerts_( const MeshTopology & topology, const UndirectedEdgeBitSet & edges )
{
    MR_TIMER
    VertBitSet res( topology.vertSize() );
    for ( auto ue : edges )
    {
        if ( auto v = topology.org( ue ) )
            res.set( v );
        if ( auto v = topology.dest( ue ) )
            res.set( v );
    } 
    return res;
}

FaceBitSet getIncidentFaces( const MeshTopology & topology, const UndirectedEdgeBitSet & edges )
{
    MR_TIMER
    FaceBitSet res( topology.faceSize() );
    for ( auto ue : edges )
    {
        if ( auto l = topology.left( ue ) )
            res.set( l );
        if ( auto r = topology.right( ue ) )
            res.set( r );
    } 
    return res;
}

static VertBitSet getInnerVerts_( const MeshTopology & topology, const UndirectedEdgeBitSet & edges )
{
    MR_TIMER
    VertBitSet res = getIncidentVerts_( topology, edges );
    BitSetParallelFor( res, [&]( VertId v )
    {
        for ( auto e : orgRing( topology, v ) )
        {
            if ( !edges.test( e.undirected() ) )
            {
                res.reset( v );
                break;
            }
        }
    } );
    return res;
}

VertBitSet getIncidentVerts( const MeshTopology & topology, const UndirectedEdgeBitSet & edges )
{
    MR_TIMER

    //TODO: if there are many set edges, find inner vertices of the complement edges and invert
    return getIncidentVerts_( topology, edges );
}

const VertBitSet & getIncidentVerts( const MeshTopology & topology, const UndirectedEdgeBitSet * edges, VertBitSet & store )
{
    MR_TIMER

    if ( !edges )
        return topology.getValidVerts();

    store = getIncidentVerts( topology, *edges );
    return store;
}

VertBitSet getInnerVerts( const MeshTopology & topology, const UndirectedEdgeBitSet & edges )
{
    MR_TIMER

    //TODO: if there are many set edges, find incident vertices of the complement edges and invert
    return getInnerVerts_( topology, edges );
}

TEST(MRMesh, findLeftBoundary) 
{
    Mesh sphere = makeUVSphere( 1, 8, 8 );
    FaceBitSet faces;
    faces.autoResizeSet( 0_f );
    auto paths = findLeftBoundary( sphere.topology, faces );
    EXPECT_EQ( paths.size(), 1 );
    for ( const auto & path : paths )
    {
        for ( auto e : path )
        {
            EXPECT_EQ( sphere.topology.left( e ), 0_f );
            EXPECT_NE( sphere.topology.right( e ), 0_f );
        }
    }
}

} //namespace MR
