import re,struct,zlib,binascii,base64,json
from pathlib import Path
import argparse
p=argparse.ArgumentParser(description='Export the actual camera glyphs from a GRKit build. Run from the repository root.')
p.add_argument('build',type=Path)
aargs=p.parse_args()
s=(aargs.build/'objects/module_payload.h').read_text()
def array(name):return bytes(int(x) for x in re.search(r'static const u8 '+name+r'\[\][^=]*= \{([^}]+)\}',s)[1].split(',') if x.strip())
def png(rgba):
 def c(k,v):return struct.pack('>I',len(v))+k+v+struct.pack('>I',binascii.crc32(k+v)&0xffffffff)
 return b'\x89PNG\r\n\x1a\n'+c(b'IHDR',struct.pack('>IIBBBBB',40,40,8,6,0,0,0))+c(b'IDAT',zlib.compress(b''.join(b'\0'+rgba[y*160:(y+1)*160] for y in range(40))))+c(b'IEND',b'')
names=['Looks','Frame','Tone','Texture','Crop','Guides','Date Stamp','Appearance','Grain','Bloom','Halation','Black Floor','Split Toning','Colour','Saturation','Hue','Brightness','Highlight Roll-off']
a=array('module_pixels');assert len(a)==18*6400
out=Path('web/public/menu-icons');out.mkdir(exist_ok=True)
images=[]
for i,n in enumerate(names):
 blob=png(a[i*6400:(i+1)*6400]);key=n.lower().replace(' ','-');out.joinpath(key+'.png').write_bytes(blob);images.append((n,base64.b64encode(blob).decode()))
soft=png(array('plugins_pixels'));out.joinpath('soft.png').write_bytes(soft)
for key,blob in [('grain-strength',soft),('grain-boost',png(a[8*6400:9*6400])),('grain-pattern',png(a[3*6400:4*6400]))]:
 out.joinpath(key+'.png').write_bytes(blob);images.append((key.replace('-',' ').title(),base64.b64encode(blob).decode()))
svg=['<svg xmlns="http://www.w3.org/2000/svg" width="920" height="910" viewBox="0 0 920 910"><rect width="920" height="910" fill="#171a1b"/><text x="46" y="56" fill="#eeebe0" font-family="sans-serif" font-size="24">Soft · camera symbols</text><text x="46" y="86" fill="#a1a7a7" font-family="sans-serif" font-size="14">Custom artwork from the camera build · shown at 2× size</text>']
for i,(n,b) in enumerate(images):
 x=44+(i%4)*220;y=120+(i//4)*125
 svg.append(f'<image x="{x}" y="{y}" width="80" height="80" href="data:image/png;base64,{b}"/><text x="{x+92}" y="{y+45}" fill="#eeebe0" font-family="sans-serif" font-size="14">{n}</text>')
svg.append('</svg>');Path('../outputs/Soft-camera-icons.svg').write_text(''.join(svg))
