/*
 * This object is MeshPartPose's constructor and destructor. Its ??_G is dropped: the vtable's ??_E
 * slot is a weak external that binds to the real ??_E (MeshPartPose_VectorDeletingDtor, defined where BsFile
 * does new[] / delete[]).
 */
#include "sdw_types.h"

#define SDW_MEMBERS_MeshPartPose MeshPartPose();
#include "sdw_classes.h"

MeshPartPose::MeshPartPose()
{
    rot[0] = rot[1] = rot[2] = 0.0f;
    pos[0] = pos[1] = pos[2] = 0.0f;
    scale[0] = scale[1] = scale[2] = 1.0f;
}

MeshPartPose::~MeshPartPose() {}
