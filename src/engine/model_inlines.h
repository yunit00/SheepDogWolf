/* Shared inline definitions. Select helpers before inclusion, then undefine their selectors. Per-helper guards prevent
 * redefinition when a later include selects more helpers. */

#if defined(SDW_INLINE_FREE_MODEL_FIRSTBOX_MODEL) && !defined(SDW_INLINE_FREE_MODEL_FIRSTBOX_MODEL_DEFINED)
#define SDW_INLINE_FREE_MODEL_FIRSTBOX_MODEL_DEFINED
inline CollBox *Model_FirstBox(Model *model)
{
    ModelBoxList *list = model->boxes;
    if (list != 0)
        return list->boxes;
    return 0;
}
#endif
