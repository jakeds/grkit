#ifndef SOFT_ADJ_SLOT_H
#define SOFT_ADJ_SLOT_H
/* Native assignments remain intact on disk. The first Image Control assignment
   is borrowed only while Soft is resident. Other assignments retain their IDs. */
typedef struct { u32 mask,soft; } SoftAdjSlots;
static SoftAdjSlots adj_slots(const u32 *kinds,u32 image_control,u32 movie) {
    SoftAdjSlots plan={0,5};u32 i;
    for(i=0;i<5;++i) {
        if(kinds[i]) plan.mask|=1U<<i;
        if(!movie && plan.soft==5 && kinds[i]==image_control) plan.soft=i;
    }
    return plan;
}
static u32 adj_slot_rank(SoftAdjSlots plan,u32 slot) {
    u32 i,n=0;for(i=0;i<slot && i<5;++i) if(plan.mask>>i&1U) ++n;return n;
}
/* Left at Soft's root selects the preceding enabled native function, wrapping.
   No other assignment means Left closes ADJ. Duplicate Image Control entries
   remain ordinary native entries except the first, which Soft borrows. */
static u32 adj_slot_previous(SoftAdjSlots plan) {
    u32 i;
    for(i=1;i<=5;++i) {u32 slot=plan.soft>=i ? plan.soft-i : plan.soft+5-i;if(slot!=plan.soft && (plan.mask>>slot&1U)) return slot;}
    return 5;
}
#endif
