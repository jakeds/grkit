/* Generated from crates/grkit-core/src/plugins.rs. Do not edit: change the
   registry there, then run GRKIT_UPDATE_GENERATED=1 cargo test -p grkit-core
   --test generated. */
/* The plugins in menu order. */
#ifndef GRKIT_GENERATED_PLUGINS_H
#define GRKIT_GENERATED_PLUGINS_H

/* Plugin n is bit n of a selection (GRKIT_HOST_PLUGINS) and module n of the
   menu. */
#define PLUGIN_IMAGE_CONTROL 0U
#define PLUGIN_FRAMING 1U
#define PLUGIN_GUIDES 2U
#define PLUGIN_DATE_STAMP 3U
#define PLUGIN_GRAIN 4U
#define PLUGIN_BLOOM 5U
#define PLUGIN_HALATION 6U
#define PLUGIN_BLACK_FLOOR 7U
#define PLUGIN_SPLIT_TONING 8U
#define PLUGIN_COLOUR_RANGE 9U
#define PLUGIN_COLOUR_SATURATION 10U
#define PLUGIN_COLOUR_HUE 11U
#define PLUGIN_COLOUR_BRIGHTNESS 12U
#define PLUGIN_HIGHLIGHT_ROLLOFF 13U
#define PLUGIN_SOFT_SHORTCUT 14U
#define PLUGIN_COUNT 15U
/* The PLUGINS.TXT names in plugin order. */
#define PLUGIN_NAMES "image-control", "framing", "guides", "date-stamp", "grain", "bloom", "halation", "black-floor", "split-toning", "colour-range", "colour-saturation", "colour-hue", "colour-brightness", "highlight-rolloff", "soft-shortcut"
/* The plugins that show only with effects. Without effects, the installer and
   the menu treat them as not installed. */
#define PLUGIN_NEEDS_EFFECTS 0x1U
/* The artwork tiles of each plugin. They follow each other in plugin order and
   end below the GRKit tile. */
#define IMAGE_CONTROL_TILES 4U
#define FRAMING_TILES 1U
#define GUIDES_TILES 1U
#define DATE_STAMP_TILES 2U
#define GRAIN_TILES 1U
#define BLOOM_TILES 1U
#define HALATION_TILES 1U
#define BLACK_FLOOR_TILES 1U
#define SPLIT_TONING_TILES 1U
#define COLOUR_RANGE_TILES 1U
#define COLOUR_SATURATION_TILES 1U
#define COLOUR_HUE_TILES 1U
#define COLOUR_BRIGHTNESS_TILES 1U
#define HIGHLIGHT_ROLLOFF_TILES 1U
#define SOFT_SHORTCUT_TILES 0U
#define PLUGIN_TILES 18U
/* The largest value of each state key. The loader restores a larger value as
   0. */
#define IMAGE_CONTROL_EFFECT_MAX 65535U
#define FRAMING_RATIO_MAX 6U
#define GUIDES_STYLE_MAX 5U
#define DATE_STAMP_STYLE_MAX 8U
#define DATE_STAMP_LOOK_MAX 4U
#define GRAIN_LEVEL_MAX 52U
#define BLOOM_LEVEL_MAX 52U
#define HALATION_LEVEL_MAX 52U
#define BLACK_FLOOR_VALUE_MAX 52U
#define SPLIT_TONING_VALUE_MAX 16U
#define SPLIT_TONING_STRENGTH_MAX 21U
#define COLOUR_RANGE_VALUE_MAX 7U
#define COLOUR_SATURATION_VALUE_MAX 536870911U
#define COLOUR_SATURATION_UPPER_MAX 32767U
#define COLOUR_HUE_VALUE_MAX 536870911U
#define COLOUR_BRIGHTNESS_VALUE_MAX 536870911U
#define HIGHLIGHT_ROLLOFF_VALUE_MAX 52U
#define SOFT_SHORTCUT_VALUE_MAX 1U

#endif
