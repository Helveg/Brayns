/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
 * All rights reserved. Do not distribute without permission.
 * Responsible Author: Cyrille Favreau <cyrille.favreau@epfl.ch>
 *
 * This file is part of BRayns
 */

// ospray
#include "ExtendedCylinders.h"
#include "ospray/common/Data.h"
#include "ospray/common/Model.h"
// ispc-generated files
#include "ExtendedCylinders_ispc.h"

namespace brayns {

ExtendedCylinders::ExtendedCylinders()
{
    this->ispcEquivalent = ispc::ExtendedCylinders_create(this);
}

void ExtendedCylinders::finalize(ospray::Model *model)
{
    radius            = getParam1f("radius",0.01f);
    materialID        = getParam1i("materialID",0);
    bytesPerCylinder  = getParam1i("bytes_per_cylinder",8*sizeof(float));
    offset_v0         = getParam1i("offset_v0",0);
    offset_v1         = getParam1i("offset_v1",3*sizeof(float));
    offset_radius     = getParam1i("offset_radius",6*sizeof(float));
    offset_frame      = getParam1i("offset_frame",7*sizeof(float));
    offset_materialID = getParam1i("offset_materialID",-1);
    data              = getParamData("extendedcylinders",nullptr);

    if (data.ptr == nullptr || bytesPerCylinder == 0)
        throw std::runtime_error("#ospray:geometry/extendedcylinders: " \
                                 "no 'extendedcylinders' data specified");
    numExtendedCylinders = data->numBytes / bytesPerCylinder;
    ispc::ExtendedCylindersGeometry_set(
                getIE(),
                model->getIE(),
                data->data,
                numExtendedCylinders,
                bytesPerCylinder,
                radius,
                materialID,
                offset_v0,
                offset_v1,
                offset_radius,
                offset_frame,
                offset_materialID);
}


OSP_REGISTER_GEOMETRY(ExtendedCylinders,extendedcylinders);

} // ::brayns
