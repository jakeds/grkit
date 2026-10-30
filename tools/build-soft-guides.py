#!/usr/bin/env python3
"""Build the English Soft installation guides (reportlab required)."""
import argparse
from pathlib import Path
from reportlab.lib import colors
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.lib.enums import TA_LEFT
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer, PageBreak
from reportlab.lib.pagesizes import A4

parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--out',type=Path,required=True)
args=parser.parse_args()
styles=getSampleStyleSheet()
styles.add(ParagraphStyle(name='SoftTitle',fontName='Helvetica',fontSize=32,leading=38,spaceAfter=12))
styles['BodyText'].fontSize=10;styles['BodyText'].leading=15;styles['BodyText'].spaceAfter=10
styles['Heading2'].fontSize=16;styles['Heading2'].leading=21;styles['Heading2'].spaceBefore=17;styles['Heading2'].spaceAfter=10
cameras=[('gr3','GR III / GR III HDF','2.10 / 2.20','00078350.588'),('gr3x','GR IIIx / GR IIIx HDF','1.60 / 1.70','00078490.609'),('gr4','GR IV / GR IV HDF / GR IV Monochrome','1.11 (IV / HDF); 1.00 (Monochrome)','00078560.636')]
for key,name,firmware,modeset in cameras:
 path=args.out/key/'en.pdf';path.parent.mkdir(parents=True,exist_ok=True)
 story=[]
 def p(t,style='BodyText'):story.append(Paragraph(t,styles[style]))
 p('Soft','SoftTitle');p(name);p('Supported firmware: '+firmware)
 p('First installation','Heading2')
 steps=[
 'Back up your photos. Turn the camera off and remove its SD card.',
 'Unzip your Soft kit. Copy everything inside <b>SD_ROOT</b> to the <b>top level</b> of the card, alongside DCIM. Do not copy the SD_ROOT folder itself.',
 f'At the card root you should see <b>SCRIPT</b>, <b>{modeset}</b> and <b>DEVELOP.MOD</b>. Use the files for your camera model.',
 'Insert the card. <b>Hold MENU while pressing Power.</b> Keep holding MENU until the activation screen appears.',
 'Wait for <b>Mode Set Command</b>, <b>Script - Enable</b>, then <b>Complete</b>. Let the camera turn itself off.',
 'Power on normally, without holding MENU. Wait for the <b>Soft Loaded</b> indicator. The camera uses its stock settings until Soft has loaded.',
 'Open <b>MENU &gt; Customize &gt; Soft</b>. Soft also appears as an extra entry in ADJ.']
 for i,t in enumerate(steps,1):p(f'<b>{i}.</b> '+t)
 p('Enable scripts once','Heading2');p('The SD-script setting stays enabled when you change cards. Each card needs its own SCRIPT folder to load Soft. After activation you can remove the ModeSet file and DEVELOP.MOD. You do not repeat activation for an update.')
 p('If the camera does not start normally','Heading2');p('Turn it off, remove the SD card and start without it. Keep your backup. Removing SCRIPT stops Soft loading; it does not disable the persistent SD-script setting. The builder provides a separate script-disable package.')
 story.append(PageBreak())
 p('Using Soft','SoftTitle')
 p('Main menu and ADJ','Heading2');p('<b>MENU &gt; Customize &gt; Soft</b> contains the installed modules and their settings. In ADJ, Up/Down selects a row, <b>Right opens</b>, <b>Left goes back</b>, and <b>OK applies without closing ADJ</b>. Press ADJ to exit. Fn still opens the native detail settings where available.')
 p('Grain','Heading2')
 if key=='gr4':p('GR IV offers Strength and Size. The III-series Boost and Pattern controls are omitted. Size Default retains the native size selection.')
 else:p('Strength uses 5% steps. Size offers <b>Default / Auto by ISO / Small / Medium / Large</b>. Boost offers Default / 1.5x / 2x. Pattern offers Moment / Fixed seed. <b>Default size + Default boost + Moment pattern</b> retains the previous Soft native noise path. Return to those values to undo the new choices. Grain Off bypasses the overrides.')
 p('Grain affects recorded JPEGs; judge it in a saved image at a useful zoom. A fixed seed repeats the generator pattern, not necessarily the entire image. Physical-camera image quality and startup timing still need checking.')
 p('Tone, colour and date','Heading2');p('Tone contains Black lift and Highlight roll-off. Light contains Bloom and Halation. Split toning separates Style from Strength. Colour stores independent Saturation, Hue and Brightness for seven colours; Colour Off disables those edits. Date separates Format, Appearance and Position, including small mono white and black. New image effects start Off or Neutral.')
 p('Updating from Soft.15','Heading2');p('Back up the whole card. Merge the new SD_ROOT/SCRIPT onto the existing SCRIPT, replacing files with the same names. For this first migration, retain <b>SCRIPT/PLUGINS/STATE.TXT</b>. Soft reads it once and converts it into <b>GRKIT/STATE.TXT</b>. Do not restore the old STARTUP.TTL over the new one. After a successful start and the new state file exists, the old SCRIPT/PLUGINS folder can be removed.')
 p('Later updates','Heading2');p('Replace SCRIPT while retaining the top-level <b>GRKIT</b> folder, which holds settings and the effect library. Activation is not repeated. Formatting the card removes these files, so back up GRKIT first.')
 p('Soft is an SD-card runtime modification, not a replacement firmware image. Software and emulation checks do not replace testing on your camera. See the builder for the current supported firmware and recovery instructions.')
 SimpleDocTemplate(str(path),pagesize=A4,rightMargin=48,leftMargin=48,topMargin=44,bottomMargin=44,title='Soft installation - '+name,author='Soft').build(story)
 print(path)
