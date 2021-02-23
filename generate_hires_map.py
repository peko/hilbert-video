from PIL import Image, ImageFilter
import sys
import os
from glob import glob
from math import log2, ceil, sqrt
from shutil import rmtree

start_z = 10
image_size = 256
quality = 85

map_dir = '_map'
try: rmtree(map_dir)
except: pass

# Гильберт
# Поиск ближайшего кводрата со стороной 2^n
# l = len(files)
n = 256

def d2xy(d): 
    t = d
    x = y = 0
    s = 1
    while s < n:
        rx = 1 & (t//2)
        ry = 1 & (t ^ rx)
        # rot(s, x, y, rx, ry)
        if ry == 0:
            if rx == 1:
                x = s-1 - x
                y = s-1 - y
            x, y = y, x
        x += s * rx
        y += s * ry
        t = t//4
        s = s<<1
    return (x,y)


def imop(n, size):
    print(n)
    try: img = Image.open(n).resize(size)
    except: img = Image.new("RGB", size)
    return img

def generate_base_zoom(map_dir, files, size):
    z = start_z
    for i, f in enumerate(files):
        x,y = d2xy(i)
        imop(f, size).save(f"{map_dir}/{z}-{x}-{y}.jpg",quality=quality)

def generate_zoom_out(map_dir, size):
    x = 0
    y = 0
    d = 0
    for z in range(start_z,1,-1):
        for x in range(0,256>>d,2):
            for y in range(0,256>>d,2):
                out = Image.new("RGB", size)
                size2 = (size[0]//2, size[1]//2) 
                im00 = imop(f"{map_dir}/{z}-{x+0}-{y+0}.jpg", size2)
                im10 = imop(f"{map_dir}/{z}-{x+1}-{y+0}.jpg", size2)
                im01 = imop(f"{map_dir}/{z}-{x+0}-{y+1}.jpg", size2)
                im11 = imop(f"{map_dir}/{z}-{x+1}-{y+1}.jpg", size2)
                out.paste(im00, (       0,        0))
                out.paste(im10, (size2[0],        0))
                out.paste(im01, (       0, size2[1]))
                out.paste(im11, (size2[0], size2[1]))
                # out = out.filter(ImageFilter.SHARPEN)
                tile = f"{map_dir}/{z-1}-{x//2}-{y//2}.jpg"
                out.save(tile, quality=quality)
        d+=1


# dirs = [p for p in glob('*') if '_map' not in p and os.path.isdir(p)]
files = sorted(glob(f"data/frames/{sys.argv[1]}/*.jpg", recursive=True))
size = Image.open(files[0]).size
map_dir = f'maps/{sys.argv[1]}_hires'
os.makedirs(map_dir, exist_ok=True)
generate_base_zoom(map_dir, files, size)
generate_zoom_out(map_dir, size)

# generate_zoom_out(sys.argv[1])
