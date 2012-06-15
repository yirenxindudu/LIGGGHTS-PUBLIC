/* ----------------------------------------------------------------------
   LIGGGHTS - LAMMPS Improved for General Granular and Granular Heat
   Transfer Simulations

   LIGGGHTS is part of the CFDEMproject
   www.liggghts.com | www.cfdem.com

   Christoph Kloss, christoph.kloss@cfdem.com
   Copyright 2009-2012 JKU Linz
   Copyright 2012-     DCS Computing GmbH, Linz

   LIGGGHTS is based on LAMMPS
   LAMMPS - Large-scale Atomic/Molecular Massively Parallel Simulator
   http://lammps.sandia.gov, Sandia National Laboratories
   Steve Plimpton, sjplimp@sandia.gov

   This software is distributed under the GNU General Public License.

   See the README file in the top-level directory.
------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
   Contributing authors:
   Christoph Kloss (JKU Linz, DCS Computing GmbH, Linz)
   Philippe Seil (JKU Linz)
------------------------------------------------------------------------- */

#ifndef LMP_MULTI_NODE_MESH_H
#define LMP_MULTI_NODE_MESH_H

#include "abstract_mesh.h"
#include "domain.h"
#include "myvector.h"
#include "math_extra_liggghts.h"
#include "update.h"
#include "container.h"
#include "bounding_box.h"
#include "random_park.h"

namespace LAMMPS_NS
{
  template<int NUM_NODES>
  class MultiNodeMesh : public AbstractMesh
  {
      friend class FixMoveMesh;
      friend class MeshMover;

      public:

        void setMeshID(const char *_mesh_id);

        // scale mesh
        virtual void scale(double factor);

        // linear move w/ total and incremental displacement
        virtual void move(double *vecTotal, double *vecIncremental);

        // linear move w/ incremental displacement
        virtual void move(double *vecIncremental);

        // rotation w/ total and incremental displacement
        //   calls rotate(double *totalQuat,double *dQuat,double *displacement)
        void rotate(double totalAngle, double dAngle, double *axis, double *p);

        // rotation w/ incremental displacement
        //   calls rotate(double *dQuat,double *displacement)
        void rotate(double dAngle, double *axis, double *p);

        // initialize movement
        bool registerMove(bool _scale, bool _translate, bool _rotate);
        void unregisterMove(bool _scale, bool _translate, bool _rotate);

        // bbox stuff
        BoundingBox getGlobalBoundingBox() const;
        BoundingBox getElementBoundingBoxOnSubdomain(int const n);
        void updateGlobalBoundingBox();

        // inline access

        inline bool isMoving()
        { return nMove_ > 0; }

        inline bool isScaling()
        { return nScale_ > 0; }

        inline bool isTranslating()
        { return nTranslate_ > 0; }

        inline bool isRotating()
        { return nRotate_ > 0; }

        inline void node(int i,int j,double *node)
        { vectorCopy3D(node_(i)[j],node);}

        inline void center(int i,double *center)
        { vectorCopy3D(center_(i),center);}

        inline int numNodes()
        { return NUM_NODES; }

        inline char* mesh_id()
        { return mesh_id_; }

        // virtual functions for size
        // parallelism implemented in derived class
        virtual int sizeLocal() = 0;
        virtual int sizeGhost() = 0;
        virtual int sizeGlobal() = 0;

      protected:

        MultiNodeMesh(LAMMPS *lmp);
        virtual ~MultiNodeMesh();
        virtual void addElement(double **nodeToAdd);
        virtual void deleteElement(int n);
        bool nodesAreEqual(int iSurf, int iNode, int jSurf, int jNode);
        void extendToElem(int const nElem) const;

        // linear move of single element w/ incremental displacement
        virtual void moveElement(int i,double *vecIncremental);

        // rotation functions using quaternions
        
        virtual void rotate(double *totalQ, double *dQ,double *totalDispl, double *dDispl);
        virtual void rotate(double *dQ, double *dDispl);

        // mesh nodes
        MultiVectorContainer<double,NUM_NODES,3> node_;

        // original mesh node_ position, used for moving mesh
        MultiVectorContainer<double,NUM_NODES,3> *node_orig_;
        double** node_orig(int i) {return (*node_orig_)(i);}

        // mesh center
        VectorContainer<double,3> center_; 
        ScalarContainer<double> rBound_; 

        // global bounding box for mesh across all processors
        BoundingBox bbox_;

        // random generator
        RanPark *random_;

        // mesh ID - same as fix mesh ID
        char *mesh_id_;

      private:

        // flags stating how many move operations are performed on the mesh
        int nMove_;
        int nScale_,nTranslate_,nRotate_;

        // reset mesh nodes to original position
        // called via mesh move functions
        void resetNodesToOrig();

        // step when nodes have been reset the last time
        // only relevant for moving mesh
        int stepLastReset_;

        // extends a given bbox to include element number nElem
        void extendToElem(BoundingBox &box, int const nElem);
        void extendToElem(int const nElem);

        inline double*** nodePtr()
        { return node_.begin(); }
  };

  // *************************************
  #include "multi_node_mesh_I.h"
  // *************************************

} /* LAMMPS_NS */
#endif /* MULTINODEMESH_H_ */