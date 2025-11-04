import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

# === LINKAGE PARAMETERS ===
L1 = 43.958   # ground (A–B)
L2 = 95.819   # input crank (A–C)
L3 = 19.766   # coupler (C–D)
L4 = 102.781  # output rocker (D–B)

A = np.array([0.0, 0.0])              # fixed pivot 1
B = np.array([-24.642, 26.459])       # fixed pivot 2

extension_fraction = -4.2              # distance multiplier for P along C–D

# === CRANK MOTION PARAMETERS ===
theta2_start_deg = 200.0               # starting angle of the crank
theta2_end_deg   = 240.0              # ending angle of the crank
n_frames_half = 150                   # number of frames for one sweep

# Convert to radians and build oscillating angle list
theta2_start = np.deg2rad(theta2_start_deg)
theta2_end = np.deg2rad(theta2_end_deg)
forward = np.linspace(theta2_start, theta2_end, n_frames_half)
backward = np.linspace(theta2_end, theta2_start, n_frames_half)
theta2_vals = np.concatenate([forward, backward])

theta4_prev = np.pi / 2

# === SOLVER: loop-closure equation ===
def solve_linkage(theta2):
    global theta4_prev

    xC = A[0] + L2 * np.cos(theta2)
    yC = A[1] + L2 * np.sin(theta2)

    def eq(theta4):
        xD = B[0] + L4 * np.cos(theta4)
        yD = B[1] + L4 * np.sin(theta4)
        return (xD - xC)**2 + (yD - yC)**2 - L3**2

    # --- start Newton from the previous frame's value ---
    theta4 = theta4_prev
    for _ in range(12):
        f  = eq(theta4)
        df = (eq(theta4 + 1e-6) - f) / 1e-6
        theta4 -= f / df

    # save for next iteration
    theta4_prev = theta4

    C = np.array([xC, yC])
    D = np.array([B[0] + L4 * np.cos(theta4), B[1] + L4 * np.sin(theta4)])

    vec = D - C
    P = C + extension_fraction * vec
    return C, D, P

# === ANIMATION SETUP ===
fig, ax = plt.subplots()
ax.set_aspect('equal')
ax.set_xlim(-120, 40)
ax.set_ylim(-200, 50)
ax.set_title("Four-Bar Linkage with Oscillating Crank")
ax.set_xlabel("X")
ax.set_ylabel("Y")

(line_links,) = ax.plot([], [], 'o-', lw=2, label="links")
(coupler_trace,) = ax.plot([], [], 'r--', lw=1, alpha=0.5, label="coupler path")
(coupler_point,) = ax.plot([], [], 'ro', ms=6)

# Moving text labels for joints
label_A = ax.text(A[0]-5, A[1]-5, "A", fontsize=9, color='k')
label_B = ax.text(B[0]+5, B[1]-5, "B", fontsize=9, color='k')
label_C = ax.text(0, 0, "C", fontsize=9, color='k')
label_D = ax.text(0, 0, "D", fontsize=9, color='k')
label_P = ax.text(0, 0, "P", fontsize=9, color='r')

trace_x, trace_y = [], []

def init():
    line_links.set_data([], [])
    coupler_trace.set_data([], [])
    coupler_point.set_data([], [])
    return line_links, coupler_trace, coupler_point, label_C, label_D, label_P

def update(frame):
    theta2 = theta2_vals[frame]
    C, D, P = solve_linkage(theta2)

    xs = [A[0], C[0], D[0], B[0]]
    ys = [A[1], C[1], D[1], B[1]]
    line_links.set_data(xs, ys)

    trace_x.append(P[0])
    trace_y.append(P[1])
    coupler_trace.set_data(trace_x, trace_y)
    coupler_point.set_data([P[0]], [P[1]])

    # update labels
    label_C.set_position((C[0]+2, C[1]+2))
    label_D.set_position((D[0]+2, D[1]+2))
    label_P.set_position((P[0]+2, P[1]+2))

    return line_links, coupler_trace, coupler_point, label_C, label_D, label_P

ani = animation.FuncAnimation(
    fig, update, frames=len(theta2_vals),
    init_func=init, interval=30, blit=True, repeat=True
)

plt.legend()
plt.show()
