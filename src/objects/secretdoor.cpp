/* SecretDoor. Uses the generated class declaration. */
#define SDW_MEMBERS_ScnObject static void *operator new(uptr size);
#include "sdw_classes.h"

void SecretDoor::PostLoadInit()
{
    u16 *props = record;
}

void SecretDoor::Update() {}

sptr SecretDoor::HandleMessage(ScnObject *sender, u32 msgId, void *arg)
{
    return 0;
}

ScnObject *SecretDoor_Create(void *record)
{
    ScnLogic *obj = new SecretDoor;
    obj = (ScnLogic *)obj->Init(record);
    return obj;
}
