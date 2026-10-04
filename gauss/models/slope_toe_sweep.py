#!/usr/bin/env python3
"""
Radial sweep: how much tsunami-transport signal (|H*u|, the magnetometer proxy)
survives as we move the sensor IN from the deep ocean toward the island coast,
and what the signal / cable-length trade-off looks like.

Linear shallow-water, staggered C-grid, forward-backward. Circular island.
"""
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

g = 9.81

# ---- domain -------------------------------------------------------------
L   = 550e3            # m, square domain
N   = 220
dx  = L / N
x   = (np.arange(N) + 0.5) * dx
X, Y = np.meshgrid(x, x, indexing="ij")

cx = cy = 275e3        # island centre
R  = np.sqrt((X - cx)**2 + (Y - cy)**2)

# ---- bathymetry: coast -> shelf -> slope -> deep ------------------------
R_coast = 38e3         # shoreline
R_shelf = 65e3         # shelf edge, H ~ 100 m
R_toe   = 120e3        # toe of slope, meets the deep plain
H_deep  = 4000.0
H_shelf = 100.0
H_min   = 25.0         # never fully dry (keeps model linear/stable)

H = np.full((N, N), H_deep)
# shelf: coast..shelf_edge  ->  H_min..H_shelf
m = (R >= R_coast) & (R < R_shelf)
H[m] = H_min + (H_shelf - H_min) * (R[m] - R_coast) / (R_shelf - R_coast)
# slope: shelf_edge..toe    ->  H_shelf..H_deep
m = (R >= R_shelf) & (R < R_toe)
H[m] = H_shelf + (H_deep - H_shelf) * (R[m] - R_shelf) / (R_toe - R_shelf)
# land
land = R < R_coast
H[land] = H_min

# ---- time stepping ------------------------------------------------------
cmax = np.sqrt(g * H_deep)
dt   = 0.30 * dx / cmax
T    = 2600
A    = 0.6             # incoming amplitude, m
sigma = 25e3          # wave-packet width
x0    = 60e3          # launch position (left edge region)
cin   = np.sqrt(g * H_deep)

eta = np.zeros((N, N))
u   = np.zeros((N + 1, N))   # x-velocity on x-faces
v   = np.zeros((N, N + 1))   # y-velocity on y-faces

# H on faces
Hu = 0.5 * (H[1:, :] + H[:-1, :])   # (N-1, N)
Hv = 0.5 * (H[:, 1:] + H[:, :-1])   # (N, N-1)

# sponge (Gauss-shaped damping ramp near all 4 edges)
def edge_ramp(n, width):
    r = np.ones(n)
    for i in range(width):
        r[i] = r[-1 - i] = (i / width)
    return r
spx = edge_ramp(N, 20)
spy = edge_ramp(N, 20)
sponge = np.outer(spx, spy)
# never damp near the island; only the outer frame
sponge = np.maximum(sponge, (R < R_toe + 40e3).astype(float))
sponge = np.clip(sponge, 0.0, 1.0)

peak = np.zeros((N, N))   # running max of |H*u| (transport magnitude)

for t in range(T):
    tt = t * dt
    # inject plane wave at left column (soft source)
    eta[0, :] = A * np.exp(-((cin * tt - (x0)) ** 2) / (2 * sigma**2))

    # u update (interior x-faces)
    dudt = -g * (eta[1:, :] - eta[:-1, :]) / dx
    u[1:-1, :] += dt * dudt
    # v update
    dvdt = -g * (eta[:, 1:] - eta[:, :-1]) / dx
    v[:, 1:-1] += dt * dvdt

    # no flux into land faces
    u[1:-1, :][land[1:, :] | land[:-1, :]] = 0.0
    v[:, 1:-1][land[:, 1:] | land[:, :-1]] = 0.0

    # eta update (continuity)
    flux_x = (Hu * u[1:-1, :])                      # (N-1,N) on x-faces interior
    div = np.zeros((N, N))
    div[1:, :]  += flux_x
    div[:-1, :] -= flux_x
    flux_y = (Hv * v[:, 1:-1])
    div[:, 1:]  += flux_y
    div[:, :-1] -= flux_y
    eta += dt * div / dx
    eta[land] = 0.0
    eta *= sponge
    # damp velocities in the outer frame too (kills edge reflections/blow-up)
    su = 0.5 * (sponge[1:, :] + sponge[:-1, :])
    sv = 0.5 * (sponge[:, 1:] + sponge[:, :-1])
    u[1:-1, :] *= su
    v[:, 1:-1] *= sv

    # transport magnitude at cell centres
    uc = 0.5 * (u[1:, :] + u[:-1, :])
    vc = 0.5 * (v[:, 1:] + v[:, :-1])
    speed = np.sqrt(uc**2 + vc**2)
    trans = H * speed
    trans[land] = 0.0
    peak = np.maximum(peak, trans)

# ---- radial sweep along the wave-facing axis (upstream, -x side) --------
# sample rings; take the median on the wave-facing 120-deg arc
theta = np.arctan2(Y - cy, X - cx)
facing = np.abs(theta - np.pi) < np.deg2rad(60)   # points toward incoming (-x) wave

radii = np.arange(R_coast + 2e3, R_toe + 80e3, 3e3)
raw = []
for rr in radii:
    ring = (np.abs(R - rr) < 2e3) & facing & (~land)
    if ring.sum() == 0:
        raw.append(np.nan); continue
    raw.append(np.median(peak[ring]))
raw = np.array(raw)

# reference = the deep-water plateau ON THE SAME wave-facing side
deep_plateau = np.nanmedian(raw[radii > R_toe + 25e3])
deep_ref = deep_plateau
sig = raw / deep_plateau

dist_from_coast = (radii - R_coast) / 1e3   # km of cable from the shore station

# cable "cost": proportional to distance from coast (station sits at coast)
cost = np.maximum(dist_from_coast, 1.0)
merit = sig / cost      # signal per km of cable

# knee: smallest distance that still keeps >= 85% of signal
good = np.where(sig >= 0.85)[0]
knee_i = good[0] if len(good) else np.nanargmax(sig)

print(f"deep_ref transport = {deep_ref:.2f}")
print(f"toe distance from coast = {(R_toe-R_coast)/1e3:.0f} km")
for d, s, mo in zip(dist_from_coast, sig, merit):
    print(f"  {d:6.1f} km   signal {s:5.2f}   merit {mo:.3f}")
print(f"knee: {dist_from_coast[knee_i]:.0f} km keeps signal {sig[knee_i]:.2f}")

# ---- figure -------------------------------------------------------------
fig, ax = plt.subplots(1, 2, figsize=(13, 5.2))

# left: bathymetry + peak transport
im = ax[0].imshow(peak.T / deep_ref, origin="lower",
                  extent=[0, L/1e3, 0, L/1e3], cmap="magma",
                  vmin=0, vmax=1.2, aspect="equal")
cs = ax[0].contour(x/1e3, x/1e3, H.T, levels=[100, 1000, 3000],
                   colors="cyan", linewidths=0.5, alpha=0.6)
th = np.linspace(0, 2*np.pi, 200)
for rr, lab, c in [(R_coast, "coast", "w"), (R_toe, "toe", "lime")]:
    ax[0].plot((cx + rr*np.cos(th))/1e3, (cy + rr*np.sin(th))/1e3,
               c, lw=1.0, ls="--")
ax[0].set_title("peak transport |H·u|  (× deep)")
ax[0].set_xlabel("km"); ax[0].set_ylabel("km")
ax[0].annotate("", xy=(70, 275), xytext=(20, 275),
               arrowprops=dict(arrowstyle="-|>", color="white", lw=2))
ax[0].text(20, 255, "tsunami", color="white")
fig.colorbar(im, ax=ax[0], shrink=0.8)

# right: signal vs distance from coast
ax[1].plot(dist_from_coast, sig, "-o", color="#c0392b", ms=4, label="signal (× deep)")
ax[1].axhline(0.85, color="gray", ls=":", lw=1)
ax[1].axhline(1.0,  color="gray", ls="--", lw=0.8)
ax[1].axvline((R_toe-R_coast)/1e3, color="lime", ls="--", lw=1, label="toe of slope")
ax[1].axvline(dist_from_coast[knee_i], color="k", ls=":", lw=1)
ax[1].plot(dist_from_coast[knee_i], sig[knee_i], "k*", ms=15,
           label=f"knee ≈ {dist_from_coast[knee_i]:.0f} km @ {sig[knee_i]:.0%}")
ax2 = ax[1].twinx()
ax2.plot(dist_from_coast, merit, "-", color="#2980b9", alpha=0.6, label="signal / km cable")
ax2.set_ylabel("signal per km cable  (value)", color="#2980b9")
ax[1].set_xlabel("cable length from coast station  [km]")
ax[1].set_ylabel("transport signal  (× deep-water)")
ax[1].set_title("how much signal survives coming in from the deep")
ax[1].set_ylim(0, 1.15)
ax[1].legend(loc="lower right", fontsize=8)
fig.tight_layout()
fig.savefig("slope_toe_sweep.png", dpi=110)
print("saved slope_toe_sweep.png")
