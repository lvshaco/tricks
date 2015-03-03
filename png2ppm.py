#_*_ coding:utf-8 _*_

import os

def getfile(path, exts, files, level):
    for f in os.listdir(path):
        f = os.path.join(path, f)
        if os.path.isfile(f):
            _, ext = os.path.splitext(os.path.basename(f))
            if ext in exts:
                files.append(f)
        else:
            getfile(f, exts, files, level+1)

def mkdirs(path, outp):
    if not os.path.exists(outp):
        os.mkdir(outp)
    for f in os.listdir(path):
        ori = os.path.join(path, f)
        if not os.path.isfile(ori):
            out = os.path.join(outp, f)
            mkdirs(ori, out)

if __name__ == "__main__":
    files = []
    path = "png"
    outp = "ppm"
    mkdirs(path, outp)
    getfile(path, (".png"), files, 0)
    for fname in files:
        outname, _ = os.path.splitext(fname)
        outname = outp+outname[3:]
        print (outname)
        os.system('convert.exe "%s" "%s.ppm"'%(fname, outname))
        os.system('convert.exe "%s" -channel A -separate "%s.pgm"'%(fname, outname))
    os.system("pause")
