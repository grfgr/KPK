# realistic-ish fingerprint mask: whorl ridges warped by noise, partial and smudged
import numpy as np
from PIL import Image, ImageFilter


def mask(w, h, seed=4, angle=-25.0):
    rng = np.random.default_rng(seed)
    yy, xx = np.mgrid[0:h, 0:w].astype(np.float32)
    cx, cy = w * 0.5, h * 0.55
    a = np.radians(angle)
    xr = (xx - cx) * np.cos(a) - (yy - cy) * np.sin(a); yr = (xx - cx) * np.sin(a) + (yy - cy) * np.cos(a)
    nx = xr / (w * 0.36); ny = yr / (h * 0.46)
    def smooth(cells):
        n = rng.random((cells, cells)).astype(np.float32)
        return np.asarray(Image.fromarray((n * 255).astype(np.uint8)).resize((w, h), Image.BICUBIC)).astype(np.float32) / 255
    warp = (smooth(6) - 0.5) * 0.35
    # whorl core above centre, loops flatten towards the bottom (like a real finger pad)
    r = np.sqrt(nx ** 2 + (ny + 0.15) ** 2 * (1.0 + 0.4 * (ny > 0))) + warp
    theta = np.arctan2(ny, nx)
    ridges = np.sin(r * 55 + theta * 0.6 + (smooth(10) - 0.5) * 3.0)
    ridges = np.clip((ridges - 0.1) * 2.5, 0, 1)
    pad = np.clip((1.0 - np.sqrt(nx ** 2 + ny ** 2)) * 6, 0, 1)
    breaks = np.clip((smooth(24) - 0.25) * 3, 0, 1)            # gaps, partial print
    press = np.clip((smooth(4) - 0.2) * 2, 0, 1)               # uneven pressure
    m = ridges * pad * breaks * press
    smear = np.clip(1 - np.abs(ny - 1.0) * 3, 0, 1) * np.clip(1 - np.abs(nx) * 1.8, 0, 1) * (ny > 0.6) * 0.45
    m = np.clip(m + smear * smooth(8), 0, 1)
    return np.asarray(Image.fromarray((m * 255).astype(np.uint8)).filter(ImageFilter.GaussianBlur(0.8))).astype(np.float32) / 255
