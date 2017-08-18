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

#ifndef FIR_WEIGHTS_H
#define FIR_WEIGHTS_H

#include "FIR-Windows.h"

/* ####################################################################################################################
 */
template<class DataType>
static inline DataType abs    (const DataType& ths) { return (ths < 0 ? -ths : ths); }
template<class DataType>
static inline void     minmax (const DataType& ths, DataType& mn, DataType& mx) { mn = (mn > ths ? ths : mn); mx = (mx < ths ? ths : mx); }
template<class DataType>
static inline DataType minimum(const DataType& ths, const DataType& tht) { return (ths < tht ? ths : tht); }
template<class DataType>
static inline DataType maximum(const DataType& ths, const DataType& tht) { return (ths > tht ? ths : tht); }

#ifndef round
#define round(x)    ((x) >= 0) ? floor((x) + 0.5) : ceil((x) - 0.5)
#endif

/* ####################################################################################################################
 */

template<class T>
class FilterWeights
{
public:
    FilterWeights()
        : weights(nullptr)
    {
    }

    ~FilterWeights()
    {
        delete[] weights;
    }

public:
    // window-position
    int first, last;

    // do we encounter positive as well as negative weights
    bool hasNegativeWeights;

    /* weights, summing up to -(1 << 15),
     * means weights are given negative
     * that enables us to use signed short
     * multiplication while occupying 0x8000
     */
    T* weights;
};

/* ####################################################################################################################
 */

void               calculateFilterRange (unsigned int srcFactor, int& srcFirst, int& srcLast,
    unsigned int dstFactor, int  dstFirst, int  dstLast,
    double blurFactor, class IWindowFunction<double>* windowFunction);

template<typename T>
FilterWeights<T>* calculateFilterWeights(unsigned int srcFactor, int  srcFirst, int  srcLast,
    unsigned int dstFactor, int  dstFirst, int  dstLast, signed short int numRepetitions,
    double blurFactor, class IWindowFunction<double>* windowFunction,
        bool peaknorm, bool& plusminus);

#endif
