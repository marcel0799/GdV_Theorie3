//=============================================================================
//
//   Exercise code for the lecture "Computer Graphics"
//     by Prof. Mario Botsch, TU Dortmund
//
//   Copyright (C)  Computer Graphics Group, TU Dortmund
//
//=============================================================================

#include "Mesh.h"
#include <pmp/visualization/PhongShader.h>
#include <cfloat>

//=============================================================================

SubdivisionMesh::SubdivisionMesh() : SurfaceMeshGL() {}

//-----------------------------------------------------------------------------

void SubdivisionMesh::subdivide()
{
    using namespace pmp;

    // reserve memory
    int nv = n_vertices();
    int ne = n_edges();
    int nf = n_faces();
    reserve(nv + ne + nf, 2 * ne + 4 * nf, 4 * nf);

    // get properties
    auto points = vertex_property<Point>("v:point");
    auto vpoint = add_vertex_property<Point>("catmull:vpoint", Point(0));
    auto epoint = add_edge_property<Point>("catmull:epoint", Point(0));
    auto fpoint = add_face_property<Point>("catmull:fpoint", Point(0));

    /** \todo Implement the generalized version of Catmull-Clark subdivision
      *   that can handle arbitrary polygonal meshes (not just quad meshes).          \n
      *   You have to compute
      *   - a new point to be inserted in each face `f`, to be stored in `fpoint[f]`,
      *   - a new point for each edge `e`, to be stored in `epoint[e]`,
      *   - a new position for every old vertex `v`, to be stored in `vpoint[v]`.
      *
      *   Note that special rules exist for boundary edges and boundary vertices.
      *   You can test whether an edge `e` or a vertex `v` is on the boundary by
      *   `is_boundary(e)` and `is_boundary(v)`.                                      \n
      *   The actual mesh refinement, i.e., the edge and face splitting, is
      *   given at the bottom.
      *
      *   Hints:
      *   - We use the pmp mesh structure, for a short tutorial see: "http://www.pmp-library.org/tutorial.html"
      *   - A `Vertex v` is just a kind of index referencing the 3D position accessed by `points[v]`
      *   - You get the two vertices of an edge `e` by `vertex(e,0)` and `vertex(e,1)`
      *   - You get the two faces of an edge `e` by `face(e,0)` and `face(e,1)`
      *   - To compute how many vertices are direct neighbors of a Vertex `v`, use `valence(v)`
      *   - To compute how many vertices are compose a `Face f`, use `valence(f)`
      *   - You can use special range based loops:
      *         for(auto v : vertices())    -->  loop through all vertices
      *         for(auto f : faces())       -->  loop through all faces
      *         for(auto e : edges())       -->  loop through all edges
      *         for(auto h : halfedges())   -->  loop through all halfedges
      *         for(auto vv : vertices(v))  -->  loop through all vertices in one ring neighborhood of `Vertex v`
      *         for(auto vf : vertices(f))  -->  loop through all vertices of `Face f`
      *         for(auto fv : faces(v))     -->  loop through all faces of `Vertex v`
      *         for(auto hv : halfedges(v)) -->  loop through all outgoing halfedges of `Vertex v`
      */

    // i) New face vertices
    //schleife ueber alle faces, weil wir alle mittelpunkte bestimmen wollen
    for(auto f: faces()) {
        vec3 p(0,0,0); //ein neuer Punkt den wir ausrechnen wollen mit 0,0,0 initializiert
        double ctr = 0; //counter variable zum zaehlen
        for(auto v : vertices(f)) { //ueber alle vetices vom face f
            p += points[v]; //alle punkte aufsummieren, entspricht der summe aus der formel
            ctr ++;
        } 
        p /= ctr;
        fpoint[f] = p;
    }

    // ii) new edge vertices
    // die neuen kanten oder so
    // wichtig ist, ist es eine innere kante oder eine rand kante, dass bestimmen wir ueber catmull clark halbkanten dings
    for(auto e : edges()) {
        vec3 p(0,0,0); //wieder der punkt den wir berechnen wollen
        //ueber vertex(e,0) und vertex(e,1) kriegen wir den anfangs und endpunkt der kante
        //durch das teilen können wir dann den mittelpunkt bestimmen
        // wie teilen ergibt sich aus dem subdivision zeugs, also welches netz wir haben
        Vertex v0 = vertex(e,0); //anfangs knoten der kante
        Vertex v1 = vertex(e,1); //endpunkt der Kante
        p += points[v0] + points[v1];
        if(is_boundary(e)) { //wenn es eine randkante ist
            epoint[e] = 0.5 * p; //mittelpunkt berechnen oder so
        } else {
            Face f0 = face(e,0); //kein plan
            Face f1 = face(e,1);
            epoint[e] = 0.25 * (p + fpoint[f0] + fpoint [f1]);
        }
    }

    // 3) update old vertex positions
    for(auto v : vertices()) {
        vec3 p(0,0,0);
        if(is_boundary(v)) {
            for(auto vv : vertices(v)) {
                if(is_boundary(vv)) {
                    p += 0.125 * points[vv];
                }
            }
            p += 0.75 * points[v];
        } else  {
            //valenc of the vertex
            double k = valence(v);
            vec3 p(0,0,0);
            for(auto f : faces(v)) {
                p +=  (1.0 / (k*k)) * fpoint[f];
            }
            for(auto vv : vertices(v)) {
                p +=  (1.0 / (k*k)) * points[vv];    
            }
            p += ((k-2.0) / k) * points[v];
            vpoint[v] = p;
        }
    }


    // assign new positions to old vertices
    for (auto v : vertices())
    {
        points[v] = vpoint[v];
    }

    // split edges
    for (auto e : edges())
    {
        insert_vertex(e, epoint[e]);
    }

    // split faces
    for (auto f : faces())
    {
        Halfedge h0 = halfedge(f);
        insert_edge(h0, next_halfedge(next_halfedge(h0)));

        Halfedge h1 = next_halfedge(h0);
        insert_vertex(edge(h1), fpoint[f]);

        Halfedge h = next_halfedge(next_halfedge(next_halfedge(h1)));
        while (h != h0)
        {
            insert_edge(h1, h);
            h = next_halfedge(next_halfedge(next_halfedge(h1)));
        }
    }

    // clean-up properties
    remove_vertex_property(vpoint);
    remove_edge_property(epoint);
    remove_face_property(fpoint);

    // upload new mesh to GPU
    update_opengl_buffers();
}
//=============================================================================
