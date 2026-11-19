# KPK interface sounds, synthesised (no samples): key click, module switch blip, random PDA beeps,
# data chirp, radio static, Monolith whisper. Writes KPK/data/sounds/*.ogg (ffmpeg, libvorbis).
import numpy as np, wave, subprocess, os
SR = 44100
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "../../KPK/data/sounds")
rng = np.random.default_rng(7)


def t(sec):
    return np.arange(int(SR * sec)) / SR


def env(n, a=0.002, r=0.02):
    e = np.ones(n); na = max(1, int(a * SR)); nr = max(1, int(r * SR))
    e[:na] = np.linspace(0, 1, na); e[-nr:] *= np.linspace(1, 0, nr) ** 2
    return e


def tone(f, sec, shape="sine", a=0.002, r=0.02):
    x = t(sec); ph = 2 * np.pi * f * x
    s = np.sin(ph) if shape == "sine" else np.tanh(3 * np.sin(ph)) * 0.6
    return s * env(len(x), a, r)


def lowpass(x, k):
    # one-pole low-pass, k in 0..1 (smaller = darker)
    y = np.zeros_like(x); acc = 0.0
    for i, v in enumerate(x):
        acc += k * (v - acc); y[i] = acc
    return y


def sil(sec):
    return np.zeros(int(SR * sec))


def save(name, x, gain):
    x = x / (np.abs(x).max() + 1e-9) * gain
    pcm = (np.clip(x, -1, 1) * 32767).astype(np.int16)
    wav = "/tmp/kpk_%s.wav" % name
    with wave.open(wav, "wb") as w:
        w.setnchannels(1); w.setsampwidth(2); w.setframerate(SR); w.writeframes(pcm.tobytes())
    subprocess.run(["ffmpeg", "-y", "-loglevel", "error", "-i", wav, "-c:a", "libvorbis", "-q:a", "5", os.path.join(OUT, name + ".ogg")], check=True)
    os.remove(wav)


os.makedirs(OUT, exist_ok=True)
# soft plastic key: a tiny noise tick plus a short damped body
n = int(SR * 0.035)
tick = rng.normal(0, 1, n) * np.exp(-np.arange(n) / (SR * 0.0012))
body = np.sin(2 * np.pi * 1850 * np.arange(n) / SR) * np.exp(-np.arange(n) / (SR * 0.006))
save("kpk_click", lowpass(tick, 0.5) * 0.7 + body * 0.5, 0.35)
# module switch: two quick rising blips
save("kpk_tab", np.concatenate([tone(1500, 0.03, "sq", r=0.012), sil(0.012), tone(2250, 0.04, "sq", r=0.02)]), 0.28)
# random PDA beeps
save("kpk_beep1", np.concatenate([tone(1950, 0.07, "sq"), sil(0.06), tone(1950, 0.07, "sq")]), 0.25)
save("kpk_beep2", np.concatenate([tone(2600, 0.05, "sq"), sil(0.04), tone(2000, 0.05, "sq"), sil(0.04), tone(1500, 0.09, "sq", r=0.05)]), 0.22)
# data chirp: stepped modem tones
steps = [tone(f, 0.018, "sq", a=0.001, r=0.004) for f in rng.choice([900, 1200, 1500, 1800, 2400], 16)]
save("kpk_data", np.concatenate(steps), 0.16)
# radio static: band-limited noise with crackles, swelling in and out
x = t(0.7); nz = lowpass(rng.normal(0, 1, len(x)), 0.35) - lowpass(rng.normal(0, 1, len(x)), 0.02)
crk = (rng.random(len(x)) > 0.9985) * rng.normal(0, 4, len(x))
save("kpk_static", (nz + crk) * np.sin(np.pi * x / x[-1]) ** 1.5, 0.2)
# Monolith: low breathy noise with a slow tremble and a faint drone
x = t(1.6); br = lowpass(rng.normal(0, 1, len(x)), 0.06) * (0.6 + 0.4 * np.sin(2 * np.pi * 5.5 * x))
drone = np.sin(2 * np.pi * 73 * x) * 0.25 + np.sin(2 * np.pi * 110.5 * x) * 0.15
save("kpk_whisper", (br + drone) * np.sin(np.pi * x / x[-1]) ** 2, 0.25)
# new message
save("kpk_msg", np.concatenate([tone(1320, 0.06), sil(0.03), tone(1760, 0.1, r=0.06)]), 0.3)
# boot: dial-up modem handshake squeezed into ~1.6 s: dial tone, DTMF digits, answer tone, then the
# warbling negotiation and a hiss of carrier noise, through a small phone-like speaker (band-limited)
def dtmf(lo, hi, sec):
    x = t(sec); return (np.sin(2 * np.pi * lo * x) + np.sin(2 * np.pi * hi * x)) * env(len(x), 0.003, 0.01) * 0.5
parts = [tone(350, 0.18, a=0.01, r=0.01) * 0.5 + tone(440, 0.18, a=0.01, r=0.01) * 0.5, sil(0.03)]
for lo, hi in [(697, 1336), (770, 1209), (852, 1477), (941, 1336), (697, 1209)]:
    parts += [dtmf(lo, hi, 0.045), sil(0.022)]
parts += [sil(0.05), tone(2100, 0.22, a=0.005, r=0.01)]
x = t(0.25); parts.append(np.sin(2 * np.pi * (1650 + 450 * np.sign(np.sin(2 * np.pi * 22 * x))) * x) * 0.6 * env(len(x), 0.004, 0.01))
x = t(0.45); fm = 1800 + 700 * np.sin(2 * np.pi * 37 * x) + 300 * np.sin(2 * np.pi * 91 * x)
carrier = np.sin(2 * np.pi * np.cumsum(fm) / SR) * 0.45 + lowpass(rng.normal(0, 1, len(x)), 0.5) * 0.55
parts.append(carrier * env(len(x), 0.01, 0.12))
m = np.concatenate(parts)
m = lowpass(m, 0.55) - lowpass(m, 0.03)
save("kpk_modem", m, 0.2)
# hiss while the KPK is open: very soft band-limited white noise, loops seamlessly (crossfaded ends)
x = t(4.0); nz = lowpass(rng.normal(0, 1, len(x)), 0.45) - lowpass(rng.normal(0, 1, len(x)), 0.01)
xf = int(SR * 0.5); head = nz[:xf].copy(); body = nz[xf:].copy()
fade = np.linspace(0, 1, xf); body[-xf:] = body[-xf:] * (1 - fade) + head * fade
save("kpk_hiss", body, 0.05)
print("ok")
