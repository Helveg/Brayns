/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
 * All rights reserved. Do not distribute without permission.
 * Responsible Author: Cyrille Favreau <cyrille.favreau@epfl.ch>
 *
 * This file is part of BRayns
 */

#include "Primitive.h"

namespace brayns
{

Primitive::Primitive(const size_t materialId)
    : _materialId(materialId)
{
    _geometryType = GT_UNDEFINED;
}

}
