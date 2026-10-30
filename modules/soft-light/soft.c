#include "../../runtime/soft-module/soft.h"
#include "../../runtime/soft-module/diffusion.h"
void soft_bloom(PictureSet *set,u32 value) {diffusion_apply(set,strength_code(value),0);}
void soft_halation(PictureSet *set,u32 value) {diffusion_apply(set,strength_code(value),1);}
