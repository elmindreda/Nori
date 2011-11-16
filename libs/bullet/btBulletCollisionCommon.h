/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it freely,
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifndef BULLET_COLLISION_COMMON_H
#define BULLET_COLLISION_COMMON_H

///Common headerfile includes for Bullet Collision Detection

///Bullet's btCollisionWorld and btCollisionObject definitions
#include "btCollisionWorld.h"
#include "btCollisionObject.h"

///Collision Shapes
#include "btBoxShape.h"
#include "btSphereShape.h"
#include "btCapsuleShape.h"
#include "btCylinderShape.h"
#include "btConeShape.h"
#include "btStaticPlaneShape.h"
#include "btConvexHullShape.h"
#include "btTriangleMesh.h"
#include "btConvexTriangleMeshShape.h"
#include "btBvhTriangleMeshShape.h"
#include "btScaledBvhTriangleMeshShape.h"
#include "btTriangleMeshShape.h"
#include "btTriangleIndexVertexArray.h"
#include "btCompoundShape.h"
#include "btTetrahedronShape.h"
#include "btEmptyShape.h"
#include "btMultiSphereShape.h"
#include "btUniformScalingShape.h"

///Narrowphase Collision Detector
#include "btSphereSphereCollisionAlgorithm.h"

//btSphereBoxCollisionAlgorithm is broken, use gjk for now
//#include "btSphereBoxCollisionAlgorithm.h"
#include "btDefaultCollisionConfiguration.h"

///Dispatching and generation of collision pairs (broadphase)
#include "btCollisionDispatcher.h"
#include "btSimpleBroadphase.h"
#include "btAxisSweep3.h"
#include "btMultiSapBroadphase.h"
#include "btDbvtBroadphase.h"

///Math library & Utils
#include "btQuaternion.h"
#include "btTransform.h"
#include "btDefaultMotionState.h"
#include "btQuickprof.h"
#include "btIDebugDraw.h"
#include "btSerializer.h"


#endif //BULLET_COLLISION_COMMON_H

