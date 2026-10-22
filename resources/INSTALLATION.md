# GRKit

For RICOH GR III and GR III HDF firmware 2.10 only.

GRKit has not been tested on a GR III HDF camera.

This Soft edition is experimental and has not been tested on a camera. Grain uses Ricoh’s engine with a moment-derived seed sampled during processing, not the exact shutter timestamp. Seeds are not stored in photo metadata. In Soft > Texture & Light, select Grain: Off or a percentage from 10% to 100% (native amplitude). Native Weak is 25%, Medium is 50%, Strong is 100%. The area outside 4:3 is solid black while composing and in the ADJ framing preview. Playback is unchanged.

## Install

1. Turn the camera off.
2. Copy the contents of `SD_ROOT` to the root of the SD card.
3. Insert the card. Hold MENU while you turn the camera on. Wait until the camera turns itself off.
4. Turn the camera on normally.

Step 3 turns on the camera's SD scripts. The camera shows Mode Set Command, then Complete, and then turns itself off.

![The Mode Set Command screen shows Script - Enable and Complete.](images/modeset.png)

If the SD scripts are already on, skip step 3.
After the installation, you can remove `00078350.588` and `DEVELOP.MOD` from the card root. This step is optional.

## Use

Open ADJ and select the sixth entry, the Soft sliders icon. GRKit loads a few seconds after power-on, so the entry can appear late.

![The GRKit page lists the modules and shows the current effect.](images/grkit-root.png)

Photo: [Snow in Akihabara](https://www.flickr.com/photos/cafetricot/51803682755/) by nagi usano, [CC BY 2.0](https://creativecommons.org/licenses/by/2.0/).

- **Image Control:** highlight an effect to preview it. Press OK to keep it. Press Fn to adjust the Standard parameters of the effect. Each effect keeps its own adjustments, and they do not change the camera's Image Control settings.
- **Framing:** select XPan, 16:9, or 4:3. The viewfinder hides the area outside 4:3 with solid black; other ratios dim it. Framing works only with the native 3:2 aspect ratio.
- **Guides:** select a guide style. The lines show only in the viewfinder.
- **Date Stamp:** select a date style. Photos get the date at the bottom right.
- **Grain:** select Off or a percentage. Only photos get the grain.

Open About (the Info icon) to see the version.

The main menu groups controls under Framing, Tone & Colour, and Texture & Light. Select a colour under Tone & Colour > Colour, then Saturation, Hue or Brightness. The title shows your current group or colour. Select Back, or press MENU or left, to return one level. GRKit keeps your settings after power-off.

## Remove or recover

To remove GRKit, turn the camera off. Then delete the `SCRIPT` folder from the card.

The SD script setting stays on. To turn it off:

1. On https://grkit.app, download the files that turn SD scripts off for your camera. The button is below the installation steps.
2. Copy the contents of `SD_ROOT` to the root of the SD card.
3. Insert the card. Hold MENU while you turn the camera on. Wait until the camera turns itself off.
4. Remove `00078350.588` and `DEVELOP.MOD` from the card root.

If the camera stops during startup, remove the battery and the card. Then start the camera without the card. If it still does not start, see the [recovery guide](https://github.com/ahonn/grkit/blob/main/resources/RECOVERY.md).

GRKit is not affiliated with Ricoh. Source code and licenses: https://github.com/ahonn/grkit
