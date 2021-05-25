import os
import sys
import numpy as np
import cv2
from scipy.interpolate import griddata

from multiprocessing import Pool


def interpolate_missing(image):
    ny, nx = image.shape[0:2]
    nz_y, nz_x = np.nonzero(image)
    X, Y = np.meshgrid(np.arange(0, nx, 1), np.arange(0, ny, 1))
    interpolated = griddata((nz_y, nz_x), image[nz_y, nz_x], (Y, X), method='linear')
    return interpolated

def crop(fpath):
    print("cropping file", fpath)
    color = cv2.imread(fpath, cv2.IMREAD_UNCHANGED)
    mask = cv2.imread(fpath.replace('color.jpg', 'rgbd.tiff'), cv2.IMREAD_GRAYSCALE)
    # mask = interpolate_missing(mask)
    result = cv2.bitwise_and(color, color, mask=np.round(mask.astype('uint8')))
    cv2.imwrite(fpath.replace('color.jpg', 'color_cropped.jpg'), result)

if __name__ == "__main__":
    path = sys.argv[1]
    # 'cn05/0000000090_rgbd.tiff
    files = map(lambda x: os.path.join(path, x), filter(lambda val: val.endswith('color.jpg'), os.listdir(path)))
    with Pool(4) as p:
        p.map(crop, files)
