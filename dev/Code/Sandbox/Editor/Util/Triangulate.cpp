/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/
// Original file Copyright Crytek GMBH or its affiliates, used under license.

#include <StdAfx.h>
#include "Triangulate.h"

// this file is essentially a wrapper for a portion of the MIT-licenced
// ConvexDecomposition library by John W. Ratcliff mailto:jratcliffscarab@gmail.com.
// it contains no code from that library, it just provides it with the required types, then includes the
// portion we need.

static const float TRIANGULATION_EPSILON = 0.0000000001f;

#define MEMALLOC_MALLOC malloc
#define MEMALLOC_FREE free

namespace TriInternal
{
    class TVec;
    typedef double NxF64;
    typedef float NxF32;
    typedef unsigned char NxU8;
    typedef unsigned int NxU32;
    typedef int NxI32;
    typedef unsigned int TU32;

    typedef std::vector< TVec >  TVecVector;
    typedef std::vector< NxU32 >  TU32Vector;
    #include "Contrib/NvFloatMath.inl"
}

#undef MEMALLOC_MALLOC
#undef MEMALLOC_FREE

namespace Triangulator
{
    // given the contour of a triangle, triangulate it, and return the result
    // as a set of triangles
    // return false if you fail to triangulate it.
    bool Triangulate(const VectorOfVectors& contour, VectorOfVectors& result)
    {
        TriInternal::CTriangulator tri;
        for (auto pt : contour)
        {
            tri.addPoint(pt.x, pt.y, pt.z);
        }
        TriInternal::NxU32 tricount = 0;
        TriInternal::NxU32* indices = tri.triangulate(tricount, TRIANGULATION_EPSILON);
        if (!indices)
        {
            return false;
        }

        for (TriInternal::NxU32 currentIdx = 0; currentIdx < tricount * 3; ++currentIdx)
        {
            TriInternal::NxU32 indexValue = *indices++;
            result.push_back(contour[indexValue]);
        }

        return result.size() > 2;
    }
}