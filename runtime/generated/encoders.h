/* Generated from crates/grkit-core/src/plugins.rs. Do not edit: change the
   registry there, then run GRKIT_UPDATE_GENERATED=1 cargo test -p grkit-core
   --test generated. */
/* The picture.encode handlers of all plugins, in call order. */
#ifndef GRKIT_GENERATED_ENCODERS_H
#define GRKIT_GENERATED_ENCODERS_H

#include "../plugins/image-control/selective.c"
#include "../plugins/date-stamp/stamp.c"
#include "../plugins/bloom/encode.c"
#include "../plugins/halation/encode.c"
#include "../plugins/black-floor/encode.c"
#include "../plugins/split-toning/encode.c"
#include "../plugins/colour-range/encode.c"
#include "../plugins/highlight-rolloff/encode.c"

#define PLUGIN_ENCODERS \
    {PLUGIN_IMAGE_CONTROL,selective_wanted,selective_encode}, \
    {PLUGIN_BLACK_FLOOR,black_floor_wanted,black_floor_encode}, \
    {PLUGIN_COLOUR_RANGE,colour_range_wanted,colour_range_encode}, \
    {PLUGIN_SPLIT_TONING,split_toning_wanted,split_toning_encode}, \
    {PLUGIN_HIGHLIGHT_ROLLOFF,highlight_rolloff_wanted,highlight_rolloff_encode}, \
    {PLUGIN_BLOOM,bloom_wanted,bloom_encode}, \
    {PLUGIN_HALATION,halation_wanted,halation_encode}, \
    {PLUGIN_DATE_STAMP,stamp_wanted,stamp_encode},

#endif
